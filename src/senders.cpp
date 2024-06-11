//
// Created by lourenco on 10/06/24.
//

#include "../inc/server.hpp"

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
		std::cout << "nick on channel send = " << server->channels[channelName]->users[i].getNickname() << std::endl;
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