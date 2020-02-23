#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Request.h"
#include "Response.h"
#include "ConnectTunnel.h"
#include "GetHandler.h"
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
    std::stringstream loggedReq;
    datetime_zone_t datetimeZone = getCurrentTime();
    loggedReq << request.getId() << ": \"" << method.data() << " " << \
    request.getUri().data() << " " << request.getProtocal().data() << "\" from " << \
    clientSocket.getIpAddr().data() << " @ " << std::put_time(&datetimeZone.first, "%c") << "\r\n";
    std::cout << loggedReq.str();

    if (strcmp(method.data(), "CONNECT") == 0) {
        if (!handleConnect(id, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "GET") == 0) {
        if (!handleGet(request, reconReqMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "POST") == 0) {
        if (!handleGet(request, reconReqMsg, serverSocket, clientSocket, connectPair)) {
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
