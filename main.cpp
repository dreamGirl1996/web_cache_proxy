#include "Client.h"
#include "ClientTest.h"

int main(int argc, char *argv[]) {
    std::string url;  //= "www.google.com";
    std::string req;  //= "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close" + "\r\n\r\n";
    std::string received;
    
    if (argc == 1) {
        if (!readInput(stdin, url, req)) {
            return EXIT_FAILURE;
        }
    }
    else if (argc > 2) {
        std::cerr << "usage: hwk2_proxy test_file.txt" << "\n";
        return EXIT_FAILURE;
    }
    else {
        FILE * f = fopen(argv[1], "r");
        if (f == NULL) {
            std::cerr << "An exception happened when opening '" << argv[1] << "\n";
            return EXIT_FAILURE;
        }
        printALine(32);
        std::cout << "Begin of testcase\n";
        if (!readInput(f, url, req)) {
            return EXIT_FAILURE;
        }
        if (!closeFile(f, argv[1])) {
            return EXIT_FAILURE;
        }
        std::cout << "End of testcase\n";
    }
    printALine(32);
    std::cout << "hostName:\n" << url << "\n";
    std::cout << "Begin of request\n" << req << "End of request\n";
    
    
    ClientSocket clientSocket(url);
    if (!clientSocket.talkToServer(req, received)) {
        return EXIT_FAILURE;
    }
    printALine(32);
    std::cout << "Proxy client received:\n" << received << "\n";
    return EXIT_SUCCESS;
}
