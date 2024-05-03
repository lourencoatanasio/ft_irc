#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 54000
#define BUFFER_SIZE 4096


int main() {
    // Create a socket
    const char *server_ip = "127.0.0.1";
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1) {
        std::cerr << "Can't create a socket!";
        return -1;
    }

    // Bind the socket to an IP / port
    sockaddr_in IP;
    IP.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip, &IP.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        exit(EXIT_FAILURE);
    }
    IP.sin_port = htons(PORT);

    if (bind(listening, (sockaddr * ) & IP, sizeof(IP)) == -1) {
        std::cerr << "Can't bind to IP/port";
        return -2;
    }

    // Tell Winsock the socket is for listening
    if (listen(listening, SOMAXCONN) == -1) {
        std::cerr << "Can't listen!";
        return -1;
    }

    // Wait for a connection

    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    int clientSocket = accept(listening, (sockaddr * ) & client, &clientSize);

    std::cout << "Connected" << std::endl;


    char buffer[BUFFER_SIZE];
    int  bytesRead;

    while(true) {
        // Wait for a message
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesRead == -1) {
            std::cerr << "Error in recv(). Quitting" << std::endl;
            break;
        }

        if (bytesRead == 0) {
            std::cout << "Client disconnected " << std::endl;
            break;
        }

        std::cout << "Received: " << std::string(buffer, 0, bytesRead) << std::endl;

        // Echo message back to client
        send(clientSocket, buffer, bytesRead + 1, 0);
    }

    close(clientSocket);
    close(listening);
}