#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define SIZE 100

int main()
{
    int server, client;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    char msg[SIZE];
    char ack[SIZE];

    server = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server, (struct sockaddr *)&addr, sizeof(addr));

    listen(server, 1);

    printf("Server waiting...\n");

    client = accept(server,
                    (struct sockaddr *)&addr,
                    &len);

    for (int i = 1; i <= 4; i++)
    {
        int n = read(client, msg, SIZE - 1);

        if (n > 0)
        {
            msg[n] = '\0';

            printf("Received Packet %d: %s\n", i, msg);

            if (i == 3)
            {
                printf("Packet %d lost! No ACK sent.\n", i);

                // Wait for retransmission
                n = read(client, msg, SIZE - 1);

                if (n > 0)
                {
                    msg[n] = '\0';

                    printf("Received Packet %d again: %s\n",
                           i, msg);

                    sprintf(ack, "ACK %d", i);

                    write(client, ack, strlen(ack));
                }
            }
            else
            {
                sprintf(ack, "ACK %d", i);

                write(client, ack, strlen(ack));
            }
        }
    }

    close(client);
    close(server);

    return 0;
}