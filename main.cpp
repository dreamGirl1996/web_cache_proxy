#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Request.h"
#include "Response.h"
#include "ConnectTunnel.h"
#include "GetHandler.h"
#include "Logger.h"
#include <thread>
#include <functional>

void runProxy(const u_long & id, 
ServerSocket & serverSocket, connect_pair_t connectPair) {
    std::vector<char> requestMsg;

    Request request(id);
    if (!serverSocket.socketRecv(requestMsg, connectPair, request)) {
        closeSockfd(connectPair.first);
        return;
    }
    if (requestMsg.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }

    std::vector<char> hostName = request.getHostName();
    std::vector<char> method = request.getMethod();
    std::vector<char> port = request.getPort();

    if (hostName.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }
    cleanVectorChar(request.getContent());
    request.getContent() = obtainContent(requestMsg);
    std::vector<char> reconReqMsg = request.reconstruct();

    ClientSocket clientSocket(hostName, port);

    // printALine(32);
    // std::cout << "Request lined header:\n[" << request.reconstructLinedHeaders().data() << "]\n";
    Logger logger;
    logger.receivedRequest(request, clientSocket.getIpAddr());

    if (strcmp(method.data(), "CONNECT") == 0) {
        if (!handleConnect(id, logger, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "GET") == 0) {
        if (!handleGet(logger, request, reconReqMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "POST") == 0) {
        if (!handleGet(logger, request, reconReqMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }

    closeSockfd(connectPair.first);
}

int main(int argc, char *argv[]) {
    try {
        u_long id = 0;
        ServerSocket serverSocket; // receive request from user's browser
        
        while (1) {
            connect_pair_t connectPair = serverSocket.socketAccept();
            id++;
            std::thread th(runProxy, std::ref(id), 
            std::ref(serverSocket), connectPair);
            th.join();
        }

        // connect_pair_t connectPair = serverSocket.socketAccept();
        // runProxy(serverSocket, connectPair);

    }
    catch (std::exception & e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
