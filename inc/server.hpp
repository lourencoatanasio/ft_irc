//
// Created by ldiogo on 03-05-2024.
//

#ifndef FT_IRC_GIT_SERVER_HPP
#define FT_IRC_GIT_SERVER_HPP

#include "main.hpp"
#include "client.hpp"

#define PORT 54000 // to be changed to receive as argument
#define BUFFER_SIZE 4096

class server
{
public:
    server();
    ~server();
    void connect(); // Not sure if this is needed
    void send(); // Not sure if this is needed
    void receive(); // Not sure if this is needed
    void disconnect(); // Not sure if this is needed
    int socket_id;
private:
    sockaddr_in IP;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    int clientSocket;
    sockaddr_in client;
    socklen_t clientSize;
    const char *server_ip;
    std::map <std::string, int> channels;
};


#endif //FT_IRC_GIT_SERVER_HPP
