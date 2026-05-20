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
int otherClient[8192];
pthread_t arrThreadId[8192];

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

    // Queue to store client
    int queueClient[8192];
    memset(queueClient, 0, sizeof(queueClient));
    memset(otherClient, 0, sizeof(otherClient));
    memset(arrThreadId, 0, sizeof(arrThreadId));
    int numClient = 0;

    // Accepting client
    while (1)
    {
        pthread_t thread_id1, thread_id2;
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            perror("Accept() failed");
            continue;
        }

        printf("Client %d has connected to server\n", client);
        queueClient[numClient] = client;
        numClient++;

        int client1, client2;

        while (numClient >= 2)
        {
            numClient--;
            client1 = queueClient[numClient];
            queueClient[numClient] = 0;

            numClient--;
            client2 = queueClient[numClient];
            queueClient[numClient] = 0;

            otherClient[client1] = client2;
            otherClient[client2] = client1;

            pthread_create(&thread_id1, NULL, thread_proc, (void *)&client1);
            pthread_create(&thread_id2, NULL, thread_proc, (void *)&client2);

            arrThreadId[client1] = thread_id1;
            arrThreadId[client2] = thread_id2;

            pthread_detach(thread_id1);
            pthread_detach(thread_id2);
        }
    }

    // close socket
    close(listener);
    return 0;
}

void *thread_proc(void *param)
{
    int client = *(int *)param;
    int other = otherClient[client];

    char buf[2048];

    while (1)
    {
        int retRecv = recv(client, buf, sizeof(buf), 0);

        if (retRecv <= 0 || strncmp(buf, "exit", 4) == 0)
        {
            printf("Client %d disconnected from server\n", client);
            otherClient[client] = 0;
            close(client);

            printf("Client %d disconnected from server\n", other);
            otherClient[other] = 0;
            close(other);
            pthread_cancel(arrThreadId[other]);
            break;
        }

        buf[retRecv] = '\0';

        printf("Client %d has sent a message to client %d: %s\n", client, other, buf);

        send(other, buf, strlen(buf), 0);
    }
}