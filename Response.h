#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "utils.h"
#include <vector>

class Response {
    public:
    Response(std::vector<char> & msg);
    std::vector<char> & getMsg() {return this->msg;}
    std::vector<char> & getHeader() {return this->header;}
    int & getContentLength() {return this->contentLength;}
    
    private:
    std::vector<char> & msg;
    std::vector<char> header;
    int contentLength;
    bool parseHeader();
    bool parseContentLength();
};

Response::Response(std::vector<char> & msg) : msg(msg), header(), contentLength(-1) {
    this->parseHeader();
    // this->parseContentLength();
}

bool Response::parseHeader() {
    cleanVectorChar(this->header);
    const char * pend = strstr(this->msg.data(), "\r\n\r\n");
    if (pend != NULL) {
        const char * pcur = this->msg.data();
        while (pcur < pend) {
            this->header.push_back(*pcur);
            pcur = pcur + 1;
        }
    }
    if (this->header.size() > 0) {
        appendCstrToVectorChar(this->header, "\r\n\r\n");
    }

    return true;
}

// bool Response::parseContentLength() {
//     this->contentLength = -1;
//     if (this->header.size() == 0) {
//         return true;
//     }
//     const char * pch = strstr(this->header.data(), "Content-Length:");
//     if (pch == NULL) {
//         return true;
//     }
//     pch = pch + strlen("Content-Length:");
//     skipSpace(&pch);
//     std::vector<char> contLenStr;
//     while (*pch == '\r') {
//         contLenStr.push_back(*pch);
//         pch = pch + 1;
//     }
//     std::cout << "!!!!!!!!!!" << contLenStr.data() << "!!!!!!!!!" << "\n";
//     if (contLenStr.size() > 0) {
//         contLenStr.push_back('\0');
//         char * endp;
//         long converted = strtol(contLenStr.data(), &endp, 10);
//         if (endp == contLenStr.data()) {
//             std::stringstream ess;
//             ess << "error occured when paring content length in " << __func__;
//             throw std::invalid_argument(ess.str());
//         }
//         this->contentLength = (int) converted;
//     }

//     return true;
// }

#endif