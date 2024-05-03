//
// Created by ldiogo on 03-05-2024.
//

#ifndef FT_IRC_GIT_CLIENT_HPP
#define FT_IRC_GIT_CLIENT_HPP

#include <string>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include "server.hpp"

#define BUFFER_SIZE 4096

class user
{
public:
    user(int newSocket);
    ~user();
    void connect(); // Not sure if this is needed
    void send(); // Not sure if this is needed
    void receive(); // Not sure if this is needed
    void disconnect(); // Not sure if this is needed
    int clientSocket;
private:
    int socket_id;
    sockaddr_in IP;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    sockaddr_in client;
    socklen_t clientSize;
};


#endif //FT_IRC_GIT_CLIENT_HPP
