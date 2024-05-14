
#include "../inc/operator.hpp"

Operator::Operator(user &user)
{
	this->isOp = true;
	this->username = user.getUsername();
	this->nickname = user.getNickname();
}

Operator::~Operator()
{
}

/* void	Operator::checkCommand(server *server, int fd)
{
	std::cout << "CHECK COMMAND\n";
	if (server->channels[channel_]->users[fd].getOpStatus() == false)
	{
		std::string message = ":" + channel_ + " :You're not channel_ operator\r\n";
		send_all(fd, message.c_str(), message.size(), 0);
		return;
	}
	if (cmd_.compare("KICK") == 0)
		kick();
	if (cmd_.compare("INVITE") == 0)
		invite();
	if (cmd_.compare("TOPIC") == 0)
		topic();
	if (cmd_.compare("MODE") == 0)
		mode(server);
}

void	Operator::setStruct(std::string cmd, std::string ch, std::string flag, std::string name)
{
	cmd_ = cmd;
	flag_ = flag;
	channel_ = ch;
	name_ = name;
}

void	Operator::kick()
{
	;
}

void	Operator::invite()
{
	;
}

void	Operator::topic()
{
	;
}

void	Operator::mode(server *server)
{
	std::cout << "MODE\n";
	std::size_t endPos = name_.find_first_of("\t\n\r ");
	if (endPos != std::string::npos)
		name_ = name_.substr(0, endPos);
	for (std::size_t i = 0; i < server->channels[channel_]->users.size(); i++)
	{
		if (server->channels[channel_]->users[i].getUsername().compare(name_) == 0
		|| server->channels[channel_]->users[i].getNickname().compare(name_) == 0)
		{
			if (flag_.compare("+o") == 0)
				server->channels[channel_]->users[i].setOpStatus(true);
			else if (flag_.compare("-o") == 0)
				server->channels[channel_]->users[i].setOpStatus(false);
			else
				return ;
			std::cout << "MESSAGE\n";
			std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channel_ + " " + flag_ + " " + name_ + "\r\n";
			for (size_t i = 0; i < server->users.size(); i++)
				send_all(server->users[i].clientSocket, message.c_str(), message.size(), 0);
		}
	}
} */