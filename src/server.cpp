//
// Created by ldiogo on 03-05-2024.
//

#include "../inc/server.hpp"

int check_valid_port(char *port)
{
	int i = 0;
	while (port[i])
	{
		if (!isdigit(port[i]))
			return (1);
		i++;
	}
	return (0);
}

void	server::disconnect(std::vector<pollfd> fds, int fd, int i)
{
	std::cout << "Client disconnected" << std::endl;
	close(fd);
	fds.erase(fds.begin() + i);
	users.erase(fd);
	for (std::map<std::string, channel *>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		for (size_t i = 0; i < it->second->users.size(); i++)
			if (i < fds.size() && fd == it->second->users[i].getSocket())
				channels.erase(it);
	}
}

void	server::shutDown(std::vector<pollfd> fds)
{
	for (size_t i = 1; i < fds.size(); i++)
	{
		close(fds[i].fd);
	}
	close(socket_id);
	for (std::map<std::string, channel *>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		delete it->second;
	}
	std::cout << "Server shutting down" << std::endl;
}

void	server::run(user user, std::vector<pollfd> fds, int fd, int i)
{
	if(user.getStillBuilding() == 1)
	{
		user.setFinalBuffer(std::strcat(user.getFinalBuffer(), user.getBuffer()));
		if(user.getFinalBuffer()[strlen(user.getFinalBuffer()) - 1] == '\n')
		{
			user.setStillBuilding(0);
			user.setBuffer(user.getFinalBuffer());
			std::memset(user.getFinalBuffer(), 0, BUFFER_SIZE);
		}
	}
	if(user.getStillBuilding() == 0)
	{
		check_leave(this, user.getBuffer(), fd);
		login(i, this, fds, user.getBuffer());
		if (user.getStatus() == 4)
		{
			bot_timeout(this, user.getBuffer(), fd);
			user.change_nick(user.getBuffer(), fd, this);
			check_channel(user.getBuffer(), fd, this);
			user.part(this, user.getBuffer());
			if(user.getTimeStart() == 0)
			{
				check_priv(user.getBuffer(), fd, this);
				user.check_operator(user.getBuffer(), fd, this);
			}
			else
			{
				std::stringstream ss;
				ss << "BOT :You're in timeout for " << ((30 * (user.getTimeout() - 1)) - static_cast<int>(std::difftime(std::time(0), user.getTimeStart()))) << " seconds. Your message was not sent\n";
				std::string message = ss.str();
				if(user.getFromNc())
					send_user(fd, message.c_str(), message.size(), 0);
				else
				{
					// get the fd from the user on the current channel
					std::string channel = get_channel(user.getBuffer());
					std::string channelMessage = ":" + user.getNickname() + "!" + user.getUsername() + " PRIVMSG " + channel + " :" + message + "\r\n";
					send_user(fd, channelMessage.c_str(), channelMessage.size(), 0);
				}
			}
		}
	}
}

server::server(char *password, char *port)
{
	if(check_valid_port(port))
	{
		std::cerr << "Invalid port\n";
		exit(EXIT_FAILURE);
	}
	int PORT = atoi(port);
    this->password = std::string(password);
    this->server_ip = "127.0.0.1";
    this->socket_id = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        std::cerr << "Error setting socket options\n";
        close(socket_id);
        exit(EXIT_FAILURE);
    }
    if (this->socket_id == -1) {
        std::cerr << "Can't create a socket!";
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(socket_id, F_GETFL, 0);
    fcntl(socket_id, F_SETFL, flags | O_NONBLOCK);

    this->IP.sin_family = AF_INET;
    if (inet_pton(AF_INET, this->server_ip, &this->IP.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        exit(EXIT_FAILURE);
    }
    this->IP.sin_port = htons(PORT);
    if (bind(this->socket_id, (sockaddr * ) & this->IP, sizeof(this->IP)) == -1) {
        std::cerr << "Can't bind to IP/port";
        exit(EXIT_FAILURE);
    }
    if (listen(this->socket_id, SOMAXCONN) == -1) {
        std::cerr << "Can't listen!";
        exit(EXIT_FAILURE);
    }
}

server::~server()
{
    close(this->socket_id);
}
