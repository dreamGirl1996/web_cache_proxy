#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "utils.h"
#include <iostream>
#include <vector>
#include <cstring>

class Request {
    public:
    Request(std::vector<char> & msg);
    std::vector<char> & getMsg() {return this->msg;}
    std::vector<char> & getHostName() {return this->hostName;}
    std::vector<char> & getPort() {return this->port;}
    std::vector<char> & getMethod() {return this->method;}

    private:
    std::vector<char> & msg;
    std::vector<char> hostName;
    std::vector<char> port;
    std::vector<char> method; 
    bool parseHostName();
    bool parseMethod();
};

Request::Request(std::vector<char> & msg) : msg(msg), hostName(), method() {
    this->parseHostName();
    this->parseMethod();
}

bool Request::parseHostName() {
    cleanVectorChar(this->hostName);
    cleanVectorChar(this->port);
    const char *pch = strstr(this->msg.data(), "Host:");
    if (pch != NULL) {
        pch = pch + 5;
        skipSpace(&pch);
        while (*pch != '\n') {
            this->hostName.push_back(*pch);
            pch = pch + 1;
        }
    }
    if (this->hostName.size() > 0) {
        cstrToVectorChar(this->port, "80");
        this->hostName.pop_back();  // remove '\r'
    }
    for (size_t i = 0; i < this->hostName.size(); i++) {
        if (this->hostName[i] == ':') {
            cstrToVectorChar(this->port, "443");
            while(this->hostName.back() != ':') {
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
    // const char *pch = NULL;
    if (strstr(this->msg.data(), "GET") != NULL) {
        // for (size_t i = 0; i < strlen("GET"); i++) {
        //     this->method.push_back(*pch);
        //     pch = pch + 1;
        // }
        cstrToVectorChar(this->method, "GET");
    }
    else if (strstr(this->msg.data(), "POST") != NULL) {
        // for (size_t i = 0; i < strlen("POST"); i++) {
        //     this->method.push_back(*pch);
        //     pch = pch + 1;
        // }
        cstrToVectorChar(this->method, "POST");
    }
    else if (strstr(this->msg.data(), "CONNECT") != NULL) {
        // for (size_t i = 0; i < strlen("CONNECT"); i++) {
        //     this->method.push_back(*pch);
        //     pch = pch + 1;
        // }
        cstrToVectorChar(this->method, "CONNECT");
    }

    return true;
}

#endif