#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Request.h"
#include "Response.h"
#include "ConnectTunnel.h"
#include "GetHandler.h"
#include <thread>
#include <functional>

void runProxy(ServerSocket & serverSocket, connect_pair_t connectPair) {
    std::vector<char> requestMsg;

    Request request;
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

    printALine(32);
    std::cout << "Request lined header:\n[" << request.reconstructLinedHeaders().data() << "]\n";

    ClientSocket clientSocket(hostName, port);

    if (strcmp(method.data(), "CONNECT") == 0) {
        if (!handleConnect(serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "GET") == 0) {
        if (!handleGet(reconReqMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "POST") == 0) {
        if (!handleGet(reconReqMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }

    closeSockfd(connectPair.first);

}


int main(int argc, char *argv[]) {
    
    try {
        // Receive request from user's browser
        ServerSocket serverSocket;
        
        while (1) {
            connect_pair_t connectPair = serverSocket.socketAccept();
            std::thread th(runProxy, std::ref(serverSocket), connectPair);
            th.join();
        }

        // connect_pair_t connectPair = serverSocket.socketAccept();
        // runProxy(serverSocket, connectPair);

        // Response response;
        // std::vector<char> msg;
        // cstrToVectorChar(msg, "nihao\r\nContent-Length: 30\r\n");
        // response.parse(msg);
        // std::cout << "content-length: " << response.getContentLength() << "\n";

    }
    catch (std::exception & e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
