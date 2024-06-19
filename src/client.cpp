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
	from_nc = 0;
	still_building = 0;
	buffer[0] = '\0';
	final_buffer[0] = '\0';
	flag = 0;
	timeout = 0;
	time_start = 0;
	current_time = 0;
	invited.clear();
	std::cout << "New client connected" << std::endl;
}

user::~user()
{
}

int	user::modeInvite(server *server, std::string channel, std::string flag)
{
	if (flag.compare("+i") == 0 && server->channels[channel]->getInviteMode() == false)
		server->channels[channel]->setInviteMode(true);
	else if (flag.compare("-i") == 0 && server->channels[channel]->getInviteMode() == true)
		server->channels[channel]->setInviteMode(false);
	else
		return (1);
	std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + "\r\n";
	send_all(server, message.c_str(), message.size(), 0, channel);
	return (0);
}

int	user::modeTopic(server *server, std::string channel, std::string flag)
{
	if (flag.compare("+t") == 0 && server->channels[channel]->getTopicMode() == false)
		server->channels[channel]->setTopicMode(true);
	else if (flag.compare("-t") == 0 && server->channels[channel]->getTopicMode() == true)
		server->channels[channel]->setTopicMode(false);
	else
		return (1);
	std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + "\r\n";
	send_all(server, message.c_str(), message.size(), 0, channel);
	return (0); 
};

void	user::modeOperator(server *server, user &newOp, std::string channel, std::string flag)
{
	if (flag.compare("+o") == 0 && newOp.getOpStatus() == false) {
		newOp.setOpStatus(true);
		server->channels[channel]->setOps(server->channels[channel]->getOps() + 1);
	}
	else if (flag.compare("-o") == 0 && newOp.getOpStatus() == true)
	{
		if (server->channels[channel]->getOps() == 1)
		{
			std::string message = ": 482 " + this->nickname + " " + channel + " You're the sole Operator of this channel! Don't give up!\r\n";
			send_user(this->clientSocket, message.c_str(), message.size(), 0);
			return ;
		}
		newOp.setOpStatus(false);
		server->channels[channel]->setOps(server->channels[channel]->getOps() - 1);
	}
	else
		return ;
	std::string message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + " " + newOp.getNickname() + "\r\n";
	send_all(server, message.c_str(), message.size(), 0, channel);
}

int	user::modePassword(server *server, std::string channel, std::string flag, std::string key)
{
	std::string message;
	if (key.empty())
		return (1);
	if (flag.compare("+k") == 0 && server->channels[channel]->getPassword().empty())
	{
		server->channels[channel]->setPassword(key);
		message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + " " + key + "\r\n";
	}
	else if (flag.compare("-k") == 0  && key.compare(server->channels[channel]->getPassword()) == 0)
	{
		server->channels[channel]->setPassword("");
		message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + " " + key + "\r\n";
	}
	else if (flag.compare("+k") == 0 || (flag.compare("-k") == 0 && key.compare(server->channels[channel]->getPassword()) != 0))
	{
		message = ":" + channel + " 467 " + this->nickname + " " + channel + " :Channel key already set\r\n";
		send_user(clientSocket, message.c_str(), message.size(), 0);
		return (1);
	}
	send_all(server, message.c_str(), message.size(), 0, channel);
	return (0);
}

int	user::modeLimit(server *server, std::string channel, std::string flag, std::string amount)
{
	std::string message;
	if (flag.compare("+l") == 0 && server->channels[channel]->getMaxUsers() >= 0)
	{
		int	n = atoi(amount.c_str());
		if (n <= 0 || n == server->channels[channel]->getMaxUsers())
			return (1);
		server->channels[channel]->setmaxUsers(n);
		size_t nSize = amount.find_first_not_of("0123456789");
		message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + " " + amount.substr(0, nSize) + "\r\n";
	}
	else if (flag.compare("-l") == 0 && server->channels[channel]->getMaxUsers() > 0)
	{
		server->channels[channel]->setmaxUsers(0);
		message = ":" + this->nickname + "!" + this->username + " MODE " + channel + " " + flag + "\r\n";
	}
	else
		return (1);
	send_all(server, message.c_str(), message.size(), 0, channel);
	return (0);
}

