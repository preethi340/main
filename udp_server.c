#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);

    char roll[50];
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("UDP Server Waiting...\n");

    recvfrom(server_fd, roll, sizeof(roll), 0,
             (struct sockaddr *)&client_addr, &len);

    FILE *fp = fopen("students.csv", "r");

    int found = 0;
    char line[200];

    while (fgets(line, sizeof(line), fp))
    {
        char temp[200];
        strcpy(temp, line);

        char *rno = strtok(temp, ",");
        char *name = strtok(NULL, ",");

        if (strcmp(rno, roll) == 0)
        {
            sprintf(buffer,
                    "Roll Number : %s\nName : %s",
                    rno, name);

            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        strcpy(buffer, "Student Record Not Found");
    }

    sendto(server_fd, buffer, strlen(buffer), 0,
           (struct sockaddr *)&client_addr, len);

    close(server_fd);

    return 0;
}
