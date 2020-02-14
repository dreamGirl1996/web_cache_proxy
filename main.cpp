#include "ClientSocket.h"
#include "ServerSocket.h"
#include "ClientTest.h"

bool testClient(int argc, char *argv[]) {
    std::string url;  //= "www.google.com";
    std::string req;  //= "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close" + "\r\n\r\n";
    std::string recvMsg;
    
    if (argc == 1) {
        if (!readInput(stdin, url, req)) {return false;}
    }
    else if (argc > 2) {
        std::cerr << "usage: hwk2_proxy test_file.txt" << "\n";
        return false;
    }
    else {
        FILE * f = fopen(argv[1], "r");
        if (f == NULL) {
            std::cerr << "An exception happened when opening '" << argv[1] << "\n";
            return false;
        }
        printALine(32);
        std::cout << "Begin of testcase\n";
        if (!readInput(f, url, req)) {return false;}
        if (!closeFile(f, argv[1])) {return false;}
        std::cout << "End of testcase\n";
    }
    printALine(32);
    std::cout << "hostName:\n" << url << "\n";
    std::cout << "Begin of request\n" << req << "End of request\n";
    
    ClientSocket clientSocket(url);
    if (!clientSocket.setup()) {return false;}
    if (!clientSocket.socketSend(req)) {return false;}
    if (!clientSocket.socketRecv(recvMsg)) {return false;}
    clientSocket.closeSocket();
    printALine(32);
    std::cout << "Proxy client received:\n" << recvMsg << "\n";

    return true;
}

bool testServer(int argc, char *argv[]) {
    std::string recvMsg;
    std::string sendMsg = "Hello World!!!";

    ServerSocket serverSocket;
    if (!serverSocket.setup()) {return false;}
    if (!serverSocket.socketRecv(recvMsg)) {return false;}
    if (!serverSocket.socketSend(sendMsg)) {return false;}
    serverSocket.closeSocket();

    std::cout << "Proxy server sent:\n" << sendMsg << "\n";
    while (recvMsg.size() == 0) {}
    std::cout << "Proxy server received:\n" << recvMsg << "\n";

    return true;
}

int main(int argc, char *argv[]) {
    if (!testServer(argc, argv)) {return EXIT_FAILURE;}
    return EXIT_SUCCESS;
}
