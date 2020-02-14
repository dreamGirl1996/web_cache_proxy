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
        std::cerr << "Error occured in server.setup\n";
        return false;
    }
    if (!serverSocket.socketRecv(recvFromUser)) {
        std::cerr << "Error occured in server.socketRecv\n";
        return false;
    }
    // Parse the request from the user
    std::string hostName;
    if (!parseObjectFromString(recvFromUser, hostName, parseHostNameFromALine)) {
        std::cerr << "Error occured in server -parseObjectFromString\n";
        return false;
    }
    // End of parsing

    // Send user's request to the Web server, and receive the response from that Web server
    ClientSocket clientSocket(hostName);
    if (!clientSocket.setup()) {
        std::cerr << "Error occured in client.setup\n";
        return false;
    }
    if (!clientSocket.socketSend(recvFromUser)) {
        std::cerr << "Error occured in client.socketRecv\n";
        return false;
    }
    if (!clientSocket.socketRecv(recvFromServer)) {
        std::cerr << "Error occured in client -parseObjectFromString\n";
        return false;
    }
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

    serverSocket.closeSocket();
    clientSocket.closeSocket();

    return true;
}

#endif