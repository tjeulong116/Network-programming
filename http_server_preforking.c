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

void signalHandler(int signo)
{
    int status;
    int pid = wait(&status);
}

int main()
{
    // create listener socket
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
    serverAddr.sin_port = htons(9100);

    // binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("bind() failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(listener, 100) == -1)
    {
        perror("Listen failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    int numProcesses = 8;
    char buf[2048];

    signal(SIGCHLD, signalHandler);

    for (int i = 0; i < numProcesses; i++)
    {
        if (fork() == 0)
        {
            // Child process
            while (1)
            {
                int client = accept(listener, NULL, NULL);
                printf("New client %d accepted in child process %d\n", client, getpid());

                int retRecv = recv(client, buf, sizeof(buf), 0);

                if (retRecv <= 0)
                {
                    continue;
                }

                buf[retRecv] = '\0';
                printf("%s\n", buf);

                char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
                send(client, msg, strlen(msg), 0);

                close(client);
            }
        }
    }

    // Free resources
    while (wait(NULL) != -1)
    {
        sleep(10);
    }

    close(listener);

    return 0;
}