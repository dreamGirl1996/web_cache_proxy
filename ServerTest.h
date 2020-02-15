#ifndef __SERVERTEST_H__
#define __SERVERTEST_H__

#include "utils.h"
#include "ServerSocket.h"
//#include <cassert>
//#include <sstream>
//#include <algorithm>

bool testServer(int argc, char *argv[]) {
    std::string recvMsg;
    std::string sendMsg = "Hello World!!!";

    ServerSocket serverSocket;
    if (!serverSocket.socketRecv(recvMsg)) {return false;}
    if (!serverSocket.socketSend(sendMsg)) {return false;}

    printALine(32);
    std::cout << "Proxy server received:\n" << recvMsg << "\n";
    printALine(32);
    std::cout << "Proxy server sent:\n" << sendMsg << "\n";

    return true;
}

#endif