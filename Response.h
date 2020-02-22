#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "utils.h"
#include "HttpParser.h"
// #include <vector>
#include <algorithm>
#include <cassert>

class Response : public HttpParser {
    public:
    Response();
    virtual void clearResponse();
    virtual bool parse(std::vector<char> & msg);
    virtual std::vector<char> & getDatetimeVectorChar() {return this->datetime;}
    virtual std::vector<char> & getTimeZone() {return this->timeZone;}
    virtual std::vector<char> reconstructLinedHeaders();
    
    protected:
    std::vector<char> datetime;
    std::vector<char> timeZone;
    virtual bool parseDateTime(std::vector<char> & msg);
};

Response::Response() : HttpParser() {
    this->builtInHeaders.insert("Date");
}

bool Response::parse(std::vector<char> & msg) {
    msg.push_back('\0');
    try {
        if (this->firstLine.size() == 0) {
            this->parseFirstLine(msg);
        }
        if (this->header.size() == 0) {
            this->parseHeader(msg);
        }
        if (this->headerFields.size() == 0) {
            this->parseHeaderFields();
        }
        if (this->contentLength < 0) {
            this->parseContentLength(msg);
        }
        if (this->datetime.size() == 0) {
            this->parseDateTime(msg);
        }
        if (this->datetime.size() > 0) {
            assert(this->timeZone.size() > 0);
        }
        //
        if (this->transferEncoding.size() == 0) {
            this->parseTransferEncoding(msg);
        }
        if (this->content.size() == 0) {
            this->parseContent(msg);
        }
        if (this->checkIsCompleted(msg)) {
            msg.pop_back();
            return true;
        } 
    }
    catch (std::invalid_argument & e) {
        msg.pop_back();
        throw std::invalid_argument(e.what());
    }
    msg.pop_back();
    return true;
}

void Response::clearResponse() {
    this->clear();
    cleanVectorChar(this->datetime);
    cleanVectorChar(this->timeZone);
}

std::vector<char> Response::reconstructLinedHeaders() {
    std::vector<char> recon;
    // // First line
    // std::vector<char> firstLine = response.getFirstLine();
    // // Date
    // std::vector<char> datetimeVectorChar = response.getDatetimeVectorChar();
    // std::vector<char> timeZone = response.getTimeZone();
    // // Content-Length
    // int contentLength = response.getContentLength();
    // // Transfer-Encoding
    // std::vector<char> transferEncoding = response.getTransferEncoding();
    // // Header fields
    // header_fields_t headerFields = response.getHeaderFields();
    
    if (this->firstLine.size() > 0) {
        appendCstrToVectorChar(recon, this->firstLine.data());
        appendCstrToVectorChar(recon, "\r\n");
    }
    if (this->datetime.size() > 0) {
        appendCstrToVectorChar(recon, "Date: ");
        appendCstrToVectorChar(recon, this->datetime.data());
        if (this->timeZone.size() > 0) {
            appendCstrToVectorChar(recon, " ");
            appendCstrToVectorChar(recon, this->timeZone.data());
        }
        appendCstrToVectorChar(recon, "\r\n");
    }
    if (this->contentLength > -1) {
        appendCstrToVectorChar(recon, "Content-Length: ");
        std::stringstream cl;
        cl << this->contentLength;
        appendCstrToVectorChar(recon, cl.str().c_str());
        appendCstrToVectorChar(recon, "\r\n");
    }
    if (this->headerFields.size() > 0) {
        std::stringstream hd;
        for (header_fields_t::iterator it = this->headerFields.begin();
        it != this->headerFields.end(); ++it) {
            hd << it->first << ": " << it->second.data() << "\r\n";
        }
        appendCstrToVectorChar(recon, hd.str().c_str());
    }
    appendCstrToVectorChar(recon, "\r\n");

    return recon;
}

bool Response::parseDateTime(std::vector<char> & msg) {
    const char * pch = strstr(msg.data(), "Date:");
    if (pch == NULL) {
        return false;
    }
    pch = pch + strlen("Date:");
    skipSpace(&pch);
    std::vector<char> tempDate;
    while (*pch != '\r' && *pch != '\0') {
        tempDate.push_back(*pch);
        pch = pch + 1;
    }
    if (*pch == '\0') {
        return false;
    }
    if (tempDate.size() > 0) {
        tempDate.push_back('\0');
        if (strstr(tempDate.data(), " ") == NULL) {
            std::cerr << "no space found in datetime!\n";
            return false;
        }
        tempDate.pop_back(); // pop '\0'
        while (tempDate[tempDate.size()-1] != ' ') {
            this->timeZone.push_back(tempDate[tempDate.size()-1]);
            tempDate.pop_back();
        }
        tempDate.pop_back(); // pop ' '
        if (this->timeZone.size() > 0) {
            std::reverse(this->timeZone.begin(), this->timeZone.end());
            this->timeZone.push_back('\0');
        }
        tempDate.push_back('\0');
        this->datetime = tempDate;
        return true;
    }
    std::cerr << "cannot parse datetime!\n";
    return false;
}

#endif