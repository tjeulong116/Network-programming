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

    pthread_t thread_id;
    int numThreads = 8;
    // Create threads
    for (int i = 0; i < numThreads; i++)
    {
        int ret = pthread_create(&thread_id, NULL, thread_proc, (void *)&listener);

        if (ret != 0)
        {
            printf("Could not create new thread.\n");
            sched_yield();
        }

        pthread_join(thread_id, NULL);
    }

    // Close socket
    close(listener);
    return 0;
}

void *thread_proc(void *param)
{
    int listener = *(int *)param;
    char buf[2048];

    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            perror("Accept() failed");
            continue;
        }

        printf("New client %d accepted in thread %ld with pid %d\n", client, pthread_self(), getpid());

        int retRecv = recv(client, buf, sizeof(buf), 0);
        buf[retRecv] = '\0';
        printf("%s", buf);

        // Send response
        char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
        send(client, msg, strlen(msg), 0);

        close(client);
    }
}
