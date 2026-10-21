#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

char gen[] = "10011";                       // generator G(x)

// XOR division: leaves the remainder in the last (glen-1) bits of t
void divide(char *t, int len, char *g, int glen)
{
  for (int i = 0; i <= len - glen; i++)
    if (t[i] == '1')
      for (int j = 0; j < glen; j++)
        t[i + j] = (t[i + j] == g[j]) ? '0' : '1';
}

int main()
{
  int glen = strlen(gen);

  int server = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(8080);
  bind(server, (struct sockaddr *)&addr, sizeof(addr));
  listen(server, 1);
  int client = accept(server, NULL, NULL);

  char frame[1000] = {0};
  int len = recv(client, frame, sizeof(frame) - 1, 0);
  frame[len] = 0;
  printf("Received frame : %s\n", frame);

  char t[1000];
  strcpy(t, frame);
  divide(t, len, gen, glen);                // divide the whole frame

  int error = 0;
  for (int i = len - (glen - 1); i < len; i++)
    if (t[i] == '1') error = 1;             // remainder not all zeros = error

  if (error) printf("Result: ERROR detected\n");
  else       printf("Result: No error, frame is correct\n");

  close(client);
  close(server);
  return 0;
}