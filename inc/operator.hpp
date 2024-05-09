
#pragma once

#include "client.hpp"

class Operator : public user
{
	public:
		Operator(user &user);
		~Operator();
};
