//
// Created by ldiogo on 03-05-2024.
//

#include "../inc/server.hpp"

server::server()
{
    this->server_ip = "127.0.0.2";
    this->socket_id = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        std::cerr << "Error setting socket options\n";
        close(socket_id);
        exit(EXIT_FAILURE);
    }
    if (this->socket_id == -1) {
        std::cerr << "Can't create a socket!";
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(socket_id, F_GETFL, 0);
    fcntl(socket_id, F_SETFL, flags & ~O_NONBLOCK);

    this->IP.sin_family = AF_INET;
    if (inet_pton(AF_INET, this->server_ip, &this->IP.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        exit(EXIT_FAILURE);
    }
    this->IP.sin_port = htons(PORT);
    if (bind(this->socket_id, (sockaddr * ) & this->IP, sizeof(this->IP)) == -1) {
        std::cerr << "Can't bind to IP/port";
        exit(EXIT_FAILURE);
    }
    if (listen(this->socket_id, SOMAXCONN) == -1) {
        std::cerr << "Can't listen!";
        exit(EXIT_FAILURE);
    }
}

server::~server()
{
    close(this->socket_id);
}
