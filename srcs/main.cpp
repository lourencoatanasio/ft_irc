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

void     get_new_user(server &server, std::vector<user> &users, std::vector<pollfd> &fds)
{
    int newClientSocket = accept(server.socket_id, NULL, NULL);
    if (newClientSocket == -1)
    {
        std::cerr << "Can't accept client!";
        return;
    }
    user newUser(newClientSocket);
    users.push_back(newUser);
    pollfd newPfd;
    newPfd.fd = newClientSocket;
    newPfd.events = POLLIN;
    fds.push_back(newPfd);
    fds[0].revents = 0;
    send_all(newPfd.fd, "Welcome to the server!\n", 23, 0);
    send_all(newPfd.fd, "Please enter your nickname and your user: \n", 43, 0);
    send_all(newPfd.fd, "NICK <nickname>\nUSER <username>\n", 32, 0);
}

void    check_login(char *buf, user &user, int fd)
{
    std::string buffer(buf);
    std::cout << user.getSocket() << std::endl;
    if (buffer.find("NICK") != std::string::npos && (buffer.find("NICK") == 0 || buffer[buffer.find("NICK") - 1] == '\n'))
    {
        std::string nick = buffer.substr(buffer.find("NICK") + 5);
        nick = nick.substr(0, nick.find("\n"));
        user.setNickname(nick);
        std::cout << "Nickname set to: " << nick << std::endl;
    }
    if (buffer.find("USER") != std::string::npos && (buffer.find("USER") == 0 || buffer[buffer.find("USER") - 1] == '\n'))
    {
        user.setUsername(buffer.substr(buffer.find("USER") + 5, buffer.find(" ", buffer.find("USER") + 5) - buffer.find("USER") - 5));
        std::cout << "Username set to: " << user.getUsername() << std::endl;
    }

	if (user.getNickname().empty() && user.getUsername().empty())
	{
		send_all(fd, "Please enter your nickname and your user: \n", 43, 0);
		send_all(fd, "NICK <nickname>\nUSER <username>\n", 32, 0);
	}
	else if (user.getNickname().empty())
		send_all(fd, "Please enter your nickname: \nNICK <nickname>\n", 46, 0);
	else if (user.getUsername().empty())
		send_all(fd, "Please enter your username: \nUSER <username>\n", 46, 0);
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    server server;
    std::vector<user> users;
    std::vector<pollfd> fds;

    pollfd serverPfd;
    serverPfd.fd = server.socket_id;
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
            get_new_user(server, users, fds);

        for (size_t i = 0; i < fds.size(); ++i)
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
                    users.erase(users.begin() + i);
                    break;
                }

                if (users[i].getNickname().empty() || users[i].getUsername().empty())
                    check_login(buffer, users[i], fds[i].fd);
                else
                    std::cout << "Received from client " << i << ": " << std::string(buffer, 0, bytesRead);
//                // Echo message back to client
//                send(fds[i].fd, buffer, bytesRead, 0);
            }
        }
    }
}