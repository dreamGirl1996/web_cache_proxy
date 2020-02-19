#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Request.h"
#include "Response.h"
#include "ConnectTunnel.h"
#include "GetHandler.h"
#include <thread>
#include <mutex>
#include <functional>

std::mutex mtx;

void runProxy(ServerSocket & serverSocket, connect_pair_t connectPair) {
    // mtx.lock();
    std::vector<char> requestMsg;

    if (!serverSocket.socketRecv(requestMsg, connectPair)) {
        closeSockfd(connectPair.first);
        return;
    }
    if (requestMsg.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }
    Request request(requestMsg);
    
    std::vector<char> hostName = request.getHostName();
    std::vector<char> method = request.getMethod();
    std::vector<char> port = request.getPort();

    if (hostName.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }
    
    printALine(32);
    std::cout << "hostName: [" << hostName.data() << "]\n";
    std::cout << "port: [" << port.data() << "]\n";
    std::cout << "method: [" << method.data() << "]\n";
    std::cout << "\nProxy server received:\n[" << requestMsg.data() << "]\n";

    ClientSocket clientSocket(hostName, port);

    if (strcmp(method.data(), "CONNECT") == 0) {
        std::vector<char> tunnelMsg;
        if (!handleConnect(tunnelMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
        if (tunnelMsg.size() > 0) {
            std::cout << "tunnelMsg:\n[" << tunnelMsg.data() << "]\n";
        }
    }
    else if (strcmp(method.data(), "GET") == 0) {
        std::vector<char> responseMsg;
        if (!handleGet(requestMsg, responseMsg, serverSocket, clientSocket, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
    }
    else if (strcmp(method.data(), "POST") == 0) {
        // Do something
    }

    closeSockfd(connectPair.first);
    // mtx.unlock();
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
    catch (std::invalid_argument & e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
