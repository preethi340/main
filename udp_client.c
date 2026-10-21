#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(server_addr);

    char roll[50];
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    printf("Enter Roll Number: ");
    scanf("%s", roll);

    sendto(sock, roll, strlen(roll), 0,
           (struct sockaddr *)&server_addr, len);

    recvfrom(sock, buffer, sizeof(buffer), 0,
             NULL, NULL);

    printf("\n%s\n", buffer);

    close(sock);

    return 0;
}
