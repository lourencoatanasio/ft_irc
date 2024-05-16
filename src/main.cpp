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
    send_user(newPfd.fd, "Please enter your nickname and your user: \n", 43, 0);
    send_user(newPfd.fd, "NICK <nickname>\nUSER <username>\n", 32, 0);
}

void    check_login(char *buf, int fd, server *server)
{
    std::string buffer(buf);
    std::string nick = server->users[fd].getNickname();
    std::string username = server->users[fd].getUsername();
    if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
    {
        std::string nick = buffer.substr(buffer.find("NICK") + 5);
        std::size_t endPos = nick.find_first_of("\r\n");
        if (endPos != std::string::npos)
            nick = nick.substr(0, endPos);
        if (!server->users[fd].getNickname().empty())
        {
            std::string oldNick = server->users[fd].getNickname();
            server->users[fd].setNickname(nick);
            std::string message = ":" + oldNick + "!" + server->users[fd].getUsername() + " NICK " + server->users[fd].getNickname() + "\r\n";
            send_user(fd, message.c_str(), message.size(), 0);
        }
        else
            server->users[fd].setNickname(nick);
        std::cout << "Nickname set to: " << nick << std::endl;
    }
    if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
    {
        if (server->users[fd].getUsername().empty()) {
            std::string username = buffer.substr(buffer.find("USER") + 5,
                                                 buffer.find(" ", buffer.find("USER") + 5) - buffer.find("USER") - 5);
            std::size_t endPos = username.find_first_of("\r\n");
            if (endPos != std::string::npos)
                username = username.substr(0, endPos);
            server->users[fd].setUsername(username);
            std::cout << "Username set to: " << username << std::endl;
        }
        else
            send_user(fd, "You may not reregister\n", 23, 0);
    }

	if (nick.empty() && username.empty())
	{
		send_user(fd, "Please enter your nickname and your user: \n", 43, 0);
		send_user(fd, "NICK <nickname>\nUSER <username>\n", 32, 0);
	}
	else if (username.empty())
		send_user(fd, "Please enter your nickname: \nNICK <nickname>\n", 46, 0);
	else if (nick.empty())
		send_user(fd, "Please enter your username: \nUSER <username>\n", 46, 0);
}

void check_channel(char *buf, int fd, server *server)
{
    std::string buffer(buf), message;
    if (buffer.find("JOIN") != std::string::npos && (buffer.find("JOIN") == 0 || buffer[buffer.find("JOIN") - 1] == '\n')) {
        std::string username = server->users[fd].getUsername();
        std::string nick = server->users[fd].getNickname();
        std::string channelName = buffer.substr(buffer.find("JOIN") + 5);
        std::size_t endPos = channelName.find_first_of("\t\n\r ");
        if (endPos != std::string::npos)
            channelName = channelName.substr(0, endPos);
        if (server->channels.find(channelName) == server->channels.end()) {
            server->channels[channelName] = new channel();
            server->channels[channelName]->users[fd] = server->users[fd];
			server->channels[channelName]->users[fd].setOpStatus(true);
			message = ":" + nick + "!" + username + " JOIN " + channelName + "\r\n";
        }
		else if (server->channels[channelName]->getMaxUsers() < (int)server->channels[channelName]->users.size()
		&& server->channels[channelName]->getMaxUsers() > 0)
			message = ":" + channelName + " :Cannot join channel (+l)\r\n";
        else
		{
            server->channels[channelName]->users[fd] = server->users[fd];
			message = ":" + nick + "!" + username + " JOIN " + channelName + "\r\n";
		}
        send_user(fd, message.c_str(), message.size(), 0);
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

        if (channel.at(0) == '#') {
            if (server->channels.find(channel) == server->channels.end()) {
                std::string message = ":Channel does not exist\r\n";
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

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    server serverT; // Create a server object
    server *server = &serverT; // Create a pointer to the server object

    std::vector<pollfd> fds;

    pollfd serverPfd;
    serverPfd.fd = server->socket_id; // Use the -> operator to access members of the object pointed to by the pointer
    serverPfd.events = POLLIN;
    fds.push_back(serverPfd);

    char buffer[BUFFER_SIZE];

    while (true) // Main server loop
    {
        int ret = poll(fds.data(), fds.size(), -1); // Wait indefinitely for events

        if (ret == -1)
        {
            std::cerr << "Error in poll(). Quitting" << std::endl;
            break;
        }

        if (fds[0].revents & POLLIN)
            get_new_user(server, fds);

        for (size_t i = 1; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                std::memset(buffer, 0, BUFFER_SIZE);

                int bytesRead = recv(fds[i].fd, buffer, BUFFER_SIZE, 0);

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
                check_login(buffer, fds[i].fd, server);
                check_channel(buffer, fds[i].fd, server);
                check_priv(buffer, fds[i].fd, server);
				server->users[fds[i].fd].check_operator(buffer, fds[i].fd, server);
            }
        }
    }
}