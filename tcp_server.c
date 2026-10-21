#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char roll[50];
    char buffer[BUFFER_SIZE];

    memset(roll, 0, sizeof(roll));
    memset(buffer, 0, sizeof(buffer));

    // Receive roll number from client
    read(client_fd, roll, sizeof(roll));

    FILE *fp = fopen("students.csv", "r");

    if (fp == NULL)
    {
        strcpy(buffer, "Unable to open students.csv");
        write(client_fd, buffer, strlen(buffer));
        close(client_fd);
        pthread_exit(NULL);
    }

    char line[200];
    int found = 0;

    while (fgets(line, sizeof(line), fp))
    {
        char temp[200];
        strcpy(temp, line);

        char *rno = strtok(temp, ",");
        char *name = strtok(NULL, ",");
        char *dept = strtok(NULL, ",");
        char *sem = strtok(NULL, ",");
        char *cgpa = strtok(NULL, ",");

        if (strcmp(rno, roll) == 0)
        {
            sprintf(buffer,
                    "Student Details\n\n"
                    "Roll Number : %s\n"
                    "Name        : %s\n"
                    "Department  : %s\n"
                    "Semester    : %s\n"
                    "CGPA        : %s",
                    rno, name, dept, sem, cgpa);

            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found)
    {
        strcpy(buffer, "Student Record Not Found");
    }

    write(client_fd, buffer, strlen(buffer));

    close(client_fd);

    pthread_exit(NULL);
}

int main()
{
    int server_fd;
    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_fd, 5);

    printf("=================================\n");
    printf("TCP Server Started\n");
    printf("Listening on Port %d\n", PORT);
    printf("=================================\n");

    while (1)
    {
        int *client_fd = malloc(sizeof(int));

        *client_fd = accept(server_fd, NULL, NULL);

        printf("Client Connected\n");

        pthread_t tid;

        pthread_create(&tid, NULL, handle_client, client_fd);

        pthread_detach(tid);
    }

    close(server_fd);

    return 0;
}

