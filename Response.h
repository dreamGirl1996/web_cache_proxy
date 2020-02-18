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
    // std::vector<char> & getHeader() {return this->header;}
    int & getContentLength() {return this->contentLength;}
    
    private:
    // std::vector<char> & msg;
    // std::vector<char> header;
    int contentLength;
    // bool parseHeader();
    bool parseContentLength(std::vector<char> & msg);
};

Response::Response() : contentLength(-1) {}

bool Response::parse(std::vector<char> & msg) {
    msg.push_back('\0');
    try {
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
    this->contentLength = -1;
}

// bool Response::parseHeader() {
//     cleanVectorChar(this->header);
//     const char * pend = strstr(this->msg.data(), "\r\n\r\n");
//     if (pend != NULL) {
//         const char * pcur = this->msg.data();
//         while (pcur < pend) {
//             this->header.push_back(*pcur);
//             pcur = pcur + 1;
//         }
//     }
//     if (this->header.size() > 0) {
//         appendCstrToVectorChar(this->header, "\r\n\r\n");
//     }

//     return true;
// }

bool Response::parseContentLength(std::vector<char> & msg) {
    // this->contentLength = -1;
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
        this->contentLength = (int) converted;
        return true;
    }

    return false;
}

#endif