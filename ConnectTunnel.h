#ifndef __CONNECTTUNNEL_H__
#define __CONNECTTUNNEL_H__

#include "ServerSocket.h"
#include "ClientSocket.h"

bool handleConnect(ServerSocket & serverSocket, ClientSocket & clientSocket, connect_pair_t & connectPair) {
    // std::cout<<"method is connnect now"<<std::endl;
    std::vector<char> responseForConnect;
    cstrToVectorChar(responseForConnect, "HTTP/1.1 200 OK\r\n\r\n");
    serverSocket.socketSend(responseForConnect, connectPair);

    int client_fd = connectPair.first;
    int server_fd = clientSocket.getWebServerSockfd();

    fd_set socket_set;
    while(1) {
        FD_ZERO(&socket_set);
        FD_SET(client_fd, &socket_set);
        FD_SET(server_fd, &socket_set);
        int max_set=(client_fd>server_fd)? client_fd : server_fd;
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        switch(select(max_set + 1, &socket_set, nullptr, nullptr, &timeout)) {
            case 0:
                std::cout << "select nothing" << std::endl;
                return true;
            case -1:
                std::cerr << "wrong select" << std::endl;
                return false;
            default:
                //receive from clien
                std::vector<char> transfer;
                Response response;
                if (FD_ISSET(client_fd, &socket_set)) {
                    try {
                        transfer.resize(65535, 0);
                        size_t recvbyte_num = recv(client_fd, &transfer.data()[0], 65535, 0);
                        if (recvbyte_num == 0){
                            break;
                        }
                        send(server_fd,transfer.data(), recvbyte_num, 0);
                        //response.parse(transfer);
                    }
                    catch (std::exception &e){
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }
                else if(FD_ISSET(server_fd, &socket_set)){
                    try {
                        transfer.resize(65535,0);
                        size_t recvbyte_num = recv(server_fd, &transfer.data()[0], 65535, 0);
                        if (recvbyte_num == 0){
                            break;
                        }
                        send(client_fd, transfer.data(), recvbyte_num,0);
                        //response.parse(transfer);
                    }
                    catch (std::exception &e){
                        std::cerr << e.what() << std::endl;
                        return false;
                    }
                }
                // std::vector<char> responseHeader = response.getHeader();
                // if (responseHeader.size() > 0) {
                //     std::cout << "\nHeader that the proxy client received:\n[" << responseHeader.data() << "]\n";
                // }
                
        }//timeout
    }
    
    return true;  // actually it is a timeout
}

#endif