//
// Created by hcorrea- on 07-05-2024.
//

#pragma once

#include "main.hpp"
#include "client.hpp"

class user;

class channel {
	
	public:
		channel();
		~channel();
		bool		getTopicMode(void){return(topicMode);}
		bool		getInviteMode(void){return(inviteMode);}
		std::string	getPassword(void){return(passwd);}
		void		setTopicMode(bool status){topicMode = status;}
		void		setInviteMode(bool status){inviteMode = status;}
		void		setPassword(std::string pass){passwd = pass;}
		std::map<int, user> users;

	private:
		bool		topicMode;
		bool		inviteMode;
		std::string	passwd;
};