//
// Created by ldiogo on 03-05-2024.
//

#pragma once

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
#include "client.hpp"
#include "channel.hpp"
#include "server.hpp"
#include <csignal>


#define NC "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define MAGENTA "\033[0;35m"

class user;
class server;

ssize_t send_user(int socket, const void *buffer, size_t length, int flags);
ssize_t	send_all(server *server, const void *buffer, size_t lenght, int flags, std::string channel);
