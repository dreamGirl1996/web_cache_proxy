#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "utils.h"
// #include <iostream>
#include <vector>
#include <cstring>
#include <cassert>

class Request {
    public:
    Request();
    void clearAll();
    bool parse(std::vector<char> & msg);
    // std::vector<char> & getMsg() {return this->msg;}
    std::vector<char> & getHostName() {return this->hostName;}
    std::vector<char> & getPort() {return this->port;}
    std::vector<char> & getMethod() {return this->method;}
    // int & getContentLength() {return this->contentLength;}

    private:
    // std::vector<char> & msg;
    std::vector<char> hostName;
    std::vector<char> port;
    std::vector<char> method; 
    // int contentLength;
    bool parseHostName(std::vector<char> & msg);
    bool parseMethod(std::vector<char> & msg);
    // bool parseContentLength();
};

Request::Request() : hostName(), port(), method() {
    // this->parseHostName();
    // this->parseMethod();

    // this->parseContentLength();
}

bool Request::parse(std::vector<char> & msg) {
    msg.push_back('\0');
    if (this->hostName.size() == 0) {
        this->parseHostName(msg);
    }
    if (this->hostName.size() > 0) {
        assert(this->port.size() > 0);
    }
    if (this->method.size() == 0) {
        this->parseMethod(msg);
    }
    msg.pop_back();

    return true;
}

void Request::clearAll() {
    cleanVectorChar(this->hostName);
    cleanVectorChar(this->port);
    cleanVectorChar(this->method);
}

bool Request::parseHostName(std::vector<char> & msg) {
    const char *pch = strstr(msg.data(), "Host:");
    if (pch == NULL) {
        return false;
    }
    pch = pch + 5;
    skipSpace(&pch);
    std::vector<char> tempHostName;
    while (*pch != '\r' && *pch != '\0') {
        tempHostName.push_back(*pch);
        pch = pch + 1;
    }
    if (*pch == '\0') {
        return false;
    }
    if (tempHostName.size() > 0) {
        cstrToVectorChar(this->port, "80");
    }
    for (size_t i = 0; i < tempHostName.size(); i++) {
        if (tempHostName[i] == ':') {
            if (i < tempHostName.size() - 1 && tempHostName[i+1] == '4') {
                cstrToVectorChar(this->port, "443");
            }
            while (tempHostName.back() != ':') {
                tempHostName.pop_back();
            }
            tempHostName.pop_back();
        }
    }
    if (tempHostName.size() > 0) {
        tempHostName.push_back('\0');
        this->hostName = tempHostName;
    }
    
    return true;
}

bool Request::parseMethod(std::vector<char> & msg) {
    if (strstr(msg.data(), "GET") != NULL) {
        cstrToVectorChar(this->method, "GET");
    }
    else if (strstr(msg.data(), "POST") != NULL) {
        cstrToVectorChar(this->method, "POST");
    }
    else if (strstr(msg.data(), "CONNECT") != NULL) {
        cstrToVectorChar(this->method, "CONNECT");
    }

    return true;
}

#endif