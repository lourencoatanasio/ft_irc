
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