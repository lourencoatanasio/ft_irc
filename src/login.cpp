//
// Created by lourenco on 10/06/24.
//

#include "../inc/server.hpp"

void	login(int i, server *server, std::vector<pollfd> &fds, char *buffer)
{
	if (server->users[fds[i].fd].getStatus() < 3 && server->users[fds[i].fd].getFromNc() == 0)
	{
		if (server->users[fds[i].fd].getStatus() == 0)
			get_username_hex(buffer, fds[i].fd, server);
		if (server->users[fds[i].fd].getStatus() == 1)
			get_nickname_hex(buffer, fds[i].fd, server);
		if (server->users[fds[i].fd].getStatus() == 2)
			get_password_hex(buffer, fds[i].fd, server);
	}
	else
	{
		if (server->users[fds[i].fd].getFromNc() == 1 && server->users[fds[i].fd].getStatus() < 3) {
			if (server->users[fds[i].fd].getStatus() == 0)
				get_password(buffer, fds[i].fd, server);
			if (server->users[fds[i].fd].getStatus() == 1)
				get_nickname(buffer, fds[i].fd, server);
			if (server->users[fds[i].fd].getStatus() == 2)
				get_username(buffer, fds[i].fd, server);
		}
	}

	if(server->users[fds[i].fd].getStatus() == 3)
	{
		if(server->users[fds[i].fd].getFromNc() == 0)
		{
			std::string message = ":ircserver 001 " + server->users[fds[i].fd].getNickname() + " :Welcome to the Internet Relay Network, " + server->users[fds[i].fd].getNickname() + "!\r\n";
			send_user(fds[i].fd, message.c_str(), message.size(), 0);
		}
		else
		{
			std::string message = ":ircserver 001 * :Welcome to the Internet Relay Network, *!\r\n";
			send_user(fds[i].fd, message.c_str(), message.size(), 0);
		}

		server->users[fds[i].fd].setStatus(4);
	}
}

void	get_username(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::istringstream iss(buffer);
	std::string cmd;
	std::string username;
	iss >> cmd >> username;
	if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n') && cmd.compare("USER") == 0)
	{
		if (server->users[fd].getUsername().empty())
		{
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
			server->users[fd].setStatus(3);
		}
		else
			send_user(fd, "You may not reregister\n", 23, 0);
	}
	else if(server->users[fd].getUsername().empty())
		send_user(fd, "Please enter your username: USER <username>\n", 45, 0);
}


void	get_username_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
	{
		if (server->users[fd].getUsername().empty())
		{
			std::string username = buffer.substr(buffer.find("USER") + 5);
			username = username.substr(0, username.find("* :realname") - 2);
			while(username[username.size() - 1] == ' ' || username[username.size() - 1] == '\t')
				username = username.substr(0, username.size() - 1);
			std::size_t endPos = username.find_first_of("\r\n");
			if (endPos != std::string::npos)
				username = username.substr(0, endPos);
			username = username.substr(0, username.find(" "));
			server->users[fd].setUsername(username);
			server->users[fd].setStatus(1);
		}
		else
			send_user(fd, "You may not reregister\r\n", 23, 0);
	}
}

int nick_checker(std::string nick)
{
	if(isalpha(nick[0]) == 0 && nick[0] != '_' && nick[0] != '-')
		return (1);
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (isalnum(nick[i]) == 0 && nick[i] != '_' && nick[i] != '-')
			return (1);
	}
	return (0);
}

void	get_nickname_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
	{
		std::string oldNick = server->users[fd].getNickname();
		std::string nick(buffer);
		nick.erase(0, nick.find("NICK") + 5);
		nick.erase(nick.find_first_of("\n\r\t"), nick.size() - 1);
		if (server->users[fd].getOldNick().empty())
			server->users[fd].setOldNick(nick);
		for(size_t i = 0; i < nick.size(); i++)
		{
			if(nick[0] == ' ' || nick[0] == '\t')
			{
				nick = nick.substr(1);
				i = 0;
			}
			else
				break;
		}
		if(nick.find(" ") != std::string::npos)
			nick = nick.substr(0, nick.find(" "));
		if(server->users[fd].check_same_nick(nick, server) == 1 || nick_checker(nick) == 1)
		{
			server->users[fd].flag = 1;
			if(server->users[fd].getFromNc() == 1)
				send_user(fd, "Nickname is erroneous or already in use. Use /NICK to try another.\r\n", 68, 0);
			else
			{
				std::string message = ":ircserver 432 * " + nick + " :Erroneous Nickname\r\n";
				send_user(fd, message.c_str(), message.size(), 0);
			}
			return ;
		}
		if (oldNick.compare(nick) == 0)
			return;
		server->users[fd].setNickname(nick);
		if (!oldNick.empty() || server->users[fd].flag == 1)
			send_user(fd, "Please enter the server password: /PASS <password>\r\n", 52, 0);
		server->users[fd].setStatus(2);
	}
	else if(server->users[fd].getNickname().empty())
		send_user(fd, "Please enter your nickname: NICK <nickname>\r\n", 45, 0);
}

void	get_nickname(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::istringstream iss(buffer);
	std::string cmd;
	std::string nick;
	iss >> cmd >> nick;
	if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n') && cmd.compare("NICK") == 0)
	{
		std::string oldNick = server->users[fd].getNickname();
		if(check_valid(nick) == 1)
		{
			send_user(fd, "Please enter a valid nickname: NICK <nickname>\n", 48, 0);
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
		if(server->users[fd].check_same_nick(nick, server) == 1)
		{
			send_user(fd, "Nickname already taken, please try again\r\n", 42, 0);
			return ;
		}
		if (oldNick.compare(nick) == 0)
			return ;
		server->users[fd].setNickname(nick);
		server->users[fd].setStatus(2);
	}
	else if(server->users[fd].getNickname().empty())
		send_user(fd, "Please enter your nickname: NICK <nickname>\r\n", 45, 0);
}

void    get_password(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::istringstream iss(buffer);
	std::string cmd;
	std::string pass;
	iss >> cmd >> pass;
	if (buffer.find("/PASS") != std::string::npos && (buffer.find("/PASS") == 0 || buffer[buffer.find("/PASS") - 1] == '\n'))
	{
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

void	get_password_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
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