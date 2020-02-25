#ifndef __GETHANDLER_H__
#define __GETHANDLER_H__

#include "ServerSocket.h"
#include "ClientSocket.h"
#include "Logger.h"

bool handleGet(Logger & logger, Request & request, std::vector<char> & requestMsg, 
ServerSocket & serverSocket, ClientSocket & clientSocket, connect_pair_t & connectPair) {
    logger.sendingRequest(request);

    std::vector<char> responseMsg;
    if (!clientSocket.socketSend(requestMsg)) {
        return false;
    }

    Response response(request.getId());
    if (!clientSocket.socketRecv(responseMsg, response)) {
        return false;
    }

    if (responseMsg.size() == 0) {
        return false;
    } // commented when responseMst resized in client socketRecv

    cleanVectorChar(response.getContent());
    response.getContent() = obtainContent(responseMsg);
    std::vector<char> reconRespMsg = response.reconstruct();

    // std::cout << "\nReal Response Header: [\n" << response.getHeader().data() << "]\n"; 
    // std::cout << "\nResponse lined header: [\n" << response.reconstructLinedHeaders().data() << "]\n";
    logger.sendingResponse(response);

    if (!serverSocket.socketSend(reconRespMsg, connectPair)) {
        return false;
    }

    logger.receivedResponse(response, request);

    return true;
}

#endif