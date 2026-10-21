#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main() {
    int sock;
    char buffer[100];
    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Set timeout = 5 seconds
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter message: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    // Send message
    sendto(sock, buffer, strlen(buffer) + 1, 0,
           (struct sockaddr*)&server, len);

    // Wait for response
    int n = recvfrom(sock, buffer, sizeof(buffer), 0,
                     (struct sockaddr*)&server, &len);

    if (n < 0) {
        printf("Timeout! Server did not respond.\n");
    } else {
        printf("Server: %s\n", buffer);
    }

    close(sock);

    return 0;
}