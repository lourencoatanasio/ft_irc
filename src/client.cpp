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

void	user::check_operator(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	std::istringstream iss(buffer);
	std::string	command, channel, flag, nameOp;
	iss >> command >> channel >> flag >> nameOp;

    if (command.compare("MODE") == 0 && (flag.compare("+o") == 0 || flag.compare("-o") == 0))
    {
        if (server->channels[channel]->users[fd].isOp == false)
        {
            std::string message = ":" + channel + " :You're not channel operator\r\n";
            send_all(fd, message.c_str(), message.size(), 0);
            return;
        }
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
                    server->channels[channel]->users[i].setOpStatus(true);
                    /* server->channels[channel]->add_operator(server->channels[channel]->users[i], channel); */
                    std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " +o " + nameOp + "\r\n";
                    for (size_t i = 0; i < server->users.size(); i++)
                    {
                        send_all(server->users[i].clientSocket, message.c_str(), message.size(), 0);
                    }

                }
            }
        }
    }
    else if (command.compare("KICK") == 0)
    {
        if (server->channels[channel]->users[fd].isOp == false)
        {
            std::string message = ":" + channel + " :You're not channel operator\r\n";
            send_all(fd, message.c_str(), message.size(), 0);
            return;
        }
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
                    std::cout << flag << std::endl;
                    std::string message = ":" + this->nickname + "!" + this->username + " KICK " + channel + " " + flag + " :" + this->nickname + "\r\n";
                    for (std::size_t j = 0; j < server->channels[channel]->users.size(); j++)
                    {
                        send_all(server->channels[channel]->users[j].getSocket(), message.c_str(), message.size(), 0);
                    }
                    break;
                }
            }
        }
    }
}