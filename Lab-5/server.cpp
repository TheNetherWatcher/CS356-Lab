#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>

#define BUFFER_SIZE 1024  // Define buffer size for data transfer
#define PORT 8081  // Define port number for the server
#define SERVER_DIRECTORY "./"  // Directory containing files

using namespace std;

// Function to send a requested file to the client
void send_file(int client_socket, const string &filename) {
    ifstream file(SERVER_DIRECTORY + filename, ios::binary);
    if (!file) {
        string error_msg = "ERROR: File Not Found\n";
        send(client_socket, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    // Get file size
    file.seekg(0, ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, ios::beg);

    // Send file size to the client before transmitting the file
    ostringstream header;
    header << "SIZE " << file_size << "\n";
    send(client_socket, header.str().c_str(), header.str().size(), 0);

    // Send file in chunks to avoid memory overload
    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, BUFFER_SIZE);
        send(client_socket, buffer, file.gcount(), 0);
    }

    file.close();
    cout << "File sent successfully: " << filename << endl;
}

// Function to list all files in the server directory and send them to the client
void list_files(int client_socket) {
    DIR *dir = opendir(SERVER_DIRECTORY);
    if (!dir) {
        string error_msg = "ERROR: Unable to open directory\n";
        send(client_socket, error_msg.c_str(), error_msg.size(), 0);
        return;
    }

    struct dirent *entry;
    ostringstream file_list;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {  // Include only regular files
            file_list << entry->d_name << "\n";
        }
    }
    closedir(dir);
    send(client_socket, file_list.str().c_str(), file_list.str().size(), 0);
}

// Function to handle a client session and process commands
void handle_client(int client_socket) {
    send(client_socket, "Welcome to Simple File Server\n", 30, 0);
    char buffer[BUFFER_SIZE];

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) break;  // Break if no data received

        string command(buffer);
        if (command.substr(0, 4) == "LIST") {  // Handle LIST command
            list_files(client_socket);
        } else if (command.substr(0, 3) == "GET") {  // Handle GET command
            string filename = command.substr(4);
            filename.erase(filename.find_last_not_of(" \n\r\t") + 1);  // Trim filename
            send_file(client_socket, filename);
        } else if (command.substr(0, 4) == "QUIT") {  // Handle QUIT command
            break;
        }
    }
    close(client_socket);  // Close client connection
}

// Main function to initialize and start the server
int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        cerr << "Failed to create socket\n";
        return 1;
    }

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Bind failed\n";
        return 1;
    }

    if (listen(server_socket, 5) < 0) {
        cerr << "Listen failed\n";
        return 1;
    }

    cout << "Server started on port " << PORT << "\n";

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket >= 0) {
            cout << "Client connected." << endl;
            handle_client(client_socket);
        }
    }

    close(server_socket);  // Close server socket
    return 0;
}
