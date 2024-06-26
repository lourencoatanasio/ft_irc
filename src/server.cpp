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

void    server::disconnect(std::vector<pollfd> &fds, int fd, int i)
{
    std::cout<< RED << "Client disconnected" << NC << std::endl;
    close(fd);
    fds.erase(fds.begin() + i);
    for (std::map<std::string, channel *>::iterator it = channels.begin(); it != channels.end(); it++) {
        if (it->second->users.find(fd) != it->second->users.end()) {
            std::string message = "PART " + it->first + " :Leaving\n\r";
            const char *str = message.c_str();
            it->second->users[fd].part(this, str);
			users.erase(fd);
            break;
        }
    }
}

std::string get_user(std::string buf, server *server)
{
    std::size_t nickStartPos = buf.find("PRIVMSG ") + 8;
    std::size_t nickEndPos = buf.find(" :", nickStartPos);
    std::string nick;
    try {
		nick = buf.substr(nickStartPos, nickEndPos - nickStartPos);
	} catch (std::out_of_range& e)
	{
		return "";
    }
    for (std::map<int, user>::iterator it = server->users.begin(); it != server->users.end(); it++)
    {
        if (it->second.getNickname() == nick)
			return (nick);
    }
    return ("");
}

void	server::shutDown(std::vector<pollfd> &fds)
{
	for (size_t i = 1; i < fds.size(); i++)
		close(fds[i].fd);
	close(socket_id);
	for (std::map<std::string, channel *>::iterator it = channels.begin(); it != channels.end(); it++)
		delete it->second;
	std::cout << RED << "\nServer shutting down" << NC << std::endl;
}

std::string get_channel(char *buf, server *server)
{
    std::string buffer(buf);
    std::size_t channelStartPos = buffer.find("#");
    if (channelStartPos == std::string::npos)
        return ("");
    else
    {
        std::string channelName = buffer.substr(channelStartPos);
        std::size_t channelEndPos = channelName.find_first_of("\t\n\r ");
        if (server->channels.find(channelName.substr(0, channelEndPos)) != server->channels.end())
                return (channelName.substr(0, channelEndPos));
        return ("");
    }
}

void server::priv_n_op(user *sUser, int fd)
{
	std::string buff = sUser->getBuffer();
	std::string start = buff.substr(0, buff.find(" "));
	std::string channelName = get_channel(sUser->getBuffer(), this);
	if (!channelName.empty() && start != "PART")
	{
		int timeoutDuration = -1;
		if (this->users[fd].timeouts.find(channelName) != this->users[fd].timeouts.end())
			timeoutDuration = ((30 * (this->users[fd].timeouts[channelName].second - 1)) - static_cast<int>(std::difftime(std::time(0), this->users[fd].timeouts[channelName].first)));
		if (this->channels[channelName]->users.find(fd)->second.getNickname().empty() || this->channels[channelName]->users.find(fd) == this->channels[channelName]->users.end() || this->users[fd].timeouts[channelName].first == 0 || timeoutDuration <= 0 || check_valid_command(start))
		{
			if (timeoutDuration <= 0 && this->users[fd].timeouts.find(channelName) != this->users[fd].timeouts.end())
				this->users[fd].timeouts[channelName].first = 0;
			check_priv(sUser->getBuffer(), fd, this);
			sUser->check_operator(sUser->getBuffer(), fd, this);
		}
		else
		{
			std::stringstream ss;
			ss << "BOT :You're in timeout for " << timeoutDuration << " seconds. Your message was not sent\n";
			std::string message = ss.str();
			if (sUser->getFromNc())
				send_user(fd, message.c_str(), message.size(), 0);
			else
			{
				std::string channelMessage = ":" + channels[channelName]->users[fd].getNickname() + "!" + channels[channelName]->users[fd].getUsername() + " PRIVMSG " + channelName + " :" + message + "\r\n";
				send_user(fd, channelMessage.c_str(), channelMessage.size(), 0);
			}
		}
	}
	else
	{
		check_priv(sUser->getBuffer(), fd, this);
	}
}

void	server::run(user *sUser, std::vector<pollfd> &fds, int fd, int i)
{
	if(sUser->getStillBuilding() == 1)
	{
		sUser->setFinalBuffer(std::strcat(sUser->getFinalBuffer(), sUser->getBuffer()));
		if(sUser->getFinalBuffer()[strlen(sUser->getFinalBuffer()) - 1] == '\n')
		{
			sUser->setStillBuilding(0);
			sUser->setBuffer(sUser->getFinalBuffer());
			std::memset(sUser->getFinalBuffer(), 0, BUFFER_SIZE);
		}
	}
	if(sUser->getStillBuilding() == 0)
	{
		if (!check_leave(fds, this, sUser->getBuffer(), fd, i))
		{
			login(i, this, fds, sUser->getBuffer());
			if (sUser->getStatus() == 4)
			{
				bot_timeout(this, sUser->getBuffer(), fd);
				sUser->change_nick(sUser->getBuffer(), fd, this);
				check_channel(sUser->getBuffer(), fd, this);
				sUser->part(this, sUser->getBuffer());
				priv_n_op(sUser, fd);
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
    if (this->socket_id == -1)
	{
        std::cerr << "Can't create a socket!";
        exit(EXIT_FAILURE);
    }

    fcntl(socket_id, F_SETFL, O_NONBLOCK);

    this->IP.sin_family = AF_INET;
    if (inet_pton(AF_INET, this->server_ip, &this->IP.sin_addr) <= 0)
	{
        std::cerr << "Invalid address/Address not supported\n";
        exit(EXIT_FAILURE);
    }
    this->IP.sin_port = htons(PORT);
    if (bind(this->socket_id, (sockaddr * ) & this->IP, sizeof(this->IP)) == -1)
	{
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
