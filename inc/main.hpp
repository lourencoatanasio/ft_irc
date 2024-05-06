//
// Created by ldiogo on 03-05-2024.
//

#ifndef FT_IRC_GIT_MAIN_HPP
#define FT_IRC_GIT_MAIN_HPP

#include <vector>
#include <poll.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <map>
#include "server.hpp"
#include "client.hpp"

class user;
void    check_login(char *buf, user &user, int fd);

#endif //FT_IRC_GIT_MAIN_HPP
