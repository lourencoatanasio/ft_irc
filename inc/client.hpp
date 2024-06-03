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
        std::string getOldNick() { return oldNick; }
        std::string getUsername() { return username; }
        int getOpStatus() { return isOp; }
        int getSocket() { return clientSocket; }
        void setNickname(std::string newNickname) { nickname = newNickname; }
        void setOldNick(std::string newOldNick) { oldNick = newOldNick; }
        void setUsername(std::string newUsername) { username = newUsername; }
        void setStatus(int newStatus) { status = newStatus; }
        int getStatus() { return status; }
		void setFromNc(int newFromnc) { from_nc = newFromnc; }
		int getFromNc() { return from_nc; }
        void setOpStatus(bool status) { isOp = status; }
		void	check_operator(char *buf, int fd, server *server);
		void	modeOperator(server *server, user &newOp, std::string flag, std::string channel);
		int		modeInvite(server *server, std::string flag, std::string channel);
		int		modeTopic(server *server, std::string flag, std::string channel);
		int		modePassword(server *server, std::string channel, std::string flag, std::string key);
		int		modeLimit(server *server, std::string channel, std::string flag, std::string amount);
		void	setBuffer(const char *buf)  { strcpy(buffer, buf); }
		char	*getBuffer() { return buffer; }
		void	setFinalBuffer(char *buf);
		char	*getFinalBuffer() { return final_buffer; }
		void	setStillBuilding(int n) { still_building = n; }
		int		getStillBuilding() { return still_building; }
		void	mode(server *server, char *buf, int fd);
		void	kick(server *server, char *buf, int fd);
		void	invite(server *server, char *buf, int fd);
		void	topic(server *server, char *buf, int fd);
		int		opCheck(server *server, std::string channel, int fd);
		void	change_nick(char *buf, int fd, server *server);
        int     check_same_nick(std::string nick, server *server);
        int flag;
    protected:
        int status;
		int from_nc;
        int socket_id;
		int	still_building;
        sockaddr_in IP;
		char final_buffer[BUFFER_SIZE];
        char buffer[BUFFER_SIZE];
        int bytesRead;
        sockaddr_in client;
        socklen_t clientSize;
        std::string nickname;
        std::string oldNick;
        std::string username;
		bool	isOp;
};