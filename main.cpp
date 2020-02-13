#include "client.h"

int main(int argc, char *argv[]) {
    std::string url = "www.google.com";
    std::string req = "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close" + "\r\n\r\n";
    std::string received;
    ClientSocket clientSocket(url);
    if (!clientSocket.talkToServer(req, received)) {
        return EXIT_FAILURE;
    }
    std::cout << "proxy client: received " << received << std::endl;
    return EXIT_SUCCESS;
}
