#include "../inc/main.hpp"

ssize_t send_all(int socket, const void *buffer, size_t length, int flags)
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

void     get_new_user(server *server, std::vector<pollfd> &fds)
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
    send_all(newPfd.fd, "Welcome to the server!\n", 23, 0);
	send_all(newPfd.fd, "Please enter the server password: PASS <password>\n", 51, 0);
}

void	login_user(std::string buffer, int fd, server *server)
{
	if ((buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
	|| (buffer.find("user") != std::string::npos && buffer.find("user") == 0))
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
            send_all(fd, "You may not reregister\n", 23, 0);
    }
}

void	login_nick(std::string buffer, int fd, server *server)
{
	 if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
    {
		std::string oldNick = server->users[fd].getNickname();
        std::string nick = buffer.substr(buffer.find("NICK") + 5);
        std::size_t endPos = nick.find_first_of("\r\n");
        if (endPos != std::string::npos)
            nick = nick.substr(0, endPos);
		std::cout << "oldNick: " << oldNick << " newNick: " << nick << "oldNick.compare(nick): " << oldNick.compare(nick) << "\n";
		if (oldNick.compare(nick) == 0)
			return ;
        //:hcorrea-!hcorrea- NICK :user
        if (!server->users[fd].getNickname().empty())
        {
            server->users[fd].setNickname(nick);
            std::string message = ":" + oldNick + "!" + server->users[fd].getUsername() + " NICK " + server->users[fd].getNickname() + "\r\n";
            send_all(fd, message.c_str(), message.size(), 0);
        }
        else
            server->users[fd].setNickname(nick);
        std::cout << "Nickname set to: " << nick << std::endl;
    }
}

void    check_login(char *buf, int fd, server *server)
{
    std::string buffer(buf);
    std::string nick = server->users[fd].getNickname();
    std::string username = server->users[fd].getUsername();

	login_nick(buffer, fd, server);
	login_user(buffer, fd, server);
	if (nick.empty() && username.empty())
	{
		send_all(fd, "Please enter your nickname and your user: \n", 43, 0);
		send_all(fd, "NICK <nickname>\nUSER <username>\n", 32, 0);
	}
	else if (username.empty())
		send_all(fd, "Please enter your nickname: \nNICK <nickname>\n", 46, 0);
	else if (nick.empty())
		send_all(fd, "Please enter your username: \nUSER <username>\n", 46, 0);
}

void check_channel(char *buf, int fd, server *server)
{
    std::cout << "user = " << server->users[fd].getUsername() << " nick = " << server->users[fd].getNickname() << " i = " << fd << std::endl;
    std::string buffer(buf);
    if (buffer.find("JOIN") != std::string::npos && (buffer.find("JOIN") == 0 || buffer[buffer.find("JOIN") - 1] == '\n')) {
        std::string username = server->users[fd].getUsername();
        std::string nick = server->users[fd].getNickname();
        std::string channelName = buffer.substr(buffer.find("JOIN") + 5);
        std::size_t endPos = channelName.find_first_of("\t\n\r ");
        if (endPos != std::string::npos)
            channelName = channelName.substr(0, endPos);
        if (server->channels.find(channelName) == server->channels.end()) {
            server->channels[channelName] = new channel();
            server->channels[channelName]->add_user(server->users[fd]);
        }
        else
            server->channels[channelName]->add_user(server->users[fd]);
        std::string message = ":" + nick + "!" + username + " JOIN " + channelName + "\r\n";
        std::cout << "join channel = " << message << "." << std::endl;
        send_all(fd, message.c_str(), message.size(), 0);
        std::cout << "Channel set to: " << channelName << std::endl;
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

        std::size_t messageStartPos = buffer.find(":", channelEndPos);
        if (messageStartPos != std::string::npos) {
            std::string receivedMessage = buffer.substr(messageStartPos + 1);
            receivedMessage.erase(0, receivedMessage.find_first_not_of(' '));
            receivedMessage.erase(receivedMessage.find_last_not_of(' ') + 1);

            for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++)
            {
                std::string message = ":" + server->channels[channel]->users[i].getNickname() + "!" + server->channels[channel]->users[i].getUsername() + " PRIVMSG " + channel + " :" + receivedMessage + "\r\n";
                if (server->channels[channel]->users[i].getSocket() != fd)
                    send_all(server->channels[channel]->users[i].getSocket(), message.c_str(), message.size(), 0);
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

void    get_username(char *buf, int fd, server *server)
{
    std::string buffer(buf);
	std::cout << "buffer = " << buffer << std::endl;
    if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
    {
        if (server->users[fd].getUsername().empty()) {
            std::string username = buffer.substr(buffer.find("USER") + 5);
			if(check_valid(username) == 1)
			{
				send_all(fd, "Please enter a valid username: USER <username>\n", 47, 0);
				return ;
			}
			while(username[username.size() - 1] == ' ' || username[username.size() - 1] == '\t')
				username = username.substr(0, username.size() - 1);
			while(username[0] == ' ' || username[0] == '\t')
				username = username.substr(1);
            std::size_t endPos = username.find_first_of("\r\n");
            if (endPos != std::string::npos)
                username = username.substr(0, endPos);
            server->users[fd].setUsername(username);
            std::cout << "Username set to: |" << username << "|\n";
            server->users[fd].setStatus(2);
            std::cout << "status = " << server->users[fd].getStatus() << "\n";
        }
        else
            send_all(fd, "You may not reregister\n", 23, 0);
    }
    else if(server->users[fd].getUsername().empty())
        send_all(fd, "Please enter your username: USER <username>\n", 45, 0);
}

void    get_username_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::cout << "buffer = " << buffer << std::endl;
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
			server->users[fd].setUsername(username);
			std::cout << "Username set to: |" << username << "|\n";
			server->users[fd].setStatus(1);
			std::cout << "status = " << server->users[fd].getStatus() << "\n";
		}
		else
			send_all(fd, "You may not reregister\n", 23, 0);
	}
	else if(server->users[fd].getUsername().empty())
		send_all(fd, "Please enter your username: USER <username>\n", 45, 0);
}

