#ifndef __CONNECTTUNNEL_H__
#define __CONNECTTUNNEL_H__

#include "ServerSocket.h"
#include "ClientSocket.h"

// -1: error
//  0: break, or no data
//  1: continue
int tunnelTransfer(std::vector<char> & transfer, Response & response, 
fd_set & socket_set, int & client_fd, int & server_fd) {
    if (FD_ISSET(client_fd, &socket_set)) {
        try {
            struct timeval tv;
            tv.tv_sec = CLIENT_RECV_TIME_OUT;
            tv.tv_usec = 0;
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            transfer.resize(65535, 0);
            size_t recvbyte_num = recv(client_fd, &transfer.data()[0], 65535, 0);
            if (recvbyte_num <= 0) {
                return 0; // break
            }
            send(server_fd,transfer.data(), recvbyte_num, 0);
            //response.parse(transfer);
        }
        catch (std::invalid_argument &e) {
            std::cerr << e.what() << std::endl;
            return -1;
        }
    }
    else if(FD_ISSET(server_fd, &socket_set)){
        try {
            struct timeval tv;
            tv.tv_sec = SERVER_RECV_TIME_OUT;
            tv.tv_usec = 0;
            setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

            transfer.resize(65535,0);
            size_t recvbyte_num = recv(server_fd, &transfer.data()[0], 65535, 0);
            if (recvbyte_num <= 0) {
                return 0;  // break
            }
            send(client_fd, transfer.data(), recvbyte_num,0);
            //response.parse(transfer);
        }
        catch (std::invalid_argument & e) {
            std::cerr << e.what() << std::endl;
            return -1;
        }
    }
    return 1;
}

bool handleConnect(const u_long & id, ServerSocket & serverSocket, ClientSocket & clientSocket, connect_pair_t & connectPair) {
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
                //receive from client
                std::vector<char> transfer;
                Request request(id);
                Response response(id);
                int r = tunnelTransfer(transfer, response, socket_set, client_fd, server_fd);
                if (r == 0) {
                    // break;
                    std::cout << "no data\n";
                    return true;
                }
                else if (r == -1) {
                    std::cerr << "Error occured in tunnelTransfer\n";
                    return false;
                }
                
        }//timeout
    }
    
    return true;  // actually it is a timeout
}

// if (FD_ISSET(client_fd, &socket_set)) {
//     if (!serverSocket.socketRecv(transfer, connectPair, request)) {
//         return false;
//     }
//     if (!clientSocket.socketSend(transfer)) {
//         return false;
//     }
// }
// else if(FD_ISSET(server_fd, &socket_set)){
//     if (!clientSocket.socketRecv(transfer, response)) {
//         return false;
//     }
//     if (!serverSocket.socketSend(transfer, connectPair)) {
//         return false;
//     }
// }

#endif