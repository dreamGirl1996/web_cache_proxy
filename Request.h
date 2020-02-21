#ifndef __REQUEST_H__
#define __REQUEST_H__

#include "utils.h"
#include "HttpParser.h"
// #include <iostream>
// #include <vector>
// #include <cstring>
#include <cassert>

class Request : public HttpParser {
    public:
    Request();
    virtual void clearRequest();
    virtual bool parse(std::vector<char> & msg);
    virtual std::vector<char> & getHostName() {return this->hostName;}
    virtual std::vector<char> & getPort() {return this->port;}
    virtual std::vector<char> & getMethod() {return this->method;}

    protected:
    std::vector<char> hostName;
    std::vector<char> port;
    std::vector<char> method; 
    virtual bool parseHostName(std::vector<char> & msg);
    virtual bool parseMethod(std::vector<char> & msg);
};

Request::Request() : HttpParser(), hostName(), port(), method() {}

bool Request::parse(std::vector<char> & msg) {
    msg.push_back('\0');
    try {
        if (this->header.size() == 0) {
            this->parseHeader(msg);
        }
        if (this->hostName.size() == 0) {
            this->parseHostName(msg);
        }
        if (this->hostName.size() > 0) {
            assert(this->port.size() > 0);
        }
        if (this->method.size() == 0) {
            this->parseMethod(msg);
        }
        if (this->contentLength < 0) {
            this->parseContentLength(msg);
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

void Request::clearRequest() {
    this->clear();
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
        tempHostName.push_back('\0');
        if (strstr(tempHostName.data(), ":") != NULL) {
            tempHostName.pop_back(); // pop '\0'
            while (tempHostName.back() != ':') {
                this->port.push_back(tempHostName[tempHostName.size()-1]);
                tempHostName.pop_back(); 
            }
            tempHostName.pop_back();  // pop ':'
            if (this->port.size() > 0) {
                std::reverse(this->port.begin(), this->port.end());
                this->port.push_back('\0');
            }
            tempHostName.push_back('\0');
        }
        else {
            // tempHostName.pop_back(); // pop '\0' // commented if tempHost already pushed '\0'
            cstrToVectorChar(this->port, "80");
        }
        this->hostName = tempHostName;
    }
    // std::cout << this->hostName.data() << "\n";
    // std::cout << this->port.data() << "\n";

    // for (size_t i = 0; i < tempHostName.size(); i++) {
    //     if (tempHostName[i] == ':') {
    //         if (i < tempHostName.size() - 1 && tempHostName[i+1] == '4') {
    //             cstrToVectorChar(this->port, "443");
    //         }
    //         tempHostName.pop_back();
    //     }
    // }
    
    // if (tempHostName.size() > 0) {
    //     tempHostName.push_back('\0');
    //     this->hostName = tempHostName;
    // }
    
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