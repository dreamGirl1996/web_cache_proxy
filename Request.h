#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "utils.h"
// #include <iostream>
#include <vector>
#include <cstring>

class Request {
    public:
    Request(std::vector<char> & msg);
    std::vector<char> & getMsg() {return this->msg;}
    std::vector<char> & getHostName() {return this->hostName;}
    std::vector<char> & getPort() {return this->port;}
    std::vector<char> & getMethod() {return this->method;}
    // int & getContentLength() {return this->contentLength;}

    private:
    std::vector<char> & msg;
    std::vector<char> hostName;
    std::vector<char> port;
    std::vector<char> method; 
    // int contentLength;
    bool parseHostName();
    bool parseMethod();
    // bool parseContentLength();
};

Request::Request(std::vector<char> & msg) : msg(msg), hostName(), method() {
// Request::Request(std::vector<char> & msg) : msg(msg), hostName(), method(), contentLength(-1) {
    this->parseHostName();
    this->parseMethod();
    // this->parseContentLength();
}

bool Request::parseHostName() {
    cleanVectorChar(this->hostName);
    cleanVectorChar(this->port);
    const char *pch = strstr(this->msg.data(), "Host:");
    if (pch != NULL) {
        pch = pch + 5;
        skipSpace(&pch);
        while (*pch != '\r') {
            this->hostName.push_back(*pch);
            pch = pch + 1;
        }
    }
    if (this->hostName.size() > 0) {
        cstrToVectorChar(this->port, "80");
    }
    for (size_t i = 0; i < this->hostName.size(); i++) {
        if (this->hostName[i] == ':') {
            if (i < this->hostName.size() - 1 && this->hostName[i+1] == '4') {
                cstrToVectorChar(this->port, "443");
            }
            while (this->hostName.back() != ':') {
                this->hostName.pop_back();
            }
            hostName.pop_back();
        }
    }
    if (this->hostName.size() > 0) {
        this->hostName.push_back('\0');
    }

    return true;
}

bool Request::parseMethod() {
    cleanVectorChar(this->method);
    if (strstr(this->msg.data(), "GET") != NULL) {
        cstrToVectorChar(this->method, "GET");
    }
    else if (strstr(this->msg.data(), "POST") != NULL) {
        cstrToVectorChar(this->method, "POST");
    }
    else if (strstr(this->msg.data(), "CONNECT") != NULL) {
        cstrToVectorChar(this->method, "CONNECT");
    }

    return true;
}

#endif