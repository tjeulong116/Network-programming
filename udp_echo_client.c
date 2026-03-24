#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    // Create socket
    int client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == -1)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(9500);

    // Communicate with server
    char msg[2048];
    char buf[2048];

    // Client send and received echo data
    while (1)
    {
        printf("Please input message: ");
        fgets(msg, sizeof(msg), stdin);
        msg[strlen(msg) - 1] = '\0';
        if (strncmp(msg, "end", 3) == 0)
        {
            break;
        }

        int retSend = sendto(client, msg, strlen(msg), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        if (retSend <= 0)
        {
            printf("Sendto failed\n");
            break;
        }

        int retRecv = recvfrom(client, buf, sizeof(buf), 0, NULL, NULL);
        if (retRecv <= 0)
        {
            printf("Recvfrom failed\n");
            break;
        }
        buf[retRecv] = '\0';
        printf("Echo message received from server: %s\n", buf);
    }

    // close socket
    close(client);

    return 0;
}