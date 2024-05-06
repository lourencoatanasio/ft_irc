//
// Created by ldiogo on 03-05-2024.
//

#ifndef FT_IRC_GIT_CLIENT_HPP
#define FT_IRC_GIT_CLIENT_HPP

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
    std::string getNickname() { return nickname; }
    std::string getUsername() { return username; }
    int getSocket() { return clientSocket; }
    void setNickname(std::string newNickname) { nickname = newNickname; }
    void setUsername(std::string newUsername) { username = newUsername; }
private:
    int status;
    int socket_id;
    sockaddr_in IP;
    char buffer[BUFFER_SIZE];
    int bytesRead;
    sockaddr_in client;
    socklen_t clientSize;
    std::string nickname;
    std::string username;
};


#endif //FT_IRC_GIT_CLIENT_HPP