int	user::opCheck(server *server, std::string channel, int fd)
{
	if (server->channels[channel]->users[fd].isOp == false)
	{
		std::string message = ":" + channel + " 482 " + this->nickname + " " + channel + " :You're not channel operator\r\n";
		send_user(fd, message.c_str(), message.size(), 0);
		return (1);
	}
	return (0);
}

void	user::mode(server *server, char *buf, int fd)
{
	std::string	command, channel, flag, nameOp;
	std::istringstream iss2(buf);
	iss2 >> command >> channel >> flag >> nameOp;
	
	if (server->channels[channel]->users[fd].opCheck(server, channel, fd) 
		|| server->channels.find(channel) == server->channels.end())
		return ;
	if (server->channels[channel]->users[fd].modeLimit(server, channel, flag, nameOp) &&
		server->channels[channel]->users[fd].modeInvite(server, channel, flag) &&
	server->channels[channel]->users[fd].modeTopic(server, channel, flag) &&
	server->channels[channel]->users[fd].modePassword(server, channel, flag, nameOp))
		return ;
	std::size_t endPos = nameOp.find_first_of("\t\n\r ");
	if (endPos != std::string::npos)
		nameOp = nameOp.substr(0, endPos);
	for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++)
	{	
		if ((server->channels[channel]->users[i].getUsername().compare(nameOp) == 0 
		|| server->channels[channel]->users[i].getNickname().compare(nameOp) == 0))
		{
			modeOperator(server, server->channels[channel]->users[i], channel, flag);
		}
	}
}

void	user::kick(server *server, char *buf, int fd)
{
	std::string command, channel, flag, nameOp;
	std::istringstream iss2(buf);
	iss2 >> command >> channel >> flag >> nameOp;
	if (server->channels[channel]->users[fd].opCheck(server, channel, fd))
		return;
	std::size_t endPos = nameOp.find_first_of("\t\n\r ");
	if (endPos != std::string::npos)
		nameOp = nameOp.substr(0, endPos);
	if (server->channels.find(channel) != server->channels.end()) {
		for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++) {
			std::string u = server->channels[channel]->users[i].getUsername();
			std::string n = server->channels[channel]->users[i].getNickname();
			if (u.compare(nameOp) == 0 || n.compare(nameOp) == 0) {
				std::string message =
						":" + this->nickname + "!" + this->username + " KICK " + channel + " " + flag + " :" +
						this->nickname + "\r\n";
				for (std::size_t j = 0; j < server->channels[channel]->users.size(); j++) {
					send_user(server->channels[channel]->users[j].getSocket(), message.c_str(), message.size(), 0);
				}
				server->channels[channel]->users.erase(i);
				break;
			}
		}
	}
}

void	user::invite(server *server, char *buf, int fd)
{
	std::string command, channel, nameOp;
	std::istringstream iss2(buf);
	iss2 >> command >> nameOp >> channel;
	bool userExists = false;

	for (std::size_t i = 0; i < server->users.size(); i++) {
		if (server->users[i].getNickname().compare(nameOp) == 0) {
			userExists = true;
			break;
		}
	}

	if (!userExists) {
		std::string message = ":" + channel + " 401 " + this->nickname + " " + nameOp + " :No such nick\r\n";
		send_user(fd, message.c_str(), message.size(), 0);
		return;
	}

	if (server->channels.find(channel) == server->channels.end()) {
		std::string message = ":" + channel + " 403 " + this->nickname + " " + channel + " :No such channel\r\n";
		send_user(fd, message.c_str(), message.size(), 0);
		return;
	}

	if (server->channels[channel]->getInviteMode() == true) {
		if (server->channels[channel]->users[fd].opCheck(server, channel, fd))
			return ;
	}

	std::size_t endPos = nameOp.find_first_of("\t\n\r ");
	if (endPos != std::string::npos)
		nameOp = nameOp.substr(0, endPos);

	for (std::size_t i = 0; i < server->channels[channel]->users.size(); i++) {
		if (server->channels[channel]->users[i].getNickname().compare(nameOp) == 0) {
			std::string message = ":" + channel + " 443 " + this->nickname + " " + nameOp + " " + channel + " :is already on channel\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			return;
		}
	}

	for (std::size_t i = 0; i < server->users.size(); i++) {
		if (server->users[i].getNickname().compare(nameOp) == 0) {
			std::string message = ":" + this->nickname + "!" + this->username + " INVITE " + nameOp + " " +
									channel + "\r\n";
			server->users[i].invited.push_back(channel);
			send_user(server->users[i].getSocket(), message.c_str(), message.size(), 0);
			message = ":" + channel + " 341 " + this->nickname + " " + nameOp + " " + channel + "\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			return;
		}
	}
}

