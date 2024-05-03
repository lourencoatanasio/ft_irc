//
// Created by ldiogo on 03-05-2024.
//

#include "../inc/client.hpp"

user::user(int newSocket)
{
    clientSize = sizeof(client);
    clientSocket = newSocket;
    if (clientSocket == -1 )
    {
        std::cerr << "Can't accept client!";
        exit(EXIT_FAILURE);
    }
    std::cout << "Client connected 1" << std::endl;
}

user::~user()
{
    close(clientSocket);
}