#include "../inc/main.hpp"

ssize_t	send_user(int socket, const void *buffer, size_t length, int flags)
{
	ssize_t totalSentBytes = 0;
	const char *bufferPtr = (const char*) buffer;

	while (totalSentBytes < static_cast<ssize_t>(length))
	{
		ssize_t sentBytes = send(socket, bufferPtr + totalSentBytes, length - totalSentBytes, flags);
		if (sentBytes == -1)
		{
			return -1; // Return -1 on failure, caller will check errno
		}

		totalSentBytes += sentBytes;
	}

	return totalSentBytes;
}

ssize_t	send_all(server *server, const void *buffer, size_t lenght, int flags, std::string channel)
{
	ssize_t totalSentBytes = 0;

	for (std::size_t j = 0; j < server->channels[channel]->users.size(); j++)
	{
		totalSentBytes += send_user(server->channels[channel]->users[j].getSocket(), buffer, lenght, flags);
	}
	return (totalSentBytes);
}

void	get_new_user(server *server, std::vector<pollfd> &fds)
{
	int newClientSocket = accept(server->socket_id, NULL, NULL);
	if (newClientSocket == -1)
	{
		std::cerr << "Can't accept client!";
		return;
	}
	user newUser(newClientSocket);
	server->users[newClientSocket] = newUser;
	pollfd newPfd;
	newPfd.fd = newClientSocket;
	newPfd.events = POLLIN;
	fds.push_back(newPfd);
	fds[0].revents = 0;
	send_user(newPfd.fd, "Welcome to the server!\n", 23, 0);
	send_user(newPfd.fd, "Please enter the server password: /PASS <password>\r\n", 52, 0);
}

int	check_valid(std::string buffer)
{
	if(buffer.empty())
	{
		std::cout << "error 1\n";
		return (1);
	}
	return (0);
}

/*
void	get_username(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
	{
		if (server->users[fd].getUsername().empty()) {
			std::string username = buffer.substr(buffer.find("USER") + 5);
			if(check_valid(username) == 1)
			{
				send_user(fd, "Please enter a valid username: USER <username>\n", 47, 0);
				return ;
			}
			while(username[username.size() - 1] == ' ' || username[username.size() - 1] == '\t')
				username = username.substr(0, username.size() - 1);
			while(username[0] == ' ' || username[0] == '\t')
				username = username.substr(1);
			std::size_t endPos = username.find_first_of("\r\n");
			if (endPos != std::string::npos)
				username = username.substr(0, endPos);
			username = username.substr(0, username.find(" "));
            server->users[fd].setUsername(username);
            std::cout << "Username set to: |" << username << "|\n";
            server->users[fd].setStatus(3);
            std::cout << "status = " << server->users[fd].getStatus() << "\n";
        }
        else
            send_user(fd, "You may not reregister\n", 23, 0);
    }
    else if(server->users[fd].getUsername().empty())
        send_user(fd, "Please enter your username: USER <username>\n", 45, 0);
}
*/

void	get_username_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
	{
		if (server->users[fd].getUsername().empty()) {
			std::string username = buffer.substr(buffer.find("USER") + 5);
			username = username.substr(0, username.find("* :realname") - 2);
			while(username[username.size() - 1] == ' ' || username[username.size() - 1] == '\t')
				username = username.substr(0, username.size() - 1);
			std::size_t endPos = username.find_first_of("\r\n");
			if (endPos != std::string::npos)
				username = username.substr(0, endPos);
			username = username.substr(0, username.find(" "));
			server->users[fd].setUsername(username);
			std::cout << "Username set to: |" << username << "|\n";
			server->users[fd].setStatus(1);
		}
		else
			send_user(fd, "You may not reregister\r\n", 23, 0);
	}
}

void	get_nickname_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
	{
		std::string oldNick = server->users[fd].getNickname();
		std::string nick(buffer);
		nick.erase(0, nick.find("NICK") + 4);
		nick.erase(0, nick.find_first_not_of(' '));
		nick.erase(nick.find_first_of("\n\r\t "), nick.size() - 1);
        if (server->users[fd].getOldNick().empty())
            server->users[fd].setOldNick(nick);
		if (server->users[fd].check_same_nick(nick, server) == 1)
		{
            server->users[fd].flag = 1;
			send_user(fd, "Nickname is erroneous or already in use. Use /NICK to try another.\r\n", 68, 0);
			return ;
		}
		if (oldNick.compare(nick) == 0)
            return;
        //server->users[fd].setNickname(nick);
        std::string message = ":" + server->users[fd].getOldNick() + "!" + server->users[fd].getUsername() + " NICK " + nick + "\r\n";
        if (!oldNick.empty() || server->users[fd].flag == 1)
          send_user(fd, message.c_str(), message.size(), 0);
        server->users[fd].setNickname(nick);
		std::cout << "Nickname set to: |" << nick << "|\n";
		server->users[fd].setStatus(2);
	}
	/*else if(server->users[fd].getNickname().empty())
		send_user(fd, "Please enter your nickname: NICK <nickname>\r\n", 45, 0);*/
}

