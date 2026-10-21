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
    char roll[50];
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Server details
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Localhost IP Address
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Connect to server
    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Get roll number from user
    printf("Enter Roll Number: ");
    scanf("%s", roll);

    // Send roll number to server
    write(sock, roll, strlen(roll));

    // Receive response from server
    read(sock, buffer, sizeof(buffer));

    // Display result
    printf("\n%s\n", buffer);

    // Close socket
    close(sock);

    return 0;
}
