#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

struct DNS {
    char domain[50];
    char ip[50];
};

int main() {
    int sock;
    char domain[50];
    char ip[50] = "Domain not found";

    struct DNS dns[] = {
        {"google.com", "142.250.72.14"},
        {"example.com", "93.184.216.34"},
        {"youtube.com", "142.250.192.14"}
    };

    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(sock, (struct sockaddr*)&server, sizeof(server));

    printf("DNS Server waiting...\n");

    recvfrom(sock, domain, sizeof(domain), 0,
             (struct sockaddr*)&client, &len);

    printf("Domain requested: %s\n", domain);

    for (int i = 0; i < 3; i++) {
        if (strcmp(domain, dns[i].domain) == 0) {
            strcpy(ip, dns[i].ip);
            break;
        }
    }

    sendto(sock, ip, strlen(ip) + 1, 0,
           (struct sockaddr*)&client, len);

    close(sock);

    return 0;
}