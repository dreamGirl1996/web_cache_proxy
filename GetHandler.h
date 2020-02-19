#ifndef __GETHANDLER_H__
#define __GETHANDLER_H__

#include "ServerSocket.h"
#include "ClientSocket.h"

bool handleGet(std::vector<char> & requestMsg, 
ServerSocket & serverSocket, ClientSocket & clientSocket, connect_pair_t & connectPair) {
    std::vector<char> responseMsg;
    if (!clientSocket.socketSend(requestMsg)) {
        return false;
    }

    Response response;
    if (!clientSocket.socketRecv(responseMsg, response)) {
        return false;
    }

    if (responseMsg.size() == 0) {
        return false;
    } // commented when responseMst resized in client socketRecv

    int contentLength = response.getContentLength();
    std::vector<char> responseHeader = response.getHeader();
    std::cout << "\ncontentLength: [" << contentLength << "]\n";
    if (responseHeader.size() > 0) {
        std::cout << "\nHeader that the proxy client received:\n[" << responseHeader.data() << "]\n";
    }

    if (!serverSocket.socketSend(responseMsg, connectPair)) {
        return false;
    }

    return true;
}

#endif