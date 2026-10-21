#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock;
    char domain[50];
    char ip[50];

    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter domain: ");
    fgets(domain, sizeof(domain), stdin);

    domain[strcspn(domain, "\n")] = '\0';

    sendto(sock, domain, strlen(domain) + 1, 0,
           (struct sockaddr*)&server, len);

    recvfrom(sock, ip, sizeof(ip), 0,
             (struct sockaddr*)&server, &len);

    printf("IP Address: %s\n", ip);

    close(sock);

    return 0;
}