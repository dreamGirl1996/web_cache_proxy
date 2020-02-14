#ifndef __CLIENTSOCKET_H__
#define __CLIENTSOCKET_H__

#include "Socket.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>

class ClientSocket : public Socket {
    public:
    std::string & hostName;
    ClientSocket(std::string & hostName);
    bool setup();
    // bool talk(std::string & req, std::string &recvMsg);
    bool socketSend(std::string & req);
    bool socketRecv(std::string & recvMsg);
    void closeSocket();

    private:
    int sockfd;
};

ClientSocket::ClientSocket(std::string & hostName) : hostName(hostName), sockfd(-1) {}

bool ClientSocket::setup() {
    std::string addr = this->hostName;
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    std::memset(&hints, 0, sizeof hints);  
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr.c_str(), SERVER_PORT, &hints, &servinfo) != 0)) { // servinfo: linked list
        std::cerr << "proxy ClientSocket getaddrinfo: " << gai_strerror(rv) << "\n";
        return false;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::perror("proxy ClientSocket: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::perror("proxy ClientSocket: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        std::cerr << "proxy ClientSocket: failed to connect" << "\n";
        return false;
    }

    inet_ntop(p->ai_family, this->getInAddr((struct sockaddr *)p->ai_addr), s, sizeof s);

    freeaddrinfo(servinfo);
    this->sockfd = sockfd;

    return true;
}

// open a socket to the real server, send request 
// to the server and receive response from it
bool ClientSocket::socketSend(std::string & req) {
    int sockfd = this->sockfd;
    if ((send(sockfd, req.c_str(), strlen(req.c_str()), 0)) == -1) {
        std::perror("send");
        return false;
    }

    return true;
}

bool ClientSocket::socketRecv(std::string & recvMsg) {
    int sockfd = this->sockfd;
    int numbytes;
    char recvBuf[MAX_DATA_SIZE];
    while ((numbytes = recv(sockfd, recvBuf, MAX_DATA_SIZE - 1, 0)) > 0) {
        recvBuf[numbytes] = '\0';
        recvMsg += recvBuf;
    }
    return true;
}

void ClientSocket::closeSocket() {close(this->sockfd);}

#endif