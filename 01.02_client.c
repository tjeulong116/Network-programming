#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

int main()
{
    // Create socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1)
    {
        perror("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(9100);

    // Connect to server
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    // Send data
    char msg[2048];
    while (1)
    {
        fgets(msg, sizeof(msg), stdin);
        if (strncmp(msg, "end", 3) == 0)
        {
            break;
        }

        msg[strlen(msg) - 1] = '\0';

        if (send(client, msg, strlen(msg), 0) <= 0)
        {
            printf("Send data failed\n");
            break;
        }
    }

    // close socket
    close(client);
    return 0;
}