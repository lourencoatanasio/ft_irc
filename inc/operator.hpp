
#pragma once

#include "client.hpp"

class Operator : public user
{
	private:
		/* std::string cmd_;
		std::string flag_;
		std::string channel_;
		std::string name_; */
	public:
		Operator(user &user);
		~Operator();

		/* void	setStruct(std::string cmd, std::string ch, std::string flag, std::string name);
		void	checkCommand(server *server, int fd);
		void	kick();
		void	invite();
		void	topic();
		void	mode(server *server); */
};


