//
// Created by ldiogo on 03-05-2024.
//

#pragma once

#include "main.hpp"
#include "client.hpp"
#include "channel.hpp"

#define PORT 10000 // to be changed to receive as argument
#define BUFFER_SIZE 4096

class channel;
class user;

class server
{
    public:
        server(char *password);
        ~server();
        void connect(); // Not sure if this is needed
        void send(); // Not sure if this is needed
        void receive(); // Not sure if this is needed
        void disconnect(); // Not sure if this is needed
        int socket_id;
        std::map<std::string, channel*> channels;
        std::map<int, user> users;
        std::string getPass() { return password; }
    private:
        sockaddr_in IP;
        char buffer[BUFFER_SIZE];
        int bytesRead;
        int clientSocket;
        sockaddr_in client;
        socklen_t clientSize;
        const char *server_ip;
        std::string password;
};