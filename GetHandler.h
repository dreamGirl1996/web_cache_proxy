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

    if (response.getDatetimeVectorChar().size() > 0) {
        datetime_zone_t dz = getDatetimeAndZone(response.getDatetimeVectorChar());
        std::cout << "\ndatetime: " << std::put_time(&dz.first, "%c") << "\n";
        std::cout << "\ntimeZone: " << dz.second.data() << "\n";
    }
    std::cout << "\nResponse header: [\n" << response.reconstructLinedHeaders().data() << "]\n";

    if (!serverSocket.socketSend(responseMsg, connectPair)) {
        return false;
    }

    return true;
}

#endif