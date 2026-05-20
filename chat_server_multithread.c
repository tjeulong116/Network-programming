#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>
#include <sys/select.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <pthread.h>

void *thread_proc(void *param);

// Array storing sockets
int clientArr[2048];
int numClient = 0;

int main()
{
    // Creating listener socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Create listener socket failed");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(9000);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind() failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(listener, 10) == -1)
    {
        perror("Listen() failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    memset(clientArr, 0, sizeof(clientArr));

    // Accepting client
    while (1)
    {
        pthread_t thread_id;
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            perror("Accept() failed");
            continue;
        }

        clientArr[numClient] = client;
        numClient++;
        printf("Client %d has connected to server\n", client);

        pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
        pthread_detach(thread_id);
    }

    // close socket
    close(listener);
    return 0;
}

void *thread_proc(void *param)
{
    int client = *(int *)param;
    int signInState = 0;
    char *idMsgQuery = "Please send your id in this format: client_id: client_name\n";
    char *msgWrongFormat = "Incorrect format. Please send again (client_id: client_name)\n";
    char *signInSuccessfully = "You have signed in successfully\n";

    send(client, idMsgQuery, strlen(idMsgQuery), 0);

    char buf[2048];
    char s1[1000], s2[1000], s3[1000];
    char id[1000];
    strcpy(s1, "");
    strcpy(s2, "");
    strcpy(s3, "");
    strcpy(id, "");

    while (1)
    {
        int retRecv = recv(client, buf, sizeof(buf), 0);

        if (retRecv <= 0)
        {
            printf("Client %d has disconnected from server\n", client);
            break;
        }

        buf[retRecv] = '\0';

        if (strncmp(buf, "exit", 4) == 0)
        {
            printf("Client %d has disconnected from server\n", client);
            break;
        }

        if (signInState == 0)
        {
            int len = sscanf(buf, "%s %s %s", s1, s2, s3);

            if (len != 2)
            {
                send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
                continue;
            }

            if (strcmp(s1, "client_id:") != 0)
            {
                send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
                continue;
            }

            strcpy(id, s2);
            signInState = 1;
            send(client, signInSuccessfully, strlen(signInSuccessfully), 0);
        }
        else if (signInState == 1)
        {
            printf("Client %d send a broadcast msg to everyone: %s\n", client, buf);

            for (int i = 0; i < numClient; i++)
            {
                if (clientArr[i] != client)
                {
                    send(clientArr[i], id, strlen(id), 0);
                    send(clientArr[i], ": ", 2, 0);
                    send(clientArr[i], buf, strlen(buf), 0);
                }
            }
        }
    }

    signInState = 0;

    for (int i = 0; i < numClient; i++)
    {
        if (clientArr[i] == client)
        {
            clientArr[i] = clientArr[numClient - 1];
            break;
        }
    }
    numClient--;

    close(client);
}