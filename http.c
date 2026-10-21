
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

int main()
{
    char url[200], host[100], path[100];
    char request[500], buffer[4096];

    printf("Enter URL (http://example.com): ");
    scanf("%199s", url);

    if (strncmp(url, "http://", 7) == 0)
        memmove(url, url + 7, strlen(url) - 6);

    char *slash = strchr(url, '/');

    if (slash)
    {
        strcpy(path, slash);
        *slash = '\0';
    }
    else
        strcpy(path, "/");

    strcpy(host, url);

    struct hostent *server = gethostbyname(host);

    if (server == NULL)
    {
        printf("Host not found\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);

    memcpy(&addr.sin_addr, server->h_addr, server->h_length);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    sprintf(request,
            "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n",
            path, host);

    send(sock, request, strlen(request), 0);

    FILE *fp = fopen("downloaded_file", "wb");

    int n;
    int header = 0;

    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0)
    {
        if (!header)
        {
            char *body = strstr(buffer, "\r\n\r\n");

            if (body)
            {
                body += 4;
                fwrite(body, 1, n - (body - buffer), fp);
                header = 1;
            }
        }
        else
            fwrite(buffer, 1, n, fp);
    }

    fclose(fp);
    close(sock);

    printf("File downloaded successfully!\n");

    return 0;
}
