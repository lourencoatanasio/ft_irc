
#pragma once

#include "client.hpp"

class Operator : public user
{
	private:
		user	&user_;

	public:
		Operator(user &user);
		~Operator();
};
