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
	std::string	command, channelName, flag, nameOp;
	iss >> command >> channelName >> flag >> nameOp;

	if (command.compare("MODE") == 0 && (flag.compare("+o") == 0 || flag.compare("-o") == 0))
	{
		std::cout << YELLOW << this->nickname << " is trying to use MODE\n" << "Is Operator:" << std::boolalpha << isOp << NC;
		if (this->isOp == false)
		{
			std::string message = ":" + channelName + " :You're not channelName operator\r\n";
			send_all(fd, message.c_str(), message.size(), 0);
			return;
		}
		std::size_t endPos = nameOp.find_first_of("\t\n\r ");
		if (endPos != std::string::npos)
			nameOp = nameOp.substr(0, endPos);
		
		if (server->channels.find(channelName) != server->channels.end())
		{
			for (std::size_t i = 0; i < server->channels[channelName]->users.size(); i++)
			{
				std::string u = server->channels[channelName]->users[i].getUsername();
				std::string n = server->channels[channelName]->users[i].getNickname();
				
				if (u.compare(nameOp) == 0 || n.compare(nameOp) == 0)
				{
					server->channels[channelName]->printUsers();
					server->channels[channelName]->users[i].isOp = true;
					server->channels[channelName]->add_operator(server->channels[channelName]->users[i], channelName);
					std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channelName + " +o " + nameOp + "\r\n";
					send_all(fd, message.c_str(), message.size(), 0);
				}
			}
		}
	}
}