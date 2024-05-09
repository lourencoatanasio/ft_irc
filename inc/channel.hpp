//
// Created by hcorrea- on 07-05-2024.
//

#pragma once

#include "main.hpp"
#include "client.hpp"

class user;
class Operator;

class channel {
    public:
        channel();
        ~channel();
        void connect(); // Not sure if this is needed
        void send(); // Not sure if this is needed
        void receive(); // Not sure if this is needed
        void disconnect(); // Not sure if this is needed
        void add_user(user user);
		void	add_operator(user &user, std::string channelName);

        std::vector<user> users;
		std::vector<Operator> ops;
    private:
};