/*void	get_nickname(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
	{
		std::string oldNick = server->users[fd].getNickname();
		std::string nick = buffer.substr(buffer.find("NICK") + 5);
		if(check_valid(nick) == 1)
		{
			send_user(fd, "Please enter a valid nickname: NICK <nickname>\n", 47, 0);
			return ;
		}
		while(nick[nick.size() - 1] == ' ' || nick[nick.size() - 1] == '\t')
			nick = nick.substr(0, nick.size() - 1);
		while(nick[0] == ' ' || nick[0] == '\t')
			nick = nick.substr(1);
		std::size_t endPos = nick.find_first_of("\r\n");
		if (endPos != std::string::npos)
			nick = nick.substr(0, endPos);
		nick = nick.substr(0, nick.find(" "));
		if(check_same_nick(nick, server) == 1)
		{
			send_user(fd, "Nickname already taken, please try again\n", 42, 0);
			return ;
		}
        if (oldNick.compare(nick) == 0)
            return ;
        server->users[fd].setNickname(nick);
        std::string message = ":" + oldNick + "!" + server->users[fd].getUsername() + " NICK " + server->users[fd].getNickname() + "\r\n";
        send_user(fd, message.c_str(), message.size(), 0);
        std::cout << "status = " << server->users[fd].getStatus() << "\n";
        std::cout << "Nickname set to: |" << nick << "|\n";
        server->users[fd].setStatus(2);
    }
    else if(server->users[fd].getNickname().empty())
        send_user(fd, "Please enter your nickname: NICK <nickname>\n", 45, 0);
}*/

/*
void    get_password(char *buf, int fd, server *server)
{
    std::string buffer(buf);
    if (buffer.find("/PASS") != std::string::npos && (buffer.find("/PASS") == 0 || buffer[buffer.find("/PASS") - 1] == '\n'))
    {
        std::string pass = buffer.substr(buffer.find("/PASS") + 6);
		if(check_valid(pass) == 1)
		{
			send_user(fd, "Please enter a valid password: /PASS <password>\n", 48, 0);
			return ;
		}
        std::size_t endPos = pass.find_first_of("\r\n");
        if (endPos != std::string::npos)
            pass = pass.substr(0, endPos);
        if (pass != server->getPass())
        {
            std::cout << server->getPass() << std::endl;
            std::cout << pass << std::endl;
            send_user(fd, "You've entered the wrong password, please try again\n", 53, 0);
            return ;
        }
        else
        {
            server->users[fd].setStatus(1);
            return ;
        }
    }
    else if(server->users[fd].getStatus() == 0)
        send_user(fd, "Please enter the server password: /PASS <password>\n", 52, 0);
}
*/

void	get_password_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::cout << buffer << "\n";
	if (buffer.find("PASS") != std::string::npos && (buffer.find("PASS") == 0 || buffer[buffer.find("PASS") - 1] == '\n'))
	{
		std::string pass = buffer.substr(buffer.find("PASS") + 5);
		if(check_valid(pass) == 1)
		{
			send_user(fd, "Please enter a valid password: PASS <password>\r\n", 49, 0);
			return ;
		}
		std::size_t endPos = pass.find_first_of("\r\n");
		if (endPos != std::string::npos)
			pass = pass.substr(0, endPos);
		if (pass != server->getPass())
		{
			send_user(fd, "You've entered the wrong password, please try again\r\n", 53, 0);
			return ;
		}
		else
		{
			server->users[fd].setStatus(3);
			return ;
		}
	}
	else if(buffer.find("CAP LS") != std::string::npos)
	{
		return ;
	}
}

void	channel_send(server *server, std::string nick, std::string username, std::string channelName, int fd)
{
	std::vector<std::string> v;
	v.push_back(":" + nick + "!" + username + " JOIN " + channelName + "\r\n");
	v.push_back(": 353 " + nick + " = " + channelName + " :");
	for (size_t i = 0; i < server->channels[channelName]->users.size(); i++)
	{
		if (server->channels[channelName]->users[i].getOpStatus() == true)
			v[1] += "@";
		v[1] += server->channels[channelName]->users[i].getNickname() + " ";
	}
	v[1] += "\r\n";
	v.push_back(": 366 " + nick + " " + channelName + " :End of /NAMES list.\r\n");
	v.push_back(": 324 " + nick + " " + channelName + " +tn\r\n");
	v.push_back(": 354 " + nick + " 152 " + channelName + " " + username + " ft_irc " + nick + "\r\n");
	v.push_back(": 315 " + nick + " " + channelName + " :End of /WHO list.\r\n");
	
	for (size_t i = 1; i < v.size(); i++)
	{
		send_user(fd, v[i].c_str(), v[i].size(), 0);
	}
}