void	user::topic(server *server, char *buf, int fd)
{
	std::string command, channel, topic;
	std::istringstream iss2(buf);
	iss2 >> command >> channel;
	std::getline(iss2, topic);
	topic = topic.substr(1);
	std::size_t endPos = topic.find_first_of("\t\n\r");
	if (!topic.empty() && topic[0] == ':')
		topic = topic.substr(1);
	if (endPos != std::string::npos)
		topic = topic.substr(0, endPos);
	if (topic.empty() == true) {
		if (server->channels[channel]->getTopic().empty() == true) {
			std::string message = ":" + channel + " 331 " + this->nickname + " " + channel + " :No topic is set.\r\n";
			send_user(fd, message.c_str(), message.size(), 0);
			return;
		}
		std::string message = ":" + channel + " 332 " + this->nickname + " " + channel + " :" + server->channels[channel]->getTopic() + "\r\n";
		send_user(fd, message.c_str(), message.size(), 0);
		message = ":" + channel + " 333 " + this->nickname + " " + channel + " " + server->channels[channel]->getNick() + "!" + server->channels[channel]->getUser() +
					" 1715866598\r\n";
		send_user(fd, message.c_str(), message.size(), 0);
		return;
	} else if (server->channels[channel]->getTopicMode() == true) {
		if (server->channels[channel]->users[fd].opCheck(server, channel, fd))
			return;
	}
	std::string message = ":" + this->nickname + "!" + this->username + " TOPIC " + channel + " :" + topic +
							"\r\n";
	send_all(server, message.c_str(), message.size(), 0, channel);
	server->channels[channel]->setTopic(topic);
	server->channels[channel]->setNick(this->nickname);
	server->channels[channel]->setUser(this->username);
}

int check_valid_channel(std::string channel, server *server)
{
	if (server->channels.find(channel) == server->channels.end())
		return (0);
	return (1);
}

void	user::check_operator(char *buf, int fd, server *server) {
	std::string buffer(buf), command, channel, flag;
	std::istringstream iss(buffer);
	iss >> command >> channel >> flag;

	if(command.compare("INVITE") == 0 && check_valid_channel(flag, server))
	{
		invite(server, buf, fd);
	}
	if (check_valid_channel(channel, server) == 1)
	{
		if (command.compare("MODE") == 0 && !flag.empty())
			mode(server, buf, fd);
		if (command.compare("KICK") == 0)
			kick(server, buf, fd);
		if (command.compare("TOPIC") == 0)
			topic(server, buf, fd);
	}
}

void	user::change_nick(char *buf, int fd, server *server)
{
	std::string buffer(buf), cmd, nick, message;
	std::istringstream iss(buffer);
	iss >> cmd >> nick;
	if (cmd.compare("NICK") == 0 && !nick.empty())
	{
		while (nick[nick.size() - 1] == ' ' || nick[nick.size() - 1] == '\t')
			nick = nick.substr(0, nick.size() - 1);
		std::size_t endPos = nick.find_first_of("\r\n");
		if (endPos != std::string::npos)
			nick = nick.substr(0, endPos);
		nick = nick.substr(0, nick.find(" "));
		if (this->check_same_nick(nick, server) == 1 && this->nickname != nick)
		{
			if(from_nc == 0)
				send_user(fd, "Nickname is erroneous or already in use. Use /NICK to try another.\r\n", 69, 0);
			else
				send_user(fd, "Nickname already taken, please try again\r\n", 42, 0);
			return;
		}
		else if (this->nickname == nick){
			return ;
		}
		for (std::map<std::string, channel*>::iterator it = server->channels.begin(); it != server->channels.end(); it++)
		{
			for (std::map<int, user>::iterator it2 = it->second->users.begin(); it2 != it->second->users.end(); it2++)
			{
				if (this->nickname == it2->second.getNickname())
				{
					message = ":" + this->nickname + "!" + this->username + " NICK :" + nick + "\r\n";
					send_all(server, message.c_str(), message.size(), 0, it->first);
					it2->second.setNickname(nick);
					
				}
			}
		}
		message = ":" + this->nickname + "!" + this->username + " NICK :" + nick + "\r\n";
		send_user(fd, message.c_str(), message.size(), 0);
		this->setNickname(nick);
	}
}

