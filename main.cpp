#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_PORT "80"
#define MAX_DATA_SIZE 1024

// get socket address, through ipv4 or ipv6
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// open a socket to the real server, send request 
// to the server and receive response from it
bool apply(std::string & addr, std::string & req, std::string &recvMsg) {
    int sockfd, numbytes;
    char recvBuf[MAX_DATA_SIZE];
    struct  addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    std::memset(&hints, 0, sizeof hints);  
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr.c_str(), SERVER_PORT, &hints, &servinfo) != 0)) { // servinfo: linked list
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return false;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::perror("proxy client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::perror("proxy client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        std::cerr << "proxy client: failed to connect" << std::endl;
        return false;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    // std::cout << "proxy client: connecting to " << s << std::endl;

    freeaddrinfo(servinfo);

    if ((send(sockfd, req.c_str(), strlen(req.c_str()), 0)) == -1) {
        std::perror("send");
        return false;
    }
    // std::cout << "proxy client: sending " << req.c_str() << std::endl;

    while ((numbytes = recv(sockfd, recvBuf, MAX_DATA_SIZE - 1, 0)) > 0) {
        recvBuf[numbytes] = '\0';
        recvMsg += recvBuf;
    }

    // if ((numbytes = recv(sockfd, recvBuf, MAX_DATA_SIZE - 1, 0)) == -1) {
    //     std::perror("recv");
    //     return false;
    // }
    // recvBuf[numbytes] = '\0';
    // recvMsg = recvBuf;

    close(sockfd);

    return true;
}

int main(int argc, char *argv[]) {
    std::string url = "www.google.com";
    std::string req = "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close" + "\r\n\r\n";
    std::string received;
    if (!apply(url, req, received)) {
        return EXIT_FAILURE;
    }
    std::cout << "proxy client: received " << received << std::endl;
    return EXIT_SUCCESS;
}
