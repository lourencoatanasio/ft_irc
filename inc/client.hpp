//
// Created by ldiogo on 03-05-2024.
//

#pragma once

#include "server.hpp"

#define BUFFER_SIZE 4096
class server;

class user
{
    public:
        user(int newSocket);
        user() : clientSocket(-1) {}
        ~user();
        int clientSocket;
        std::string getNickname() { return nickname; }
        std::string getUsername() { return username; }
        int getOpStatus() { return isOp; }
        int getSocket() { return clientSocket; }
        void setNickname(std::string newNickname) { nickname = newNickname; }
        void setUsername(std::string newUsername) { username = newUsername; }
        void setOpStatus(bool status) { isOp = status; }
		void	check_operator(char *buf, int fd, server *server);
		void	modeOperator(server *server, user &newOp, std::string flag, std::string channel);
		int		modeInvite(server *server, std::string flag, std::string channel);
		int		modeTopic(server *server, std::string flag, std::string channel);
		void	mode(server *server, char *buffer, int fd);
		int		modeCheck(server *server, std::string channel, int fd);
		int		modePassword(server *server, std::string channel, std::string flag, std::string key);
    protected:
        int status;
        int socket_id;
        sockaddr_in IP;
        char buffer[BUFFER_SIZE];
        int bytesRead;
        sockaddr_in client;
        socklen_t clientSize;
        std::string nickname;
        std::string username;
		bool	isOp;
};