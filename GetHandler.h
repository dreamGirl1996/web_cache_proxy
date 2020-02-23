#ifndef __GETHANDLER_H__
#define __GETHANDLER_H__

#include "ServerSocket.h"
#include "ClientSocket.h"

bool handleGet(Request & request, std::vector<char> & requestMsg, 
ServerSocket & serverSocket, ClientSocket & clientSocket, connect_pair_t & connectPair) {
    std::stringstream loggedReq;
    loggedReq << request.getId() << ": Requesting \"" << request.getMethod().data() << " " << \
    request.getUri().data() << " " << request.getProtocal().data() << "\" from " << \
    request.getHostName().data() << "\r\n";
    std::cout << loggedReq.str();
    
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
    std::stringstream loggedresp;
    loggedresp << response.getId() << ": Responding \"" << response.getProtocal().data() << \
    " " << response.getStatusCode().data() << " " << response.getReasonPhrase().data() << \
    "\"" << "\r\n";
    std::cout << loggedresp.str();

    if (!serverSocket.socketSend(reconRespMsg, connectPair)) {
        return false;
    }

    std::stringstream loggedresp_;
    loggedresp_ << response.getId() << ": Received \"" << response.getProtocal().data() << \
    " " << response.getStatusCode().data() << " " << response.getReasonPhrase().data() << \
    "\"" << " from " << request.getHostName().data() << "\r\n";
    std::cout << loggedresp_.str();

    return true;
}

#endif