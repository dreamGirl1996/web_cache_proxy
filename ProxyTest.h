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
    if (!serverSocket.setup()) {
        std::cerr << "Error occured in setup\n";
        return false;
    }
    if (!serverSocket.socketRecv(recvFromUser)) {
        std::cerr << "Error occured in socketRecv\n";
        return false;
    }
    // Parse the request from the user
    std::string hostName;
    if (!parseObjectFromString(recvFromUser, hostName, parseHostNameFromALine)) {
        std::cerr << "Error occured in parseObjectFromString\n";
        return false;
    }
    // End of parsing

    // Send user's request to the Web server, and receive the response from that Web server
    // ClientSocket clientSocket(url);
    // if (!clientSocket.setup()) {return false;}
    // if (!clientSocket.socketSend(req)) {return false;}
    // if (!clientSocket.socketRecv(recvMsg)) {return false;}
    // Parse the response from the Web server
    // End of parsing

    // Send Web server's response back to the user's browser
    std::string sendMsg = "Hello World!";
    if (!serverSocket.socketSend(sendMsg)) {return false;}

    printALine(32);
    std::cout << "Proxy server received:\n" << recvFromUser << "\n";
    printALine(32);
    std::cout << "hostName:\n" << hostName << "\n";
    serverSocket.closeSocket();
    // clientSocket.closeSocket();

    return true;

    // std::string recvMsg;
    // std::string sendMsg = "Hello World!!!";

    // ServerSocket serverSocket;
    // if (!serverSocket.setup()) {return false;}
    // if (!serverSocket.socketRecv(recvMsg)) {return false;}
    // if (!serverSocket.socketSend(sendMsg)) {return false;}
    // serverSocket.closeSocket();

    // printALine(32);
    // std::cout << "Proxy server received:\n" << recvMsg << "\n";
    // printALine(32);
    // std::cout << "Proxy server sent:\n" << sendMsg << "\n";

    // return true;
}

#endif