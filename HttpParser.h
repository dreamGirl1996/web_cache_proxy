#ifndef __HTTPPARSER_H__
#define __HTTPPARSER_H__

#include "utils.h"

class HttpParser {
    public:
    HttpParser();
    virtual bool parse(std::vector<char> & msg) = 0;
    virtual bool & getIsCompleted() {return this->isCompleted;}
    virtual std::vector<char> & getFirstLine() {return this->firstLine;}
    virtual std::vector<char> & getHeader() {return this->header;}
    virtual std::vector<char> & getContent() {return this->content;}
    virtual int & getContentLength() {return this->contentLength;}
    virtual std::vector<char> & getTransferEncoding() {return this->transferEncoding;}

    protected:
    bool isCompleted;
    std::vector<char> firstLine;
    std::vector<char> header;
    std::vector<char> content;
    int contentLength;
    std::vector<char> transferEncoding;
    virtual void clear();
    virtual bool checkIsCompleted(std::vector<char> & msg);
    virtual const char * parseFirstLine(std::vector<char> & msg);
    virtual const char * parseHeader(std::vector<char> & msg);
    virtual const char * parseContent(std::vector<char> & msg);
    virtual bool parseContentLength(std::vector<char> & msg);
    virtual bool parseTransferEncoding(std::vector<char> & msg);
};

HttpParser::HttpParser() : isCompleted(false), header(), contentLength(-1), transferEncoding() {}

void HttpParser::clear() {
    this->isCompleted = false;
    cleanVectorChar(this->header);
    this->contentLength = -1;
    cleanVectorChar(this->transferEncoding);
}

bool HttpParser::checkIsCompleted(std::vector<char> & msg) {
    // should check content-length first
    if (msg.size() == 0 || this->header.size() == 0) {
        this->isCompleted = false;
        return false;
    }
    if (this->contentLength > 0 || 
    (this->transferEncoding.size() > 0 && 
    strcmp(this->transferEncoding.data(), "chunked") == 0)) {
        if (this->content.size() > 0) {
            this->isCompleted = true;
            return true;
        }
    }
    else {
    // if (msg.size() > 0) {
        if (strstr(msg.data(), "\r\n\r\n") != NULL) {
            this->isCompleted = true;
            return true;
        }
    }
    this->isCompleted = false;
    return false;
}

bool HttpParser::parseTransferEncoding(std::vector<char> & msg) {
    // if (msg.size() == 0) {
    //     return false;
    // }
    const char * pch = strstr(msg.data(), "Transfer-Encoding:");
    if (pch == NULL) {
        return false;
    }
    pch = pch + strlen("Transfer-Encoding:");
    skipSpace(&pch);
    std::vector<char> temp;
    while (*pch != '\r' && *pch != '\0') {
        temp.push_back(*pch);
        pch = pch + 1;
    }
    if (*pch == '\0') {
        return false;
    }
    if (temp.size() > 0) {
        temp.push_back('\0');
        this->transferEncoding = temp;
        return true;
    }
    return false;
}

const char * HttpParser::parseFirstLine(std::vector<char> & msg) {
    const char * pend = strstr(msg.data(), "\r\n");
    if (pend == NULL) {
        return NULL;
    }
    const char * pcur = msg.data();
    while (pcur < pend) {
        this->firstLine.push_back(*pcur);
        pcur = pcur + 1;
    }
    if (this->firstLine.size() > 0) {
        this->firstLine.push_back('\0');
        pend = pend - 1 + strlen("\r\n");
        return pend;
    }
    return NULL;
}

const char * HttpParser::parseHeader(std::vector<char> & msg) {
    const char * pcur = strstr(msg.data(), "\r\n");
    if (pcur == NULL) {
        return NULL;
    }
    pcur = pcur + strlen("\r\n");
    const char * pend = strstr(msg.data(), "\r\n\r\n");
    if (pend != NULL) {
        // const char * pcur = msg.data();
        while (pcur < pend) {
            this->header.push_back(*pcur);
            pcur = pcur + 1;
        }
    }
    if (this->header.size() > 0) {
        appendCstrToVectorChar(this->header, "\r\n\r\n");
        pend = pend - 1 + strlen("\r\n\r\n");
        return pend;
    }

    return NULL;
}

const char * HttpParser::parseContent(std::vector<char> & msg) {
    if (this->header.size() == 0) {
        return NULL;
    }
    if (contentLength <= 0 && (this->transferEncoding.size() == 0 ||
    strcmp(this->transferEncoding.data(), "chunked") != 0)) {
        return NULL;
    }
    if (msg.size() <= this->header.size()) {
        return NULL;
    }

    const char * pcur = strstr(msg.data(), "\r\n\r\n");
    if (pcur == NULL) {
        std::cerr << "Not found cslr cslr in " << __func__ << "\n";
        return NULL;
    }
    pcur = pcur + strlen("\r\n\r\n"); 

    if (contentLength <= 0) { // chunked data
        //std::cout << "chunked!\n";
        const char * pend = strstr(msg.data(), "\r\n0\r\n\r\n");
        if (pend == NULL) {
            return NULL;
        }
        while (pcur < pend) {
            this->content.push_back(*pcur);
            pcur = pcur + 1;
        }
        if (this->content.size() > 0) {
            appendCstrToVectorChar(this->content, "\r\n0\r\n\r\n");
            pend = pend - 1 + strlen("\r\n0\r\n\r\n");
            return pend;
        }
    }
    else { // content-length
        // std::cout << "contentlenth!\n";
        if ((int) msg.size() - (int) this->header.size() -
        (int) this->firstLine.size() - (int) strlen("\r\n") >= this->contentLength) {
            while (*pcur != '\0') {
                this->content.push_back(*pcur);
                pcur = pcur + 1;
            }
            if (this->content.size() > 0) {
                this->content.push_back('\0');
                return pcur;
            }
        }
    }
    
    return NULL;
}

bool HttpParser::parseContentLength(std::vector<char> & msg) {
    if (msg.size() == 0) {
        return false;
    }
    const char * pch = strstr(msg.data(), "Content-Length:");
    if (pch == NULL) {
        return false;
    }
    pch = pch + strlen("Content-Length:");
    skipSpace(&pch);
    std::vector<char> contLenStr;
    while (*pch != '\r' && *pch != '\0') {
        contLenStr.push_back(*pch);
        pch = pch + 1;
    }
    if (*pch == '\0') {
        return false;
    }
    if (contLenStr.size() > 0) {
        contLenStr.push_back('\0');
        char * endp;
        long converted = strtol(contLenStr.data(), &endp, 10);
        if (endp == contLenStr.data()) {
            std::stringstream ess;
            ess << "error occured when parsing Content-Length in " \
            << __func__;
            throw std::invalid_argument(ess.str());
        }
        this->contentLength = (size_t) converted;
        return true;
    }
    std::cerr << "cannot parse content-length!\n";
    return false;
}

#endif