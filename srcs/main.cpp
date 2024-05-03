#include "../inc/client.hpp"
#include "../inc/server.hpp"
#include "../inc/main.hpp"

void     get_new_user(server &server, std::vector<user> &users, std::vector<pollfd> &fds)
{
    int newClientSocket = accept(server.socket_id, NULL, NULL);
    if (newClientSocket == -1) {
        std::cerr << "Can't accept client!";
        return;
    }
    user newUser(newClientSocket);
    users.push_back(newUser);
    pollfd newPfd;
    newPfd.fd = newClientSocket;
    newPfd.events = POLLIN;
    fds.push_back(newPfd);
    fds[0].revents = 0;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    server server;
    std::vector<user> users;
    std::vector<pollfd> fds;

    pollfd serverPfd;
    serverPfd.fd = server.socket_id;
    serverPfd.events = POLLIN;
    fds.push_back(serverPfd);

    char buffer[BUFFER_SIZE];

    while (true) // Main server loop
    {
        int ret = poll(fds.data(), fds.size(), -1); // Wait indefinitely for events

        if (ret == -1) {
            std::cerr << "Error in poll(). Quitting" << std::endl;
            break;
        }

        if (fds[0].revents & POLLIN) {
            get_new_user(server, users, fds);
        }

        for (size_t i = 0; i < fds.size(); ++i) {
            if (fds[i].revents & POLLIN) {
                std::memset(buffer, 0, BUFFER_SIZE);
                int bytesRead = recv(fds[i].fd, buffer, BUFFER_SIZE, 0);
                if (bytesRead == -1)
                {
                    std::cerr << "Error in recv(). Quitting" << std::endl;
                    break;
                }

                if (bytesRead == 0) {
                    std::cout << "Client disconnected" << std::endl;
                    close(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    users.erase(users.begin() + i);
                    break;
                }

                std::cout << "Received from client " << i << ": " << std::string(buffer, 0, bytesRead);
                // Echo message back to client
                send(fds[i].fd, buffer, bytesRead, 0);
            }
        }
    }
}