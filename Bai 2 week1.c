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
    if (argc != 5 || strcmp(argv[1], "tcp_server") != 0)
    {
        printf("Wrong format\n");
        exit(1);
    }

    // Create socket for server
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Create socket failed");
        exit(1);
    }

    // Initialize server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(atoi(argv[2]));

    // Bind socket to server address
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed");
        exit(1);
    }

    // Set socket to listening state
    if (listen(listener, 5) == -1)
    {
        perror("Listen failed");
    }

    // Accept a client
    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        perror("Accept failed");
    }

    // Send hello string from a file to client
    char msg[2048];
    FILE *fp = fopen(argv[3], "rb");

    while (1)
    {
        int len = fread(msg, 1, sizeof(msg), fp);

        int ret = send(client, msg, len, 0);
        if (ret <= 0 || len <= 0)
        {
            break;
        }
    }

    // Write data received from client to a file
    char buf[2048];
    FILE *fp2 = fopen(argv[4], "wb");

    while (1)
    {
        int retReceive = recv(client, buf, sizeof(buf), 0);
        if (retReceive <= 0)
        {
            break;
        }

        fwrite(buf, 1, retReceive, fp2);
        buf[retReceive] = '\0';
        printf("Received string: %s", buf);
    }

    // close socket
    close(client);

    return 0;
}