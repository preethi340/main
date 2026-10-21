#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 8080
#define SIZE 100

int main()
{
    int sock;
    struct sockaddr_in addr;

    char *messages[] = {
        "Hello",
        "How are you?",
        "I'm good",
        "Thank you"
    };

    char packet[SIZE];
    char ack[SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    inet_pton(AF_INET,
              "127.0.0.1",
              &addr.sin_addr);

    connect(sock,
            (struct sockaddr *)&addr,
            sizeof(addr));

    // 5 second timeout
    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(sock,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &timeout,
               sizeof(timeout));

    for (int i = 1; i <= 4; i++)
    {
        // Add sequence number to message
        sprintf(packet,
                "Packet %d: %s",
                i,
                messages[i - 1]);

        printf("Sending: %s\n", packet);

        write(sock,
              packet,
              strlen(packet));

        // Wait for ACK
        int n = read(sock, ack, SIZE - 1);

        if (n < 0)
        {
            printf("Timeout for Packet %d!\n", i);

            printf("Resending Packet %d...\n", i);

            write(sock,
                  packet,
                  strlen(packet));

            n = read(sock, ack, SIZE - 1);

            if (n > 0)
            {
                ack[n] = '\0';

                printf("Received: %s\n", ack);
            }
        }
        else
        {
            ack[n] = '\0';

            printf("Received: %s\n", ack);
        }
    }

    close(sock);

    return 0;
}