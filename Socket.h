#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>

// #define SERVER_PORT_80 "80"
// #define SERVER_PORT_443 "443"
#define MAX_DATA_SIZE 1

void closeSockfd(int & sockfd) {
    if (sockfd != -1) {close(sockfd);}
}

class Socket {
    public:
    Socket() {}
    virtual ~Socket() {}
    // virtual bool socketSend(std::string & sendMsg) = 0;
    // virtual bool socketRecv(std::string & recvMsg) = 0;

    protected:
    void *getInAddr(struct sockaddr *sa);
    virtual bool setup() = 0;
    virtual void closeSocket() = 0;
};

// get socket address, through ipv4 or ipv6
void * Socket::getInAddr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

#endif