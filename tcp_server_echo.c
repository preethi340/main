#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    char buffer[1024];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    listen(server_fd, 5);

    printf("Server waiting...\n");

    client_fd = accept(server_fd, (struct sockaddr*)&client, &len);

    recv(client_fd, buffer, sizeof(buffer), 0);
    printf("Client: %s\n", buffer);

    send(client_fd, buffer, strlen(buffer) + 1, 0);

    close(client_fd);
    close(server_fd);

    return 0;
}