#include "ClientSocket.h"
#include "ServerSocket.h"
#include "ProxyTest.h"
#include "ClientTest.h"
#include "ServerTest.h"

int main(int argc, char *argv[]) {
    try {
        // if (!testServer(argc, argv)) {return EXIT_FAILURE;}
        // if (!testClient(argc, argv)) {return EXIT_FAILURE;}
        if (!testProxy(argc, argv)) {return EXIT_FAILURE;}
    }
    catch (std::invalid_argument & e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