int channel_size(server *server, std::string channel) {
	int size = 0;
	for (std::map<int, user>::iterator it = server->channels[channel]->users.begin(); it != server->channels[channel]->users.end(); it++) {
		if (!it->second.getNickname().empty())
			size++;
	}
	return (size);
}

void	user::part(server *server, const char *buf)
{
	std::string cmd, channelName, message, reason;
	std::istringstream iss(buf);
	iss >> cmd >> channelName >> reason;
	if (cmd.compare("PART") == 0 && !channelName.empty() && check_valid_channel(channelName, server) == 1)
	{
		for (std::map<int, user>::iterator it = server->channels[channelName]->users.begin(); it != server->channels[channelName]->users.end(); it++)
		{	
			if (it->second.getNickname() == this->nickname)
			{
				if (server->channels[channelName]->users[it->first].isOp)
				{
					server->channels[channelName]->setOps(server->channels[channelName]->getOps() - 1);
					if (server->channels[channelName]->getOps() == 0)
					{
						for (std::map<int, user>::iterator it2 = server->channels[channelName]->users.begin();
							it2 != server->channels[channelName]->users.end(); ++it2) {
							if (it2->second.getNickname() != this->nickname && it2->second.isOp == false && !it2->second.getNickname().empty())
							{
								it2->second.setOpStatus(true);
								server->channels[channelName]->setOps(server->channels[channelName]->getOps() + 1);
								message = ":" + this->nickname + "!" + this->username + " MODE " + channelName + " " + "+o" + " " + it2->second.getNickname() + "\r\n";
								send_all(server, message.c_str(), message.size(), 0, channelName);
								message = ":" + this->nickname + "!" + this->username + " PART " + channelName + " " + reason + "\r\n";
								send_all(server, message.c_str(), message.size(), 0, channelName);
								server->channels[channelName]->users.erase(it);
								return ;
							}
						}
						if (channel_size(server, channelName) == 1)
						{
							message = ":" + this->nickname + "!" + this->username + " PART " + channelName + " " + reason + "\r\n";
							send_all(server, message.c_str(), message.size(), 0, channelName);
							server->channels[channelName]->users.erase(it);
							delete server->channels[channelName];
							server->channels.erase(channelName);
							return ;
						}
					}
					else
					{
						message = ":" + this->nickname + "!" + this->username + " PART " + channelName + " " + reason + "\r\n";
						send_all(server, message.c_str(), message.size(), 0, channelName);
						server->channels[channelName]->users.erase(it);
						return ;
					}
				}
				else
				{
					message = ":" + this->nickname + "!" + this->username + " PART " + channelName + " " + reason + "\r\n";
					send_all(server, message.c_str(), message.size(), 0, channelName);
					server->channels[channelName]->users.erase(it);
					return ;
				}
			}
		}
	}
}

int user::check_same_nick(std::string nick, server *server)
{
	std::map<int, user>::iterator it;
	for (it = server->users.begin(); it != server->users.end(); ++it)
	{
		if (it->second.getNickname() == nick && this->getNickname() != it->second.getNickname())
			return (1);
	}
	return (0);
}

void	user::setFinalBuffer(char *buf)
{
	char tmp[BUFFER_SIZE];

	std::strcpy(tmp, buf);
	std::strcpy(final_buffer, tmp);
}
