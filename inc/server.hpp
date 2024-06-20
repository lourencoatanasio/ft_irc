//
// Created by ldiogo on 03-05-2024.
//

#pragma once

#include "main.hpp"
#include "client.hpp"
#include "channel.hpp"

#define BUFFER_SIZE 4096

class channel;
class user;

class server
{
    public:
        server(char *password, char *port);
        ~server();
        void send();
        int socket_id;
        std::map<std::string, channel*> channels;
        std::map<int, user> users;
        std::string getPass() { return password; }
		void		run(user *user, std::vector<pollfd> &fds, int fd, int i);
	void    	disconnect(std::vector<pollfd> &fds, int fd, int i);
	void		shutDown(std::vector<pollfd> &fds);
	void		priv_n_op(user *sUser, int fd);
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
