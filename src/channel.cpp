//
// Created by hcorrea- on 07-05-2024.
//

#include "../inc/channel.hpp"
#include "../inc/operator.hpp"

channel::channel()
{
}

channel::~channel()
{
}

void channel::add_user(user user)
{
	this->users.push_back(user);
}

void channel::add_operator(user &user, std::string channelName)
{
	Operator op(user);
	this->ops.push_back(op);
	std::cout << GREEN << op.getUsername() << ": is an Operator of the channel " << channelName << NC << "\n";
}