void    get_nickname_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
	{
		std::string oldNick = server->users[fd].getNickname();
		std::string nick = buffer.substr(buffer.find("NICK") + 5);
		while(nick[nick.size() - 1] == ' ' || nick[nick.size() - 1] == '\t')
			nick = nick.substr(0, nick.size() - 1);
		std::size_t endPos = nick.find_first_of("\r\n");
		if (endPos != std::string::npos)
			nick = nick.substr(0, endPos);
		if (oldNick.compare(nick) == 0)
			return ;
		if (!server->users[fd].getNickname().empty())
		{
			server->users[fd].setNickname(nick);
			std::string message = ":" + oldNick + "!" + server->users[fd].getUsername() + " NICK " + server->users[fd].getNickname() + "\r\n";
			send_all(fd, message.c_str(), message.size(), 0);
		}
		else
			server->users[fd].setNickname(nick);
		std::cout << "status = " << server->users[fd].getStatus() << "\n";
		std::cout << "Nickname set to: |" << nick << "|\n";
		server->users[fd].setStatus(2);
		std::cout << "status = " << server->users[fd].getStatus() << "\n";
	}
	else if(server->users[fd].getNickname().empty())
		send_all(fd, "Please enter your nickname: NICK <nickname>\n", 45, 0);
}

void    get_nickname(char *buf, int fd, server *server)
{
    std::string buffer(buf);
    if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
    {
        std::string oldNick = server->users[fd].getNickname();
        std::string nick = buffer.substr(buffer.find("NICK") + 5);
		if(check_valid(nick) == 1)
		{
			send_all(fd, "Please enter a valid nickname: NICK <nickname>\n", 47, 0);
			return ;
		}
		while(nick[nick.size() - 1] == ' ' || nick[nick.size() - 1] == '\t')
			nick = nick.substr(0, nick.size() - 1);
		while(nick[0] == ' ' || nick[0] == '\t')
			nick = nick.substr(1);
        std::size_t endPos = nick.find_first_of("\r\n");
        if (endPos != std::string::npos)
            nick = nick.substr(0, endPos);
        if (oldNick.compare(nick) == 0)
            return ;
        if (!server->users[fd].getNickname().empty())
        {
            server->users[fd].setNickname(nick);
            std::string message = ":" + oldNick + "!" + server->users[fd].getUsername() + " NICK " + server->users[fd].getNickname() + "\r\n";
            send_all(fd, message.c_str(), message.size(), 0);
        }
        else
            server->users[fd].setNickname(nick);
        std::cout << "status = " << server->users[fd].getStatus() << "\n";
        std::cout << "Nickname set to: |" << nick << "|\n";
        server->users[fd].setStatus(3);
    }
    else if(server->users[fd].getNickname().empty())
        send_all(fd, "Please enter your nickname: NICK <nickname>\n", 45, 0);
}

