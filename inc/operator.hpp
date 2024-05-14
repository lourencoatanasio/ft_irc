
#pragma once

#include "client.hpp"

class Operator : public user
{
	private:
		std::string cmd_;
		std::string flag_;
		std::string channel_;
		std::string name_;
	public:
		Operator(user &user);
		~Operator();
};


