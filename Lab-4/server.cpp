#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to get server information
string get_server_info() {
    time_t now = time(0);
    char* dt = ctime(&now);
    return "Server IP: 127.0.0.1\nCurrent time: " + string(dt);
}

// Function to execute a command and return its output
string execute_command(const string& command) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "Error executing command.";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result.empty() ? "Command executed successfully." : result;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation failed" << endl;
        return -1;
    }

    // Set up server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Bind failed" << endl;
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed" << endl;
        return -1;
    }

    cout << "Server listening on port " << PORT << endl;

    while(true) {
        // Accept a new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            cerr << "Accept failed" << endl;
            continue;
        }

        // Send server information to the client
        string server_info = get_server_info();
        send(new_socket, server_info.c_str(), server_info.length(), 0);

        while(true) {
            memset(buffer, 0, BUFFER_SIZE);
            // Read command from client
            int valread = read(new_socket, buffer, BUFFER_SIZE);
            if (valread <= 0) break;

            string command(buffer);
            // Execute the command and get the result
            string result = execute_command(command);
            // Send the result back to the client
            send(new_socket, result.c_str(), result.length(), 0);
        }

        // Close the connected socket
        close(new_socket);
    }

    return 0;
}
