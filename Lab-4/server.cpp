#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    // Create a socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Error creating socket\n";
        return -1;
    }
    // Bind the socket to an IP address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;     // Listen on any available interface
    serverAddress.sin_port = htons(8080); // Use port 8080
    if (bind(serverSocket, (struct sockaddr *)&serverAddress,
             sizeof(serverAddress)) == -1)
    {

        std::cerr << "Error binding socket\n";
        close(serverSocket);
        return -1;
    }
    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "Error listening for connections\n";
        close(serverSocket);
        return -1;
    }
    // Accept a connection
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1)
    {
        std::cerr << "Error accepting connection\n";
        close(serverSocket);
        return -1;
    }
    // Receive data from the client
    char buffer[1024];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == -1)
    {
        std::cerr << "Error receiving data\n";
        close(serverSocket);
        close(clientSocket);
        return -1;
    }
    // Display the received message
    std::cout << "Received message from client: " << buffer << "\n";
    // Close sockets
    close(serverSocket);
    close(clientSocket);
    return 0;
}