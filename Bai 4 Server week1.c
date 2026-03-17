#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct sinhvien
{
  char dateTime[50];
  char mssv[50];
  char hoTen[50];
  char dob[50];
  double gpa;
};

int main(int argc, char *argv[])
{
  // Create server socket
  int listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (listener == -1)
  {
    perror("Create socket failed");
  }

  // Initialize server address
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddr.sin_port = htons(atoi(argv[1]));

  // Bind server address to socket
  if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
  {
    perror("Bind failed");
  }

  FILE *fp = fopen(argv[2], "wb");

  // Receive struct message from client
  while (1)
  {
    struct sinhvien sv;
    struct sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    char buf[512];

    int ret = recvfrom(listener, &sv, sizeof(sv), 0, (struct sockaddr *)&clientAddr, &clientAddrSize);
    if (strncmp(sv.mssv, "exit", 4) == 0 || ret <= 0)
    {
      break;
    }

    snprintf(buf, sizeof(buf), "%s %s %s %s %s %.2lf %d", inet_ntoa(clientAddr.sin_addr), sv.dateTime, sv.mssv, sv.hoTen, sv.dob, sv.gpa, '\n');
    printf("Received data line: %s\n", buf);
    fwrite(buf, 1, strlen(buf), fp);
  }

  // close socket
  close(listener);

  return 0;
}