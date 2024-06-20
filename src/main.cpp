#include "../inc/main.hpp"

extern bool turn_off;

void	get_new_user(server *server, std::vector<pollfd> &fds)
{
	int newClientSocket = accept(server->socket_id, NULL, NULL);
	if (newClientSocket == -1)
	{
		std::cerr << "Error in accept(). Quitting" << std::endl;
		return;
	}
	user newUser(newClientSocket);
	server->users[newClientSocket] = newUser;
	pollfd newPfd;
	newPfd.fd = newClientSocket;
	newPfd.events = POLLIN;
	newPfd.revents = 0;
	fds.push_back(newPfd);
	fds[0].revents = 0;
	std::cout << GREEN << "New client connected" << NC << std::endl;
	send_user(newPfd.fd, "Welcome to the server!\n", 23, 0);
	send_user(newPfd.fd, "Please enter the server password: /PASS <password>\r\n", 52, 0);
}

std::string user_in_channel(char *buf, server *server, int fd)
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
        {
            if (!server->channels[channelName.substr(0, channelEndPos)]->users.find(fd)->second.getNickname().empty())
                return (channelName.substr(0, channelEndPos));
        }
		return ("");
	}
}

void	bot_timeout(server *server, char *buffer, int fd)
{
	int timeout_time = 30;
	std::string buf(buffer);
	int caps = 0;
	int total = 0;

	std::string channel = user_in_channel(buffer, server, fd);

	size_t messageStartPos = check_message(buf);

	if (messageStartPos == 0 || channel.empty())
		return ;
	for (size_t i = messageStartPos; i < buf.size(); i++)
	{
		if (isupper(buf[i]))
			caps++;
		total++;
	}
	if (caps > total / 2 && server->users[fd].timeouts[channel].first == 0)
	{
		server->users[fd].timeouts[channel].second = server->users[fd].timeouts[channel].second + 1;
	}
	if(server->users[fd].timeouts[channel].second == 1 && caps > total / 2 && server->users[fd].timeouts[channel].first == 0)
	{
		if(server->channels[channel]->users[fd].getFromNc())
			send_user(fd, "BOT :If you keep screaming I'm going to silence you\r\n", 53, 0);
		else
		{
			std::string channelMessage = ":" + server->channels[channel]->users[fd].getNickname() + "!" + server->channels[channel]->users[fd].getUsername() + " PRIVMSG " + channel + " :" + "BOT :If you keep screaming I'm going to silence you" + "\r\n";
			send_user(fd, channelMessage.c_str(), channelMessage.size(), 0);
		}
	}
	else if(server->users[fd].timeouts[channel].second > 1 && caps > total / 2 && server->users[fd].timeouts[channel].first == 0)
	{
		server->users[fd].timeouts[channel].first = std::time(0);
		if(server->channels[channel]->users[fd].getFromNc())
			send_user(fd, "BOT :I warned you\r\n", 19, 0);
		else
		{
			std::string channelMessage = ":" + server->channels[channel]->users[fd].getNickname() + "!" + server->channels[channel]->users[fd].getUsername() + " PRIVMSG " + channel + " :" + "BOT :I warned you" + "\r\n";
			send_user(fd, channelMessage.c_str(), channelMessage.size(), 0);
		}
		return ;
	}
	if (std::difftime(std::time(0), server->users[fd].timeouts[channel].first) > timeout_time * (server->users[fd].timeouts[channel].second - 1))
	{
		server->users[fd].timeouts[channel].first = 0;
	}
}

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		std::cout << "Error: Proper use is <./ft_irc <port> <password>\n";
		return (1);
	}
	server serverT(argv[2], argv[1]);
	server *server = &serverT;
	std::vector<pollfd> fds;

	std::cout << GREEN << "Server started" << NC << std::endl;
	pollfd serverPfd;
	serverPfd.fd = server->socket_id;
	serverPfd.events = POLLIN;
	serverPfd.revents = 0;
	fds.push_back(serverPfd);

	while (true)
	{
		sigHandler();

		int ret = poll(fds.data(), fds.size(), 100);

		if (turn_off)
		{
			server->shutDown(fds);
			break ;
		}

		if (ret == -1)
		{
			std::cerr << "Error in poll(). Quitting" << std::endl;
			break;
		}

		if (fds[0].revents & POLLIN)
			get_new_user(server, fds);

		for (size_t i = 1; i < fds.size(); ++i)
		{
			check_source(fds[i].fd, server, ret);
			if (fds[i].revents & POLLIN)
			{
				std::memset(server->users[fds[i].fd].getBuffer(), 0, BUFFER_SIZE + 1);
				int bytesRead = recv(fds[i].fd, server->users[fds[i].fd].getBuffer(), BUFFER_SIZE, 0);
				check_still_building(fds[i].fd, server);
				if (bytesRead == -1)
				{
					std::cerr << "Error in recv(). Quitting" << std::endl;
					break;
				}
				if (bytesRead == 0)
				{
					server->disconnect(fds, fds[i].fd, i);
					break ;
				}
				server->run(&server->users[fds[i].fd], fds, fds[i].fd, i);
			}
		}
	}
	return (0);
}