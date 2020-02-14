#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT "80"
#define MAX_DATA_SIZE 1024

class Socket {
    // public:
    // virtual bool talk(std::string & req, std::string &recvMsg) = 0;
    // virtual bool talk() = 0;

    protected:
    void *getInAddr(struct sockaddr *sa);
};

// get socket address, through ipv4 or ipv6
void * Socket::getInAddr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

#endif