#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
using namespace std;

#define PORT 8080
#define BUFSIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Bind failed" << endl;
        return 1;
    }

    cout << "Server is running and waiting for connections on port " << PORT << endl;

    char buffer[BUFSIZE];
    while (true) {
        socklen_t len = sizeof(client_addr);
        int received = recvfrom(sock, buffer, BUFSIZE, 0,
                              (struct sockaddr*)&client_addr, &len);

        if (received < 0) {
            cerr << "Error receiving data" << endl;
            continue;
        }

        buffer[received] = '\0';
        cout << "Received: " << buffer << " from "
                  << inet_ntoa(client_addr.sin_addr) << endl;

        // Send pong response
        string response = "PONG " + string(buffer + 5);  // Skip "PING "
        sendto(sock, response.c_str(), response.length(), 0,
               (struct sockaddr*)&client_addr, len);
    }

    close(sock);
    return 0;
}