void    get_password(char *buf, int fd, server *server)
{
    std::string buffer(buf);
    if (buffer.find("PASS") != std::string::npos && (buffer.find("PASS") == 0 || buffer[buffer.find("PASS") - 1] == '\n'))
    {
        std::string pass = buffer.substr(buffer.find("PASS") + 5);
		if(check_valid(pass) == 1)
		{
			send_all(fd, "Please enter a valid password: PASS <password>\n", 47, 0);
			return ;
		}
        std::size_t endPos = pass.find_first_of("\r\n");
        if (endPos != std::string::npos)
            pass = pass.substr(0, endPos);
        if (pass != server->getPass())
        {
            std::cout << server->getPass() << std::endl;
            std::cout << pass << std::endl;
            send_all(fd, "You've entered the wrong password, please try again\n", 53, 0);
            return ;
        }
        else
        {
            server->users[fd].setStatus(1);
            return ;
        }
    }
    else if(server->users[fd].getStatus() == 0)
        send_all(fd, "Please enter the server password: PASS <password>\n", 51, 0);
}

void    get_password_hex(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("PASS") != std::string::npos && (buffer.find("PASS") == 0 || buffer[buffer.find("PASS") - 1] == '\n'))
	{
		std::string pass = buffer.substr(buffer.find("PASS") + 5);
		if(check_valid(pass) == 1)
		{
			send_all(fd, "Please enter a valid password: PASS <password>\n", 47, 0);
			return ;
		}
		std::size_t endPos = pass.find_first_of("\r\n");
		if (endPos != std::string::npos)
			pass = pass.substr(0, endPos);
		if (pass != server->getPass())
		{
			std::cout << server->getPass() << std::endl;
			std::cout << pass << std::endl;
			send_all(fd, "You've entered the wrong password, please try again\n", 53, 0);
			return ;
		}
		else
		{
			server->users[fd].setStatus(3);
			return ;
		}
	}
	else if(server->users[fd].getStatus() == 2)
		send_all(fd, "Please enter the server password: PASS <password>\n", 51, 0);
}

void    check_source(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("CAP") != std::string::npos && (buffer.find("CAP") == 0 || buffer[buffer.find("CAP") - 1] == '\n'))
	{
		server->users[fd].setFromHex(1); // Em testes
		std::cout << "from_hex = " << server->users[fd].getFromHex() << std::endl;
	}
}

int main(int argc, char **argv)
{
    (void)argv;
    if(argc != 3)
    {
        std::cout << "Error: Proper use is <./ft_irc <port> <password>\n";
        return (1);
    }
    server serverT(argv[2]); // Create a server object
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

        //std::cout << "users size = " << users.size() << "fds size = " << fds.size() << std::endl;

        for (size_t i = 1; i < fds.size(); i++)
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
                    break;
                }

                std::cout << "status from user number " << i << " = " << server->users[fds[i].fd].getStatus() << std::endl;
                if (server->users[fds[i].fd].getStatus() == -1)
				{
					check_source(buffer, fds[i].fd, server);
					server->users[fds[i].fd].setStatus(0);
				}
				if (server->users[fds[i].fd].getFromHex() == 1 && server->users[fds[i].fd].getStatus() < 3)
				{
					std::cout << "if from hex\n";
					if (server->users[fds[i].fd].getStatus() == 0)
						get_username_hex(buffer, fds[i].fd, server);
					if (server->users[fds[i].fd].getStatus() == 1)
						get_nickname_hex(buffer, fds[i].fd, server);
					if (server->users[fds[i].fd].getStatus() == 2)
						get_password_hex(buffer, fds[i].fd, server);
				}
				else if (server->users[fds[i].fd].getFromHex() == 0 && server->users[fds[i].fd].getStatus() < 3)
				{
					std::cout << "if not from hex\n";
					if (server->users[fds[i].fd].getStatus() == 0)
						get_password(buffer, fds[i].fd, server);
					if (server->users[fds[i].fd].getStatus() == 1)
						get_username(buffer, fds[i].fd, server);
					if (server->users[fds[i].fd].getStatus() == 2)
						get_nickname(buffer, fds[i].fd, server);
				}
				if(server->users[fds[i].fd].getStatus() == 3)
				{
					send_all(fds[i].fd, "Congratulations, you are now connected to the server!\n", 55, 0);
					server->users[fds[i].fd].setStatus(4);
				}
//                check_login(buffer, fds[i].fd, server);
//                check_channel(buffer, fds[i].fd, server);
//                check_priv(buffer, fds[i].fd, server);
            }
        }
    }
}

//CAP LS 302
//NICK ldiogo
//USER ldiogo 0 * :realname

// testar com o hexchat
// se o nc enviar as mensagens do hexchat

