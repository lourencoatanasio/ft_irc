//
// Created by hcorrea- on 07-05-2024.
//

#include "../inc/channel.hpp"

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
