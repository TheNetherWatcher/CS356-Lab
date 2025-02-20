#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8080
#define BUF_SIZE 100

int is_delim(char c) {
    return (c == ' ' || c == '\t' || c == ',' || c == ';' || c == ':' || c == '.');
}

int main(void) {  
    int sock_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];
    char filename[256];
    char new_filename[300];
    int file_fd;
    ssize_t total_bytes = 0, total_words = 0;
    int in_word = 0;
    ssize_t bytes_received;
    int data_received = 0;
    ssize_t i;  

    memset(&serv_addr, 0, sizeof(serv_addr));

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("invalid address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Enter the filename to request: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        perror("Error reading filename");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    filename[strcspn(filename, "\n")] = '\0';

    if (write(sock_fd, filename, strlen(filename)) < 0) {
        perror("Error sending filename");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    snprintf(new_filename, sizeof(new_filename), "server_%s", filename);
    file_fd = open(new_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file_fd < 0) {
        perror("Error opening file for writing");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    while ((bytes_received = read(sock_fd, buffer, BUF_SIZE)) > 0) {
        data_received = 1;
        if (write(file_fd, buffer, bytes_received) < 0) {
            perror("Error writing to file");
            close(file_fd);
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
        total_bytes += bytes_received;

        for (i = 0; i < bytes_received; i++) {
            if (!is_delim(buffer[i]) && !in_word) {
                total_words++;
                in_word = 1;
            } else if (is_delim(buffer[i])) {
                in_word = 0;
            }
        }
    }

    close(file_fd);
    close(sock_fd);

    if (!data_received) {
        printf("ERR 01: File Not Found\n");
    } else {
        printf("The file transfer is successful. Size of the file = %zd bytes, no. of words = %zd\n", 
               total_bytes, total_words);
    }

    return 0;
}