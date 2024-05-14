//
// Created by ldiogo on 03-05-2024.
//

#include "../inc/client.hpp"
#include "../inc/operator.hpp"

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

int	user::modeInvite(user *user, std::string flag)
{
	if (flag.compare("+o") == 0)
		user.setOpStatus(true);
	else if (flag.compare("-o") == 0)
		user.setOpStatus(false);
	else
		return ;
}

void	user::modeAddRemove(server *server, user &user, std::string flag, std::string channel, std::string nameOp)
{
	if (flag.compare("+o") == 0)
		user.setOpStatus(true);
	else if (flag.compare("-o") == 0)
		user.setOpStatus(false);
	else
		return ;
	std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + " " + nameOp + "\r\n";
	for (size_t i = 0; i < server->users.size(); i++)
		send_all(server->users[i].clientSocket, message.c_str(), message.size(), 0);
}

void	user::check_operator(char *buf, int fd, server *server)
{
	std::string buffer(buf), command;
	std::istringstream iss(buffer);
	iss >> command;

	if (command.compare("MODE") == 0)
	{
		std::string	command, channel, flag, nameOp;
		std::istringstream iss2(buffer);
		iss2 >> command >> channel >> flag >> nameOp;
		if (server->channels.find(channel) == server->channels.end())
			return ;
		if (server->channels[channel]->users[fd].isOp == false)
		{
			std::string message = ":" + channel + " :You're not channel operator\r\n";
			send_all(fd, message.c_str(), message.size(), 0);
			return;
		}
		std::size_t endPos = nameOp.find_first_of("\t\n\r ");
		if (endPos != std::string::npos)
			nameOp = nameOp.substr(0, endPos);
	
		for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++)
		{
			std::string u = server->channels[channel]->users[i].getUsername();
			std::string n = server->channels[channel]->users[i].getNickname();
			
			if ((u.compare(nameOp) == 0 || n.compare(nameOp) == 0))
			{
				modeAddRemove(server, server->channels[channel]->users[i], flag, channel, nameOp);
			}
		}
	}
}