void check_channel(char *buf, int fd, server *server)
{
	std::string buffer(buf), message, cmd;
	std::istringstream iss(buffer);
	iss >> cmd;
	if (cmd.compare("JOIN") == 0) {
		std::string username = server->users[fd].getUsername();
		std::string nick = server->users[fd].getNickname();
		std::string channelName = buffer.substr(buffer.find("JOIN") + 5);
		std::size_t endPos = channelName.find_first_of("\t\n\r ");
		if (endPos != std::string::npos)
			channelName = channelName.substr(0, endPos);
		if (server->channels.find(channelName) == server->channels.end())
		{
			server->channels[channelName] = new channel();
			server->channels[channelName]->users[fd] = server->users[fd];
			server->channels[channelName]->users[fd].setOpStatus(true);
			message = ":" + nick + "!" + username + " JOIN " + channelName + "\r\n";
		}
		else if (server->channels[channelName]->getMaxUsers() < (int)server->channels[channelName]->users.size()
		&& server->channels[channelName]->getMaxUsers() > 0)
		{
			message = ": 471 " + nick + " " + channelName + " :Cannot join channel (+l)\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			return;
		}
		else if (server->channels[channelName]->getInviteMode() == true)
		{
			std::string message = ": 473 " + nick + " " + channelName + " :Cannot join channel (+i)\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			return;
		}
		else
			server->channels[channelName]->users[fd] = server->users[fd];
		message = ":" + nick + "!" + username + " JOIN " + channelName + "\r\n";
		send_all(server, message.c_str(), message.size(), 0, channelName);
		channel_send(server, nick, username, channelName, fd);
		if (server->channels[channelName]->getTopic().empty() == false) {
			std::string message = ":" + channelName + " 332 " + nick + " " + channelName + " :" + server->channels[channelName]->getTopic() + "\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			message = ":" + channelName + " 333 " + nick + " " + channelName + " " + server->channels[channelName]->getNick() + "!" + server->channels[channelName]->getUser() +
					" 1715866598\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
		}
	}
}

void check_priv(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("PRIVMSG") != std::string::npos && (buffer.find("PRIVMSG") == 0 || buffer[buffer.find("PRIVMSG") - 1] == '\n')) {
		std::string nick = server->users[fd].getNickname();
		std::string username = server->users[fd].getUsername();
		std::size_t channelEndPos = buffer.find(" ", buffer.find("PRIVMSG") + 8);
		std::string channel = buffer.substr(buffer.find("PRIVMSG") + 8, channelEndPos - (buffer.find("PRIVMSG") + 8));

		if (!channel.empty() && channel.at(0) == '#') {
			if (server->channels.find(channel) == server->channels.end()) {
				std::string message = ": 403 " + nick + " " + channel + " :No such channel\r\n";
				send_user(fd, message.c_str(), message.size(), 0);
				return;
			}
			std::size_t messageStartPos = buffer.find(":", channelEndPos);
			if (messageStartPos != std::string::npos) {
				std::string receivedMessage = buffer.substr(messageStartPos + 1);
				receivedMessage.erase(0, receivedMessage.find_first_not_of(' '));
				receivedMessage.erase(receivedMessage.find_last_not_of(' ') + 1);

				for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++) {
					std::string message = ":" + nick + "!" + username + " PRIVMSG " + channel + " :" + receivedMessage + "\r\n";
					if (server->channels[channel]->users[i].getSocket() != fd)
						send_user(server->channels[channel]->users[i].getSocket(), message.c_str(), message.size(), 0);
				}
			}
		}
		else {
			std::size_t messageStartPos = buffer.find(":", channelEndPos);
			if (messageStartPos != std::string::npos) {
				std::string receivedMessage = buffer.substr(messageStartPos + 1);
				receivedMessage.erase(0, receivedMessage.find_first_not_of(' '));
				receivedMessage.erase(receivedMessage.find_last_not_of(' ') + 1);
				std::map<int, user>::iterator it;
				for (it = server->users.begin(); it != server->users.end(); ++it) {
					std::string message = ":" + nick + "!" + username + " PRIVMSG " + it->second.getNickname() + " :" + receivedMessage + "\r\n";
					if (it->second.getNickname() == channel) {
						std::cout << "Sending private message to " << it->second.getNickname() << std::endl;
						send_user(it->second.getSocket(), message.c_str(), message.size(), 0);
					}
				}
			}
		}
	}
}

