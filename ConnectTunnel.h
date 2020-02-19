#ifndef __CONNECTTUNNEL_H__
#define __CONNECTTUNNEL_H__

#include "ServerSocket.h"
#include "ClientSocket.h"

bool talkInTunnel(std::vector<char> & tunnelMsg, 
ServerSocket & serverSocket, ClientSocket & clientSocket,
connect_pair_t & connectPair, bool fromUser) {
    if (fromUser) {
        if (!serverSocket.socketRecv(tunnelMsg, connectPair)) {
            return false;
        }
        clientSocket.socketSend(tunnelMsg);
    }
    else {
        Response tunnelResp;
        if (!clientSocket.socketRecv(tunnelMsg, tunnelResp)) {
            return false;
        }
        serverSocket.socketSend(tunnelMsg, connectPair);
    }
    return true;
}

bool handleConnect(std::vector<char> & tunnelMsg, 
ServerSocket & serverSocket, ClientSocket & clientSocket, connect_pair_t & connectPair) {
    std::vector<char> msg_connect;
    cstrToVectorChar(msg_connect, "HTTP/1.1 200 OK\r\n\r\n");

    if (!serverSocket.socketSend(msg_connect, connectPair)) {
        return false;
    }

    int client_fd = connectPair.first;
    int server_fd = clientSocket.getWebServerSockfd();

    fd_set socket_set;
    
    FD_ZERO(&socket_set);
    FD_SET(client_fd, &socket_set);  // client_fd
    FD_SET(server_fd, &socket_set);  // server_fd
    int max_set = (client_fd > server_fd)? client_fd : server_fd;
    switch(select(max_set + 1, &socket_set, nullptr, nullptr, nullptr)) {
        case 0:
            std::cerr << "select nothing" << std::endl;
            break;
        case -1:
            std::cerr << "wrong select" << std::endl;
            break;
        default:
            //receive from client
            if (FD_ISSET(client_fd, &socket_set)) {
                std::cout << "tunnel: receive from client to server" << std::endl;
                if (!talkInTunnel(
                    tunnelMsg, serverSocket, clientSocket, connectPair, true)) {
                    return false;
                }
            }
            else if (FD_ISSET(server_fd, &socket_set)) {//receive  from  server
                std::cout << "tunnel: receive from server to client" << std::endl;
                if (!talkInTunnel(
                    tunnelMsg, serverSocket, clientSocket, connectPair, false)) {
                    return false;
                }
            }
            return true;
    }//timeout
    return false;
}

#endif