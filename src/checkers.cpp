//
// Created by lourenco on 10/06/24.
//

#include "../inc/server.hpp"

size_t check_message(std::string buffer)
{
	if (buffer.find("PRIVMSG") != std::string::npos && (buffer.find("PRIVMSG") == 0 || buffer[buffer.find("PRIVMSG") - 1] == '\n'))
	{
		std::size_t messageStartPos = buffer.find(":", buffer.find("PRIVMSG") + 8);
		if (messageStartPos != std::string::npos)
		{
			std::string receivedMessage = buffer.substr(messageStartPos + 1);
			receivedMessage.erase(0, receivedMessage.find_first_not_of(' '));
			receivedMessage.erase(receivedMessage.find_last_not_of(' ') + 1);
			if (receivedMessage.empty())
				return (0);
			else
				return (messageStartPos);
		}
	}
	return (0);
}

void	check_source(int fd, server *server, int ret)
{
	if(ret == 0 && (server->users[fd].getNickname().empty() && server->users[fd].getUsername().empty()))
	{
		server->users[fd].setFromNc(1);
	}
}

void	check_still_building(int fd, server *server)
{
	std::string buffer(server->users[fd].getBuffer());

	if (buffer.find("\n") == std::string::npos)
	{
		server->users[fd].setStillBuilding(1);
	}
}

void 	check_channel(char *buf, int fd, server *server)
{
	std::string buffer(buf), message, cmd;
	std::istringstream iss(buffer);
	iss >> cmd;
	if (cmd.compare("JOIN") == 0) {
		std::string username = server->users[fd].getUsername();
		std::string nick = server->users[fd].getNickname();
		std::string channelName = buffer.substr(buffer.find("JOIN") + 5);
		std::size_t endPos = channelName.find_first_of("\t\n\r ,");
		if (endPos != std::string::npos)
			channelName = channelName.substr(0, endPos);
		if(channelName[0] != '#' || channelName.size() > 200)
		{
			std::string message = ": 403 " + nick + " " + channelName + " :No such channel\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			return;
		}
		if (server->channels.find(channelName) == server->channels.end())
		{
			std::cout << "Creating channel " << channelName << std::endl;
			server->channels[channelName] = new channel();
			server->channels[channelName]->setOps(1);
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
			if (server->channels[channel]->users[fd].getSocket() <= 0)
			{
				std::string message = ": 404 " + nick + " " + channel + " :Cannot send to channel\r\n";
				send_user(fd, message.c_str(), message.size(), 0);
				return ;
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
			std::size_t messageStartPos = buffer.find(":", channelEndPos + 1);
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
						return ;
					}
				}
				std::string message =  ": 401 " + nick + " " + channel + " :No such nick\r\n";
				send_user(server->users[fd].getSocket(), message.c_str(), message.size(), 0);
			}
		}
	}
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

void    check_leave(server *server, char *buffer, int fd)
{
	std::string buf(buffer);
	if (buf.find("QUIT") != std::string::npos && (buf.find("QUIT") == 0 || buf[buf.find("QUIT") - 1] == '\n'))
	{
		delete_user(server, fd);
	}
}