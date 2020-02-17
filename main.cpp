#include "utils.h"
#include "ClientSocket.h"
#include "ServerSocket.h"
// #include "ProxyTest.h"
// #include "ClientTest.h"
// #include "ServerTest.h"
#include <thread>
#include <mutex>
#include <functional>

// std::mutex mtx;

void cleanHeaders(std::string & hostName, std::string & method) {
    hostName = "";
    method = "";
}

void testProxyHelper(ServerSocket & serverSocket, connect_pair_t & connectPair) {
    // mtx.lock();
    std::string recvFromUser;
    std::string recvFromServer;

    std::string hostName;
    std::string method;
    std::string port;
    while(hostName.size() == 0 || method.size() == 0) {
        while (recvFromUser.size() == 0) {
            if (!serverSocket.socketRecv(recvFromUser, connectPair)) {return;}
        }
        // Parse the request from the user
        if (!parseObjectFromString(recvFromUser, hostName, parseHostNameFromALine)) {
            cleanHeaders(hostName, method);
            return;
        }
        if (!parseObjectFromString(recvFromUser, method, parseMethodFromALine)) {
            cleanHeaders(hostName, method);
            return;
        }
    }    
    // End of parsing
    // std::cout << method << "\n";
    if (method == "GET" || method == "CONNECT") {
        // Send user's request to the Web server, and receive the response from that Web server
        if (method == "GET") {
            port = SERVER_PORT_80;
        }
        else if (method == "CONNECT") {
            port = SERVER_PORT_443;
        }
        ClientSocket clientSocket(hostName, port);
        if (!clientSocket.socketSend(recvFromUser)) {return;}
        if (!clientSocket.socketRecv(recvFromServer)) {return;}
        // Parse the response from the Web server
        // End of parsing
        printALine(32);
        std::cout << "Proxy server received:\n" << recvFromUser << "\n";
        printALine(32);
        std::cout << "Proxy server received: \n" << recvFromServer << "\n";

        // Send Web server's response back to the user's browser
        // std::string sendMsg = "Hello World!";
        if (!serverSocket.socketSend(recvFromServer, connectPair)) {return;}
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
        // std::thread th(testProxyHelper, std::ref(serverSocket)
        // , std::ref(connectPair));
        // th.join();

        ////// //////
        // if (!serverSocket.socketAccept()) {return EXIT_FAILURE;}
        // connect_pair_queue_t connectPairQueue = serverSocket.getConnectPairQueue();
        // if (connectPairQueue.size() > 0) {
        //     connect_pair_t connectPair = connectPairQueue.front();
        //     connectPairQueue.pop();
            
        //     std::thread th(testProxyHelper, std::ref(serverSocket), 
        //     std::ref(recvFromUser), std::ref(recvFromServer), std::ref(connectPair));
        //     th.join();
        // }
        ////// //////
    }
    catch (std::invalid_argument & e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
