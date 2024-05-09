//
// Created by ldiogo on 03-05-2024.
//

#include "../inc/client.hpp"

user::user(int newSocket)
{
	isOp = false;
    clientSize = sizeof(client);
    clientSocket = newSocket;
    if (clientSocket == -1 )
    {
        std::cerr << "Can't accept client!";
        exit(EXIT_FAILURE);
    }
    status = 0;
    std::cout << "New client connected" << std::endl;
}

user::~user()
{
}

void	user::check_operator(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::istringstream iss(buffer);
	std::string	command, channel, flag, nameOp;
	iss >> command >> channel >> flag >> nameOp;

	if (command.compare("MODE") == 0 && (flag.compare("+o") == 0 || flag.compare("-o") == 0))
	{
		std::size_t endPos = nameOp.find_first_of("\t\n\r ");
		if (endPos != std::string::npos)
			nameOp = nameOp.substr(0, endPos);
		
		if (server->channels.find(channel) != server->channels.end())
		{
			for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++)
			{
				std::string u = server->channels[channel]->users[i].getUsername();
				std::string n = server->channels[channel]->users[i].getNickname();
				
				if (u.compare(nameOp) == 0 || n.compare(nameOp) == 0)
				{
					std::cout << YELLOW << server->channels[channel]->users.size() << NC << "\n";
					std::cout << YELLOW << server->users[fd].getUsername() << NC << "\n";
					std::cout << YELLOW << server->users[i].getUsername() << NC << "\n";
					server->channels[channel]->add_operator(server->users[fd], channel);
					send_all(fd, "TEST\r\n", 8, 0);
				}
			}
		}
	}
	(void)server;
	(void)fd;
}