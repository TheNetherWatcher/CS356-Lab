#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
using namespace std;

#define PORT 8080
#define PING_INTERVAL 1  // 1 second
#define MAX_PINGS 10
#define BUFSIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "Socket creation failed" << endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost

    // Set socket timeout
    struct timeval tv;
    tv.tv_sec = 1;  // 1 second timeout
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        cerr << "Error setting timeout" << endl;
        return 1;
    }

    char buffer[BUFSIZE];
    int sent_packets = 0;
    int received_packets = 0;

    for (int i = 0; i < MAX_PINGS; i++) {
        string message = "PING " + to_string(i) + " ";
        struct timeval start, end;

        gettimeofday(&start, NULL);
        
        // Send ping
        if (sendto(sock, message.c_str(), message.length(), 0,
                   (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "Failed to send packet " << i << endl;
            continue;
        }
        sent_packets++;

        // Receive pong
        socklen_t len = sizeof(server_addr);
        int received = recvfrom(sock, buffer, BUFSIZE, 0,
                              (struct sockaddr*)&server_addr, &len);

        if (received < 0) {
            if (errno == EWOULDBLOCK) {
                cout << "Packet " << i << ": No response - timeout" << endl;
            } else {
                cerr << "Error receiving packet " << i << endl;
            }
            continue;
        }

        gettimeofday(&end, NULL);
        received_packets++;

        // Calculate RTT in milliseconds
        double rtt = (end.tv_sec - start.tv_sec) * 1000.0;
        rtt += (end.tv_usec - start.tv_usec) / 1000.0;

        buffer[received] = '\0';
        cout << "Reply from server: " << buffer << endl;
        cout << "RTT: " << rtt << " ms" << endl;

        sleep(1);  // Wait 1 second before sending next ping
    }

    // Print statistics
    cout << "\n--- Ping statistics ---\n";
    cout << sent_packets << " packets transmitted, "
              << received_packets << " packets received, "
              << (100.0 * (sent_packets - received_packets) / sent_packets)
              << "% packet loss" << endl;

    close(sock);
    return 0;
}