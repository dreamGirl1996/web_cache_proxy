#ifndef __SERVERSOCKET_H__
#define __SERVERSOCKET_H__

#include "Socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>	 // usleep
#include <exception>
#include <sstream>

#define PORT "12345"  // the port users will be connecting to
#define BACKLOG 1   // how many pending connections queue will hold
#define SERVER_RECV_TIME_OUT 1  // max waiting seconds for receiving

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

class ServerSocket : public Socket {
    public:
    ServerSocket();
    virtual ~ServerSocket();    
    virtual bool socketRecv(std::string & recvMsg);
    virtual bool socketSend(std::string & sendMsg);

    protected:
    virtual bool setup();
    virtual void closeSocket();

    private:
    struct sockaddr_storage their_addr;
    int sockfd;
    int new_fd;
};

ServerSocket::ServerSocket() : Socket(), sockfd(-1), new_fd(-1) {
    if(!this->setup()) {
        std::stringstream ess;
        ess << __func__;
        throw std::invalid_argument(ess.str());
    }
}

ServerSocket::~ServerSocket() {this->closeSocket();}

bool ServerSocket::setup() {
    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        std::cerr << "proxy ServerSocket: getaddrinfo " << gai_strerror(rv) << "\n";
        return false;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::perror("proxy ServerSocket: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            std::perror("proxy ServerSocket: setsockopt");
            return false;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::perror("proxy ServerSocket: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        std::cerr << "proxy ServerSocket: failed to bind\n";
        return false;
    }

    if (listen(sockfd, BACKLOG) == -1) {
        std::perror("proxy ServerSocket: listen");
        return false;
    }
    
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        std::perror("proxy ServerSocket: sigaction");
        return false;
    }

    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    while (new_fd == -1) {
        std::perror("accept");
        continue;
    }

    this->their_addr = their_addr;
    this->new_fd = new_fd;

    return true;
}

// Begin citation
// https://www.binarytides.com/receive-full-data-with-recv-socket-function-in-c/
bool ServerSocket::socketRecv(std::string & recvMsg) {
    their_addr = this->their_addr;
    new_fd = this->new_fd;
    char s[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family,
        this->getInAddr((struct sockaddr *)&their_addr), s, sizeof s);

    // close(sockfd); // child doesn't need the listener
    int numbytes;
    char recvBuf[MAX_DATA_SIZE];

    struct  timeval begin, now;
    double timeDiff;
    gettimeofday(&begin, NULL);
    while (1) { 
        gettimeofday(&now, NULL);
        timeDiff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
        // If you got some data, then break after timeout
        if (recvMsg.size() > 0 && timeDiff > SERVER_RECV_TIME_OUT) {
            break;
        }
        // If you got no data at all, wait a little longer, twice the timeout
        else if (timeDiff > SERVER_RECV_TIME_OUT * 2) {break;}
        if ((numbytes = recv(new_fd, recvBuf, MAX_DATA_SIZE - 1, MSG_DONTWAIT)) != -1) {
            recvBuf[numbytes] = '\0';
            recvMsg += recvBuf;
            // gettimeofday(&begin , NULL);
        }
        else {
            // If nothing was received then we want to wait a little before trying again, 0.1 seconds
            usleep(100000);
        }
    }
    // close(new_fd);

    return true;
}
// End of citation

bool ServerSocket::socketSend(std::string & sendMsg) {
    their_addr = this->their_addr;
    new_fd = this->new_fd;
    char s[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family,
        this->getInAddr((struct sockaddr *)&their_addr), s, sizeof s);

    // close(sockfd); // child doesn't need the listener

    if (send(new_fd, sendMsg.c_str(), strlen(sendMsg.c_str()), 0) == -1) {
        std::perror("send");
    }
    // close(new_fd);

    return true;
}

void ServerSocket::closeSocket() {
    if (this->sockfd != -1) {close(this->sockfd);}
    if (this->new_fd != -1) {close(this->new_fd);}
}

#endif