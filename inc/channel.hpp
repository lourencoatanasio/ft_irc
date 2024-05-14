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

        std::map<int, user> users;
		std::vector<Operator> ops;
    private:
};