void	check_source(int fd, server *server, int ret)
{
	if(ret == 0 && (server->users[fd].getNickname().empty() || server->users[fd].getUsername().empty()))
	{
		server->users[fd].setFromNc(1);
	}
}

void	login(int i, server *server, std::vector<pollfd> &fds, char *buffer)
{
    if (server->users[fds[i].fd].getStatus() < 3) {
        if (server->users[fds[i].fd].getStatus() == 0)
            get_username_hex(buffer, fds[i].fd, server);
        if (server->users[fds[i].fd].getStatus() == 1)
            get_nickname_hex(buffer, fds[i].fd, server);
        if (server->users[fds[i].fd].getStatus() == 2)
            get_password_hex(buffer, fds[i].fd, server);
    }
	/*else if (server->users[fds[i].fd].getFromNc() == 1 && server->users[fds[i].fd].getStatus() < 3)
	{
		if (server->users[fds[i].fd].getStatus() == 0)
			get_password(buffer, fds[i].fd, server);
		if (server->users[fds[i].fd].getStatus() == 1)
			get_nickname(buffer, fds[i].fd, server);
		if (server->users[fds[i].fd].getStatus() == 2)
			get_username(buffer, fds[i].fd, server);;
	}*/
	if(server->users[fds[i].fd].getStatus() == 3)
	{
		send_user(fds[i].fd, "Congratulations, you are now connected to the server!\r\n", 55, 0);
		server->users[fds[i].fd].setStatus(4);
	}
}

void	sigHandler()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
}

void	check_still_building(int fd, server *server)
{
	std::string buffer(server->users[fd].getBuffer());

	if (buffer.find("\n") == std::string::npos)
	{
		std::cout << "still building\n";
		server->users[fd].setStillBuilding(1);
	}
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        std::cout << "Error: Proper use is <./ft_irc <port> <password>\n";
        return (1);
    }
    server serverT(argv[2], argv[1]); // Create a server object
    server *server = &serverT; // Create a pointer to the server object

	std::vector<pollfd> fds;

	pollfd serverPfd;
	serverPfd.fd = server->socket_id; // Use the -> operator to access members of the object pointed to by the pointer
	serverPfd.events = POLLIN;
	fds.push_back(serverPfd);

    while (true) // Main server loop
    {
		// sigHandler();
        int ret = poll(fds.data(), fds.size(), 100);

		if (ret == -1)
		{
			std::cerr << "Error in poll(). Quitting" << std::endl;
			break;
		}

		if (fds[0].revents & POLLIN)
			get_new_user(server, fds);

		//std::cout << "users size = " << users.size() << "fds size = " << fds.size() << std::endl;

		for (size_t i = 1; i < fds.size(); ++i)
		{
			check_source(fds[i].fd, server, ret);
			if (fds[i].revents & POLLIN)
            {
                std::memset(server->users[fds[i].fd].getBuffer(), 0, BUFFER_SIZE);
                int bytesRead = recv(fds[i].fd, server->users[fds[i].fd].getBuffer(), BUFFER_SIZE, 0);
				check_still_building(fds[i].fd, server);
				if (bytesRead == -1)
				{
					std::cerr << "Error in recv(). Quitting" << std::endl;
					break;
				}

				if (bytesRead == 0)
				{
					std::cout << "Client disconnected" << std::endl;
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					server->users.erase(fds[i].fd);
					for (std::map<std::string, channel *>::iterator it = server->channels.begin(); it != server->channels.end(); it++)
					{
						for (size_t i = 0; i < it->second->users.size(); i++)
						{
							if (it->second->users[i].getSocket() == fds[i].fd)
							{
								server->channels.erase(it);
							}
						}
					}
                    break;
                }
				if(server->users[fds[i].fd].getStillBuilding() == 1)
				{
					server->users[fds[i].fd].setFinalBuffer(std::strcat(server->users[fds[i].fd].getFinalBuffer(), server->users[fds[i].fd].getBuffer()));
					if(server->users[fds[i].fd].getFinalBuffer()[strlen(server->users[fds[i].fd].getFinalBuffer()) - 1] == '\n')
					{
						server->users[fds[i].fd].setStillBuilding(0);
						server->users[fds[i].fd].setBuffer(server->users[fds[i].fd].getFinalBuffer());
					}
				}
				if(server->users[fds[i].fd].getStillBuilding() == 0)
				{
					login(i, server, fds, server->users[fds[i].fd].getBuffer());
					if (server->users[fds[i].fd].getStatus() == 4)
					{
						server->users[fds[i].fd].change_nick(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
						check_channel(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
						check_priv(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
						server->users[fds[i].fd].check_operator(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
					}
				}
			}
		}
	}
}