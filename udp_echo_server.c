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
    int listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (listener == -1)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(9500);

    // Declare client address
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed\n");
        exit(EXIT_FAILURE);
    }

    // Receive data from client and send back
    char buf[2048];

    while (1)
    {
        int retRecv = recvfrom(listener, buf, sizeof(buf), 0, (struct sockaddr *)&clientAddr, &clientAddrSize);
        if (retRecv <= 0)
        {
            break;
        }
        buf[retRecv] = '\0';

        printf("Data received: %s\n", buf);

        int retSend = sendto(listener, buf, strlen(buf), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
        if (retSend <= 0)
        {
            break;
        }
    }

    // close socket
    close(listener);
    return 0;
}