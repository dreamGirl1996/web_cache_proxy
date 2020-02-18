#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Request.h"
#include "Response.h"
#include "ConnectTunnel.h"
// #include "ProxyTest.h"
// #include "ClientTest.h"
// #include "ServerTest.h"
#include <thread>
#include <mutex>
#include <functional>

std::mutex mtx;

void testProxyHelper(ServerSocket & serverSocket, connect_pair_t connectPair) {
    // mtx.lock();
    std::vector<char> recvFromUser;
    std::vector<char> recvFromServer;
    std::vector<char> tunnelMsg;

    if (!serverSocket.socketRecv(recvFromUser, connectPair)) {
        closeSockfd(connectPair.first);
        return;
    }
    if (recvFromUser.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }
    Request request(recvFromUser);
    
    std::vector<char> hostName = request.getHostName();
    std::vector<char> method = request.getMethod();
    std::vector<char> port = request.getPort();

    if (hostName.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }

    // if (method.size() == 0 || strcmp(method.data(), "GET") != 0) {
    //     closeSockfd(connectPair.first);
    //     return;
    // }
    
    printALine(32);
    std::cout << "hostName: [" << hostName.data() << "]\n";
    std::cout << "port: [" << port.data() << "]\n";
    std::cout << "method: [" << method.data() << "]\n";
    std::cout << "\nProxy server received:\n[" << recvFromUser.data() << "]\n";

    ClientSocket clientSocket(hostName, port);
    if (!clientSocket.socketSend(recvFromUser)) {
        closeSockfd(connectPair.first);
        return;
    }

    if (strcmp(method.data(), "CONNECT") == 0) {
        if (!handleConnect(tunnelMsg, serverSocket, clientSocket, method, connectPair)) {
            closeSockfd(connectPair.first);
            return;
        }
        if (tunnelMsg.size() > 0) {
            std::cout << "tunnelMsg:\n[" << tunnelMsg.data() << "]\n";
        }
    }

    Response response;
    if (!clientSocket.socketRecv(recvFromServer, response)) {
        closeSockfd(connectPair.first);
        return;
    }

    if (recvFromServer.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }

    int contentLength = response.getContentLength();
    // std::vector<char> responseHeader = response.getHeader();
    std::cout << "\ncontentLength: [" << contentLength << "]\n";
    // if (responseHeader.size() > 0) {
    //     std::cout << "\nHeader that the proxy client received:\n[" << responseHeader.data() << "]\n";
    // }

    if (!serverSocket.socketSend(recvFromServer, connectPair)) {
        closeSockfd(connectPair.first);
        return;
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
            std::thread th(testProxyHelper, std::ref(serverSocket), connectPair);
            th.join();
        }

        // connect_pair_t connectPair = serverSocket.socketAccept();
        // testProxyHelper(serverSocket, connectPair);

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
