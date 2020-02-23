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

    cleanVectorChar(response.getContent());
    response.getContent() = obtainContent(responseMsg);
    std::vector<char> reconMsg = response.reconstruct();

    // std::cout << "\nReal Response Header: [\n" << response.getHeader().data() << "]\n"; 
    std::cout << "\nResponse lined header: [\n" << response.reconstructLinedHeaders().data() << "]\n";

    if (!serverSocket.socketSend(reconMsg, connectPair)) {
        return false;
    }

    return true;
}

#endif