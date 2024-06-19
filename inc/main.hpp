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
#include <filesystem>
#include <cstdio>
#include "client.hpp"
#include "channel.hpp"
#include "server.hpp"
#include <csignal>
#include <ctime>
#include <algorithm>

#define NC "\033[0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"

class user;
class server;

ssize_t send_user(int socket, const void *buffer, size_t length, int flags);
ssize_t	send_all(server *server, const void *buffer, size_t lenght, int flags, std::string channel);
void	check_source(int fd, server *server, int ret);
void	sigHandler();
void	login(int i, server *server, std::vector<pollfd> &fds, char *buffer);
void	get_username(char *buf, int fd, server *server);
void	get_username_hex(char *buf, int fd, server *server);
void	get_nickname_hex(char *buf, int fd, server *server);
void	get_nickname(char *buf, int fd, server *server);
void    get_password(char *buf, int fd, server *server);
void	get_password_hex(char *buf, int fd, server *server);
ssize_t send_user(int socket, const void *buffer, size_t length, int flags);
ssize_t send_all(server *server, const void *buffer, size_t length, int flags, std::string channel);
void get_new_user(server *server, std::vector<pollfd> &fds);
int check_valid(std::string buffer);
void channel_send(server *server, std::string nick, std::string username, std::string channelName, int fd);
void check_channel(char *buf, int fd, server *server);
void check_priv(char *buf, int fd, server *server);
void check_source(int fd, server *server, int ret);
void check_still_building(int fd, server *server);
int	check_valid_command(std::string str);
int	checkInvited(std::vector<std::string> *invitedChannels, std::string channel);
int    check_leave(std::vector<pollfd> &fds, server *server, char *buffer, int fd, int i);
std::string get_channel(char *buf, server *server);
size_t check_message(std::string buffer);
void	bot_timeout(server *server, char *buffer, int fd);
std::string get_user(std::string buf, server *server);
int channel_size(server *server, std::string channel);
