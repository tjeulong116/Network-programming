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
    // Create server socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8000);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(listener, 5) == -1)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Accepting
    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    char buf[2056];
    // recv dir name
    int retName = recv(client, buf, sizeof(buf), 0);
    if (retName <= 0)
    {
        printf("Receive directory name failed\n");
        exit(EXIT_FAILURE);
    }

    buf[retName] = '\0';
    printf("%s\n", buf);

    // recv numFiles
    long numFiles = 0;
    int retCnt = recv(client, &numFiles, sizeof(long), 0);
    if (retCnt <= 0)
    {
        printf("Receive number of files failed\n");
        exit(EXIT_FAILURE);
    }

    // recv file info

    for (int i = 0; i < numFiles; i++)
    {
        long nameSize = 0;
        int ret0 = recv(client, &nameSize, sizeof(long), 0);
        if (ret0 <= 0)
        {
            printf("Receive nameSize failed\n");
            break;
        }

        int ret1 = recv(client, buf, nameSize, 0);
        if (ret1 <= 0)
        {
            printf("Receive nameFile failed\n");
            break;
        }
        buf[ret1] = '\0';
        printf("%s - ", buf);

        long sz = 0;
        int ret2 = recv(client, &sz, sizeof(long), 0);
        if (ret2 <= 0)
        {
            printf("Receive fileSize failed\n");
            break;
        }
        printf("%ld bytes\n", sz);
    }

    // close sockets
    close(client);
    close(listener);
    return 0;
}