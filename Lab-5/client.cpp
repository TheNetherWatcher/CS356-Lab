// client.cpp - Simple File Client Implementation in C++
// Connects to the server, sends commands, and receives file data.

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"  // Localhost IP
#define SERVER_PORT 8081  // Port number where server is listening

using namespace std;

// Function to receive and display data from the server
void receive_data(int socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(socket, buffer, BUFFER_SIZE, 0);
    if (bytes_received > 0) {
        cout << buffer << endl;  // Print received message from server
    }
}

// Function to handle file download from server
void get_file(int socket, const string &filename) {
    string command = "GET " + filename + "\n";  // Format GET command
    send(socket, command.c_str(), command.size(), 0);  // Send command to server

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = recv(socket, buffer, BUFFER_SIZE, 0);  // Receive response
    string response(buffer, bytes_received);  // Use bytes_received to construct the response

    if (response.substr(0, 4) == "SIZE") {  // Check if response contains file size
        size_t header_end = response.find('\n');
        if (header_end == string::npos) {
            cout << "Invalid header format" << endl;
            return;
        }

        size_t file_size = stoi(response.substr(5, header_end - 5));  // Extract file size
        ofstream file(filename, ios::binary);  // Open file to write received data
        ofstream copy_file("copy_of_" + filename, ios::binary); // Open copy file

        // Write any data received after the header
        string file_data = response.substr(header_end + 1);
        file.write(file_data.data(), file_data.size());
        copy_file.write(file_data.data(), file_data.size());
        size_t received_bytes = file_data.size();

        // Continue receiving remaining data
        while (received_bytes < file_size) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes = recv(socket, buffer, BUFFER_SIZE, 0);
            if (bytes <= 0) break;  // Connection closed or error
            file.write(buffer, bytes);
            copy_file.write(buffer, bytes);
            received_bytes += bytes;
        }

        file.close();  // Close original file
        copy_file.close();  // Close copy file

        cout << "File received successfully: " << filename << " (" << received_bytes << " bytes)\n";
        cout << "Copy created as: copy_of_" << filename << "\n";
    } else {
        cout << string(buffer, bytes_received) << endl;  // Print error message
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);  // Create socket
    if (client_socket == -1) {
        cerr << "Failed to create socket\n";
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Connection failed\n";
        return 1;
    }

    receive_data(client_socket);  // Receive initial welcome message from server

    string command;
    while (true) {
        cout << "Enter command (LIST, GET <filename>, QUIT): ";
        getline(cin, command);

        if (command == "QUIT") {  // If user enters QUIT, close connection
            send(client_socket, command.c_str(), command.size(), 0);
            break;
        } else if (command.substr(0, 4) == "LIST") {  // Handle LIST command
            send(client_socket, command.c_str(), command.size(), 0);
            receive_data(client_socket);  // Receive and print file list
        } else if (command.substr(0, 3) == "GET") {  // Handle GET command
            string filename = command.substr(4);  // Extract filename
            get_file(client_socket, filename);  // Call function to download file
        }
    }

    close(client_socket);  // Close the socket connection
    return 0;
}
