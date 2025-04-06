#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUF_SIZE 100

int main(void) {
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUF_SIZE];
    /* Declare all variables at the start */
    int opt = 1;
    int bytes_read;
    int file_fd;
    ssize_t sent_bytes;
    ssize_t n;

    /* Initialize address structure with zeros */
    memset(&address, 0, sizeof(address));

    /* Initialize TCP server socket */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running and waiting for connections on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        /* Handle client request */
        memset(buffer, 0, BUF_SIZE);
        bytes_read = read(client_fd, buffer, BUF_SIZE - 1);
        if (bytes_read <= 0) {
            close(client_fd);
            continue;
        }
        buffer[bytes_read] = '\0';
        printf("Client requested file: %s\n", buffer);

        /* Process file transfer */
        file_fd = open(buffer, O_RDONLY);
        if (file_fd < 0) {
            close(client_fd);
            continue;
        }

        while ((bytes_read = read(file_fd, buffer, BUF_SIZE)) > 0) {
            sent_bytes = 0;
            while (sent_bytes < bytes_read) {
                n = write(client_fd, buffer + sent_bytes, bytes_read - sent_bytes);
                if (n <= 0) {
                    perror("write to client failed");
                    break;
                }
                sent_bytes += n;
            }
        }
        close(file_fd);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}