#include "../inc/main.hpp"

extern bool turn_off;

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
	newPfd.revents = 0;
	fds.push_back(newPfd);
	fds[0].revents = 0;
	send_user(newPfd.fd, "Welcome to the server!\n", 23, 0);
	send_user(newPfd.fd, "Please enter the server password: /PASS <password>\r\n", 52, 0);
}

std::string get_channel(char *buf)
{
	std::string buffer(buf);
	// channel = first # found in buffer
	std::size_t channelStartPos = buffer.find("#");
	if (channelStartPos == std::string::npos)
	{
		std::cout << "No channel found in buffer" << std::endl;
		return ("");
	}
	else
	{
		std::string channel = buffer.substr(channelStartPos);
		std::size_t channelEndPos = channel.find_first_of("\t\n\r ");
		std::cout << "channel = " << channel.substr(0, channelEndPos) << std::endl;
		return (channel.substr(0, channelEndPos));
	}
}

void	bot_timeout(server *server, char *buffer, int fd)
{
	int timeout_time = 30;
	std::string buf(buffer);
	int caps = 0;
	int total = 0;
	size_t messageStartPos = check_message(buf);
	if (messageStartPos == 0)
		return ;
	for (size_t i = messageStartPos; i < buf.size(); i++)
	{
		if (isupper(buf[i]))
			caps++;
		total++;
	}
	if (caps > total / 2 && server->users[fd].getTimeStart() == 0)
	{
		server->users[fd].setTimeout(server->users[fd].getTimeout() + 1);
	}
	if(server->users[fd].getTimeout() == 1 && caps > total / 2 && server->users[fd].getTimeStart() == 0)
	{
		if(server->users[fd].getFromNc())
			send_user(fd, "BOT :If you keep screaming I'm going to silence you\n", 53, 0);
		else
		{
			std::string channel = get_channel(server->users[fd].getBuffer());
			std::string channelMessage = ":" + server->users[fd].getNickname() + "!" + server->users[fd].getUsername() + " PRIVMSG " + channel + " :" + "BOT :If you keep screaming I'm going to silence you" + "\r\n";
			send_user(fd, channelMessage.c_str(), channelMessage.size(), 0);
		}
	}
	else if(server->users[fd].getTimeout() > 1 && caps > total / 2 && server->users[fd].getTimeStart() == 0)
	{
		server->users[fd].setTimeStart(std::time(0));
		if(server->users[fd].getFromNc())
			send_user(fd, "BOT :I warned you\n", 19, 0);
		else
		{
			std::string channel = get_channel(server->users[fd].getBuffer());
			std::string channelMessage = ":" + server->users[fd].getNickname() + "!" + server->users[fd].getUsername() + " PRIVMSG " + channel + " :" + "BOT :I warned you" + "\r\n";
			send_user(fd, channelMessage.c_str(), channelMessage.size(), 0);
		}
		return ;
	}
	if (std::difftime(std::time(0), server->users[fd].getTimeStart()) > timeout_time * (server->users[fd].getTimeout() - 1))
	{
		server->users[fd].setTimeStart(0);
	}
}

void delete_user(server *server, int fd)
{
    std::string nick = server->users[fd].getNickname();
    std::string username = server->users[fd].getUsername();
    std::string message = ":" + nick + "!" + username + " QUIT :Client disconnected\r\n";
    for (std::map<int, user>::iterator it = server->users.begin(); it != server->users.end(); ++it)
    {
        if (it->first != fd)
            send_user(it->first, message.c_str(), message.size(), 0);
    }
    close(fd);

    // Iterate over all channels
    for (std::map<std::string, channel*>::iterator it = server->channels.begin(); it != server->channels.end(); ++it)
    {
        // Get the map of users in the current channel
        std::map<int, user>& usersInChannel = it->second->users;

        // Check if the user is in the current channel
        if (usersInChannel.find(fd) != usersInChannel.end())
        {
            // Remove the user from the channel
            usersInChannel.erase(fd);
        }
    }

    // Remove the user from the server's user map
    server->users.erase(fd);
}

void    check_leave(server *server, char *buffer, int fd)
{
	std::string buf(buffer);
	if (buf.find("QUIT") != std::string::npos && (buf.find("QUIT") == 0 || buf[buf.find("QUIT") - 1] == '\n'))
	{
		delete_user(server, fd);
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
	serverPfd.revents = 0;
	fds.push_back(serverPfd);

    while (true) // Main server loop
    {
		sigHandler();

        int ret = poll(fds.data(), fds.size(), 100);

		if (turn_off)
		{
			for (size_t i = 1; i < fds.size(); i++)
			{
				close(fds[i].fd);
			}
			close(server->socket_id);
			for (std::map<std::string, channel *>::iterator it = server->channels.begin(); it != server->channels.end(); it++)
			{
				delete it->second;
			}
			std::cout << "Server shutting down" << std::endl;
			break;
		}

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
							if (i < fds.size() && fds[i].fd == it->second->users[i].getSocket())
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
						std::memset(server->users[fds[i].fd].getFinalBuffer(), 0, BUFFER_SIZE);
					}
				}
				if(server->users[fds[i].fd].getStillBuilding() == 0)
				{
					check_leave(server, server->users[fds[i].fd].getBuffer(), fds[i].fd);
					login(i, server, fds, server->users[fds[i].fd].getBuffer());
					if (server->users[fds[i].fd].getStatus() == 4)
					{
						bot_timeout(server, server->users[fds[i].fd].getBuffer(), fds[i].fd);
						if(server->users[fds[i].fd].getTimeStart() == 0)
						{
							server->users[fds[i].fd].change_nick(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
							check_channel(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
							check_priv(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
							server->users[fds[i].fd].check_operator(server->users[fds[i].fd].getBuffer(), fds[i].fd, server);
							server->users[fds[i].fd].part(server, server->users[fds[i].fd].getBuffer());
						}
						else
						{
							std::stringstream ss;
							ss << "BOT :You're in timeout for " << ((30 * (server->users[fds[i].fd].getTimeout() - 1)) - static_cast<int>(std::difftime(std::time(0), server->users[fds[i].fd].getTimeStart()))) << " seconds. Your message was not sent\n";
							std::string message = ss.str();
							if(server->users[fds[i].fd].getFromNc())
								send_user(fds[i].fd, message.c_str(), message.size(), 0);
							else
							{
								// get the fd from the user on the current channel
								std::string channel = get_channel(server->users[fds[i].fd].getBuffer());
								std::string channelMessage = ":" + server->users[fds[i].fd].getNickname() + "!" + server->users[fds[i].fd].getUsername() + " PRIVMSG " + channel + " :" + message + "\r\n";
								send_user(fds[i].fd, channelMessage.c_str(), channelMessage.size(), 0);
							}
						}
					}
				}
				std::cout << "Buffer: " << server->users[fds[i].fd].getBuffer() << std::endl;
				std::cout << "status = " << server->users[fds[i].fd].getStatus() << "\n";
				std::cout << "nickname = " << server->users[fds[i].fd].getNickname() << "\n";
				std::cout << "username = " << server->users[fds[i].fd].getUsername() << "\n";
				std::cout << "from_nc = " << server->users[fds[i].fd].getFromNc() << "\n";
			}
		}
	}
}