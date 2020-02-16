// Test a whole process that the proxy handling a single request
#ifndef __PROXYTEST_H__
#define __PROXYTEST_H__

#include "utils.h"
#include "ServerSocket.h"
#include "ClientSocket.h"

bool testProxy(int argc, char *argv[]) {
    std::string recvFromUser;
    std::string recvFromServer;
    // Receive request from user's browser
    ServerSocket serverSocket;
    if (!serverSocket.socketAccept()) {return false;}
    if (!serverSocket.socketRecv(recvFromUser)) {return false;}
    // Parse the request from the user
    std::string hostName;
    if (!parseObjectFromString(recvFromUser, hostName, parseHostNameFromALine)) {return false;}
    // End of parsing

    // Send user's request to the Web server, and receive the response from that Web server
    ClientSocket clientSocket(hostName);
    if (!clientSocket.socketSend(recvFromUser)) {return false;}
    if (!clientSocket.socketRecv(recvFromServer)) {return false;}
    // Parse the response from the Web server
    // End of parsing

    // Send Web server's response back to the user's browser
    // std::string sendMsg = "Hello World!";
    if (!serverSocket.socketSend(recvFromServer)) {return false;}

    printALine(32);
    std::cout << "Proxy server received:\n" << recvFromUser << "\n";
    printALine(32);
    std::cout << "hostName:\n" << hostName << "\n";
    printALine(32);
    std::cout << "Proxy client received:\n" << recvFromServer << "\n";

    return true;
}

#endif