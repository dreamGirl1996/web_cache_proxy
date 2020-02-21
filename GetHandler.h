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

    std::vector<char> datetimeVectorChar = response.getDatetimeVectorChar();
    std::cout << "\ncontentLength: [" << response.getContentLength() << "]\n";
    if (datetimeVectorChar.size() > 0) {
        std::cout << "datetimeVectorChar: [" << datetimeVectorChar.data() << "]\n";
        std::tm datetime = getDatetime(datetimeVectorChar);
        std::cout << "datetime: [" << std::put_time(&datetime, "%c") << "]\n";
    }
    std::cout << "Response first line: [" << response.getFirstLine().data() << "]\n";
    if (response.getHeader().size() > 0) {
        std::cout << "Response header:\n[" << response.getHeader().data() << "]\n";
    }

    if (!serverSocket.socketSend(responseMsg, connectPair)) {
        return false;
    }

    return true;
}

#endif