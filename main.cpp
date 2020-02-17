#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
#include "Request.h"
// #include "ProxyTest.h"
// #include "ClientTest.h"
// #include "ServerTest.h"
#include <thread>
#include <mutex>
#include <functional>

std::mutex mtx;

void testProxyHelper(ServerSocket & serverSocket, connect_pair_t & connectPair) {
    // mtx.lock();
    std::vector<char> recvFromUser;
    std::vector<char> recvFromServer;

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
    // printALine(32);
    // std::cout << strlen(hostName.data()) << " hostName: " << hostName.data() << "\n";
    // std::cout << strlen(port.data()) << " port: " << port.data() << "\n";
    // std::cout << strlen(method.data()) << " method: " << method.data() << "\n";
    // std::cout << "Proxy server received:\n" << recvFromUser.data() << "\n";
    ClientSocket clientSocket(hostName, port);
    if (!clientSocket.socketSend(recvFromUser)) {
        closeSockfd(connectPair.first);
        return;
    }
    if (!clientSocket.socketRecv(recvFromServer)) {
        closeSockfd(connectPair.first);
        return;
    }

    if (recvFromServer.size() == 0) {
        closeSockfd(connectPair.first);
        return;
    }
    if (!serverSocket.socketSend(recvFromServer, connectPair)) {
        closeSockfd(connectPair.first);
        return;
    }

    closeSockfd(connectPair.first);
    // mtx.unlock();
}


int main(int argc, char *argv[]) {
    
    try {
        // if (!testServer(argc, argv)) {return EXIT_FAILURE;}
        // if (!testClient(argc, argv)) {return EXIT_FAILURE;}
        // if (!testProxy(argc, argv)) {return EXIT_FAILURE;}
        // Receive request from user's browser
        ServerSocket serverSocket;
        
        while (1) {
            connect_pair_t connectPair = serverSocket.socketAccept();
            std::thread th(testProxyHelper, std::ref(serverSocket)
            , std::ref(connectPair));
            th.join();
        }

        // connect_pair_t connectPair = serverSocket.socketAccept();
        // testProxyHelper(serverSocket, connectPair);

    }
    catch (std::invalid_argument & e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
