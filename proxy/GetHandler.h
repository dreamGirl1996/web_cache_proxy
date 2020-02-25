#ifndef __GETHANDLER_H__
#define __GETHANDLER_H__

#include "ServerSocket.h"
#include "ClientSocket.h"
#include "Logger.h"

bool handleGet(Logger & logger, Request & request, std::vector<char> & requestMsg, 
ServerSocket & serverSocket, connect_pair_t & connectPair) {
    logger.sendingRequest(request);

    ClientSocket clientSocket(request.getHostName(), request.getPort());
    std::vector<char> responseMsg;
    clientSocket.socketSend(requestMsg);

    Response response(request.getId());
    clientSocket.socketRecv(responseMsg, response);

    if (responseMsg.size() == 0) {
        closeSockfd(connectPair.first);
        return false;
    } // commented when responseMst resized in client socketRecv

    cleanVectorChar(response.getContent());
    response.getContent() = obtainContent(responseMsg);
    std::vector<char> reconRespMsg = response.reconstruct();

    // std::cout << "\nReal Response Header: [\n" << response.getHeader().data() << "]\n"; 
    // std::cout << "\nResponse lined header: [\n" << response.reconstructLinedHeaders().data() << "]\n";
    logger.sendingResponse(response);

    serverSocket.socketSend(reconRespMsg, connectPair);

    logger.receivedResponse(response, request);

    return true;
}

#endif