#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
using namespace std;

#define PORT 1234
#define INTERVAL 2
#define MAX 20
#define BUFSIZE 512

int main(){
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        cerr << "Failed to create socker" << endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0){
        cerr << "Error setting timeout" << endl;
        return 1;
    }
    
    char buffer[BUFSIZ];
    int sent = 0, receive = 0;

    for(int i=0; i<MAX; i++){
        string resp = "PING_" + to_string(i) + " ";
        struct timeval start, end;

        gettimeofday(&start, NULL);

        if(sendto(sock, resp.c_str(), resp.length(), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
            cerr << "Error sending packet" << i << endl;
            continue;
        }

        sent++;

        socklen_t len = sizeof(server_addr);
        int received = recvfrom(sock, buffer, BUFSIZ, 0, (struct sockaddr*)&server_addr, &len);

        if(received < 0){
            if (errno = EWOULDBLOCK) {
                cout << "Packet " << i << ": timeout" << endl;
            } else {
                cerr << "Error receiving packet " << i << endl;
            }
            continue;
        }

        gettimeofday(&end, NULL);
        receive++;

        double rtt = (end.tv_sec - start.tv_sec) * 1000.0;
        rtt += (end.tv_usec - start.tv_usec) / 1000.0;

        buffer[received] = '\0';
        cout << "Reply from server: " << buffer << endl;
        cout << "RTT: " << rtt << " ms" << endl;
    }

    cout << "\n--- Ping statistics ---\n";
    cout << sent << " packets transmitted, "
              << receive << " packets received, "
              << (100.0 * (sent - receive) / sent)
              << "% packet loss" << endl;

    close(sock);
    return 0;
}