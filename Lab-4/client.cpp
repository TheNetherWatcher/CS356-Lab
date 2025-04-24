#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    // Set up server address structure
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    // Receive and display server information
    int valread = read(sock, buffer, BUFFER_SIZE);
    std::cout << "Server Info:\n" << buffer << std::endl;

    while(true) {
        std::string command;
        std::cout << "Enter command (or 'exit' to quit): ";
        std::getline(std::cin, command);

        if (command == "exit") break;

        // Send command to server
        send(sock, command.c_str(), command.length(), 0);
        memset(buffer, 0, BUFFER_SIZE);
        // Receive and display server response
        valread = read(sock, buffer, BUFFER_SIZE);
        std::cout << "Server response:\n" << buffer << std::endl;
    }

    // Close the connected socket
    close(sock);
    return 0;
}
