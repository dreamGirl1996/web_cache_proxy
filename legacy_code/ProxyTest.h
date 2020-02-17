// Test a whole process that the proxy handling a single request
#ifndef __PROXYTEST_H__
#define __PROXYTEST_H__

#include "utils.h"
#include "ServerSocket.h"
#include "ClientSocket.h"
#include <thread>
#include <mutex>

std::mutex mtx;

bool testProxyHelper(ServerSocket & serverSocket, 
std::string & recvFromUser, std::string & recvFromServer, int & cur_fd) {
    mtx.lock();
    if (!serverSocket.socketRecv(recvFromUser, cur_fd)) {return false;}
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
    if (!serverSocket.socketSend(recvFromServer, cur_fd)) {return false;}

    printALine(32);
    std::cout << "Proxy server received:\n" << recvFromUser << "\n";
    printALine(32);
    std::cout << "hostName:\n" << hostName << "\n";
    printALine(32);
    std::cout << "Proxy client received:\n" << recvFromServer << "\n";

    closeSockfd(cur_fd);
    mtx.unlock();
}

bool testProxy(int argc, char *argv[]) {
    std::string recvFromUser;
    std::string recvFromServer;
    // Receive request from user's browser
    ServerSocket serverSocket;
    // std::queue<std::thread> threadQueue;
    
    while (1) {
        if (!serverSocket.socketAccept()) {return false;}
        std::queue<int> new_fd_queue = serverSocket.get_new_fd_queue();
        if (new_fd_queue.size() > 0) {
            int new_fd = new_fd_queue.front();
            new_fd_queue.pop();
            
            std::thread th(testProxyHelper, serverSocket, recvFromUser, recvFromServer, new_fd);
            // threadQueue.push(th);
            // threadQueue.front().join();
            th.join();
        }
    }
    return true;
}

#endif