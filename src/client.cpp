//
// Created by ldiogo on 03-05-2024.
//

#include "../inc/client.hpp"

user::user(int newSocket)
{
	isOp = false;
    clientSize = sizeof(client);
    clientSocket = newSocket;
    if (clientSocket == -1 )
    {
        std::cerr << "Can't accept client!";
        exit(EXIT_FAILURE);
    }
    status = 0;
    std::cout << "New client connected" << std::endl;
}

user::~user()
{
}

void	user::check_operator(char *buf, int fd, server *server)
{
	std::string buffer(buf);
	if (buffer.find("MODE") != std::string::npos && (buffer.find("MODE") == 0))
	{
		std::cout << YELLOW << buffer << "\n" << NC;
		;
	}
	(void)server;
	(void)fd;
}