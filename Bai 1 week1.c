#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    // Check if user input valid arguments
    if (strcmp(argv[1], "tcp_client") != 0 || argc != 4)
    {
        printf("Wrong format\n");
        printf("%s", argv[1]);
        exit(1);
    }

    if (inet_addr(argv[2]) == -1)
    {
        perror("Convert IP failed");
        exit(1);
    }

    // Create socket for client
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1)
    {
        perror("Create socket failed");
        exit(1);
    }

    // Initialize server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[2]);
    serverAddr.sin_port = htons(atoi(argv[3]));

    // Connect to server
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connect failed");
        exit(1);
    }

    // Send message to server part
    char msg[2048];

    while (1)
    {
        printf("Enter message: ");
        fgets(msg, sizeof(msg), stdin);

        int ret = send(client, msg, strlen(msg), 0);
        if (strncmp(msg, "end", 3) == 0 || ret <= 0)
        {
            break;
        }
    }

    // close socket
    close(client);

    return 0;
}