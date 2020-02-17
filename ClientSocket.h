#ifndef __CLIENTSOCKET_H__
#define __CLIENTSOCKET_H__

#include "Socket.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
// #include <unistd.h>
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
// #include <sys/socket.h>
// #include <arpa/inet.h>
#include <sys/time.h>
// #include <unistd.h>	 // usleep
#include <exception>
#include <sstream>

#define CLIENT_RECV_TIME_OUT 0.5

class ClientSocket : public Socket {
    public:
    std::string & hostName;
    std::string & port; 
    ClientSocket(std::string & hostName, std::string & port);
    virtual ~ClientSocket();
    virtual bool socketSend(std::string & sendMsg);
    virtual bool socketRecv(std::string & recvMsg);
    
    protected:
    virtual bool setup();
    virtual void closeSocket();

    private:
    int sockfd;
};

ClientSocket::ClientSocket(std::string & hostName, std::string & port) : \
Socket(), hostName(hostName), port(port), sockfd(-1) {
    if(!this->setup()) {
        std::stringstream ess;
        ess << __func__;
        throw std::invalid_argument(ess.str());
    }
}

ClientSocket::~ClientSocket() {this->closeSocket();}

bool ClientSocket::setup() {
    std::string addr = this->hostName;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    std::memset(&hints, 0, sizeof hints);  
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr.c_str(), this->port.c_str(), &hints, &servinfo) != 0)) { // servinfo: linked list
        std::cerr << "proxy ClientSocket getaddrinfo: " << gai_strerror(rv) << "\n";
        std::cerr << "hostName: " << addr << "\n";
        return false;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((this->sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::perror("proxy ClientSocket: socket");
            continue;
        }
        if (connect(this->sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(this->sockfd);
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

    return true;
}

// open a socket to the real server, send request 
// to the server and receive response from it
bool ClientSocket::socketSend(std::string & sendMsg) {
    if ((send(this->sockfd, sendMsg.c_str(), strlen(sendMsg.c_str()), 0)) == -1) {
        std::perror("send");
        return false;
    }

    return true;
}

// Begin citation
// https://www.binarytides.com/receive-full-data-with-recv-socket-function-in-c/
bool ClientSocket::socketRecv(std::string & recvMsg) {
    int numbytes;
    char recvBuf[MAX_DATA_SIZE];

    struct  timeval begin, now;
    double timeDiff;
    gettimeofday(&begin, NULL);
    while (1) {
        gettimeofday(&now, NULL);
        timeDiff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
        // If you got some data, then break after timeout
        if (recvMsg.size() > 0 && timeDiff > CLIENT_RECV_TIME_OUT) {
            break;
        }
        // If you got no data at all, wait a little longer, twice the timeout
        else if (timeDiff > CLIENT_RECV_TIME_OUT * 2) {break;}
        if ((numbytes = recv(this->sockfd, recvBuf, MAX_DATA_SIZE - 1, MSG_DONTWAIT)) != -1) {
            recvBuf[numbytes] = '\0';
            recvMsg += recvBuf;
            // gettimeofday(&begin , NULL);
        }
        else {
            // If nothing was received then we want to wait a little before trying again, 0.1 seconds
            // usleep(100000); // original
            usleep(1000);
        }
    }
    return true;
}
// End of citation

void ClientSocket::closeSocket() {
    // if (this->sockfd != -1) {close(this->sockfd);}
    closeSockfd(this->sockfd);
}

#endif