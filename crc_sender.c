#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

char gen[] = "10011";

void divide(char *t, int len, char *g, int glen)
{
  for (int i = 0; i <= len - glen; i++)
    if (t[i] == '1')
      for (int j = 0; j < glen; j++)
        t[i + j] = (t[i + j] == g[j]) ? '0' : '1';
}

int main()
{
  char text[100], msg[1000], t[1000], frame[1000];
  int n = 0, glen = strlen(gen);

  printf("Enter data: ");
  fgets(text, sizeof(text), stdin);
  text[strcspn(text, "\n")] = 0;

  // 1. data -> binary (8 bits per character)
  for (int i = 0; text[i]; i++)
    for (int b = 7; b >= 0; b--)
      msg[n++] = ((text[i] >> b) & 1) ? '1' : '0';
  msg[n] = 0;
  printf("Binary data : %s\n", msg);

  // 2. append (glen-1) zeros, divide, remainder = CRC
  strcpy(t, msg);
  for (int i = 0; i < glen - 1; i++) t[n + i] = '0';
  t[n + glen - 1] = 0;
  divide(t, n + glen - 1, gen, glen);
  char *crc = t + n;
  printf("CRC         : %s\n", crc);

  // 3. frame = data + CRC
  strcpy(frame, msg);
  strcat(frame, crc);
  printf("Frame       : %s\n", frame);

  // 4. manual error: flip one bit
  int pos;
  printf("Bit position to flip (0 = no error): ");
  scanf("%d", &pos);
  if (pos > 0 && pos <= (int)strlen(frame))
    frame[pos - 1] = (frame[pos - 1] == '0') ? '1' : '0';

  // 5. send over TCP
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
  connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  send(sock, frame, strlen(frame), 0);
  close(sock);
  return 0;
}