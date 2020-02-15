#ifndef __CLIENTTEST_H__
#define __CLIENTTEST_H__

#include "utils.h"
#include "ClientSocket.h"
#include <cassert>
//#include <sstream>
#include <algorithm>

bool readInput(FILE * in, std::string & hostName, std::string & request) {
    assert(in != NULL);
    char * line = NULL;
    size_t sz;
    bool isFound = false;
    while (getline(&line, &sz, in) != -1) {
        const char * temp = line;
        std::string lineMsg = temp;
        lineMsg = trim(lineMsg);

        if (parseHostNameFromALine(isFound, lineMsg, hostName) == -1) {
            return false;
        }
        
        request += lineMsg + "\r\n";
        if (in != stdin) {
            std::cout << lineMsg << "\n";
        }
    }
    if (hostName.size() == 0 || request.size() == 0) {
        std::cerr << "client test: void information was get from testcase file" << "\n";
        return false;
    }
    request += "\r\n";
    free(line);
    return true;
}

bool closeFile(FILE * f, const char * fileNm) {
  assert(f != stdin);
  if (fclose(f) != 0) {
    std::cerr << "An exception happened when closing '" << fileNm << "'\n";
    perror("Close failure");
    return false;
  }
  return true;
}

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
    printALine(32);
    std::cout << "Begin of request\n" << req << "End of request\n";
    
    ClientSocket clientSocket(url);
    if (!clientSocket.socketSend(req)) {return false;}
    if (!clientSocket.socketRecv(recvMsg)) {return false;}
    printALine(32);
    std::cout << "Proxy client received:\n" << recvMsg << "\n";

    return true;
}

#endif