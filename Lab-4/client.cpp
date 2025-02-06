#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
int main()
{
    // Create a socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        std::cerr << "Error creating socket\n";
        return -1;
    }
    // Connect to the server
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server's IP address
        serverAddress.sin_port = htons(8080); // Server's port
    if (connect(clientSocket, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) == -1)
    {

        std::cerr << "Error connecting to server\n";
        close(clientSocket);
        return -1;
    }
    // Send data to the server
    const char *message = "Hello Message";
    if (send(clientSocket, message, strlen(message), 0) == -1)
    {
        std::cerr << "Error sending data\n";
        close(clientSocket);
        return -1;
    }
    // Close socket
    close(clientSocket);
    return 0;
}