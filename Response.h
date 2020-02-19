#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "utils.h"
#include <vector>

class Response {
    public:
    Response();
    void clearAll();
    bool parse(std::vector<char> & msg);
    // std::vector<char> & getMsg() {return this->msg;}
    std::vector<char> & getHeader() {return this->header;}
    size_t & getContentLength() {return this->contentLength;}
    
    private:
    // std::vector<char> & msg;
    std::vector<char> header;
    size_t contentLength;
    const char * parseHeader(std::vector<char> & msg);
    bool parseContentLength(std::vector<char> & msg);
};

Response::Response() : header(), contentLength(-1) {}

bool Response::parse(std::vector<char> & msg) {
    msg.push_back('\0');
    try {
        if (this->header.size() == 0) {
            this->parseHeader(msg);
        }
        if (this->contentLength < 0) {
            this->parseContentLength(msg);
        }
    }
    catch (std::invalid_argument & e) {
        msg.pop_back();
        throw std::invalid_argument(e.what());
    }
    msg.pop_back();
    return true;
}

void Response::clearAll() {
    cleanVectorChar(this->header);
    this->contentLength = -1;
}

const char * Response::parseHeader(std::vector<char> & msg) {
    const char * pend = strstr(msg.data(), "\r\n\r\n");
    if (pend != NULL) {
        const char * pcur = msg.data();
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

bool Response::parseContentLength(std::vector<char> & msg) {
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

    return false;
}

#endif