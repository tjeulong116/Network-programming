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
    // Create a socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(9100);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed\n");
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(listener, 5) == -1)
    {
        perror("Listen failed\n");
        exit(EXIT_FAILURE);
    }

    // Accepting
    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        perror("Accept failed\n");
        exit(EXIT_FAILURE);
    }

    // recv data
    char buf[2048];
    char lastMsg[4096] = "";
    int cnt = 0;

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            break;
        }
        buf[ret] = '\0';

        // counting
        strcat(lastMsg, buf);
        for (int i = 0; i <= strlen(lastMsg) - 10; i++)
        {
            if (strncmp(lastMsg + i, "0123456789", 10) == 0)
            {
                cnt++;
            }
        }
        printf("%d\n", cnt);

        memset(lastMsg, '\0', sizeof(lastMsg));
        if (strlen(buf) >= 9)
        {
            strncpy(lastMsg, buf + strlen(buf) - 9, 9);
        }
        else
        {
            strncpy(lastMsg, buf, strlen(buf));
        }
        memset(buf, '\0', sizeof(buf));
    }

    // close socket
    close(listener);
    close(client);
    return 0;
}