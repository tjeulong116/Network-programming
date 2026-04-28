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

int main()
{
    // Create listener socket
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
    serverAddr.sin_port = htons(9000);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(listener, 1000) == -1)
    {
        perror("Listen failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    // Declare variable
    struct pollfd fds[512];
    int nfds = 1;

    char topic[511][50][20];
    memset(topic, '\0', sizeof(topic));

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    char msgHello[200] = "Xin chao client\n";
    char msgWrongFormat[200] = "Wrong command\n";
    char buf[2048];

    while (1)
    {
        int retPoll = poll(fds, nfds, -1);

        if (retPoll == -1)
        {
            perror("poll() failed");
            close(listener);
            exit(EXIT_FAILURE);
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            printf("New client connected %d\n", client);
            send(client, msgHello, strlen(msgHello), 0);

            fds[nfds].fd = client;
            fds[nfds].events = POLLIN;
            nfds++;
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                int retRecv = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);

                if (strncmp(buf, "exit", 4) == 0 || retRecv <= 0)
                {
                    // Xu ly out
                    close(fds[i].fd);
                    for (int j = 0; j < 50; j++)
                    {
                        strcpy(topic[i][j], topic[nfds - 1][j]);
                    }

                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                    continue;
                }

                // Xu ly message
                buf[retRecv] = '\0';

                if (strncmp(buf, "SUB ", 4) == 0)
                {
                    char retrievedTopic[20];
                    sscanf(buf, "SUB %s", retrievedTopic);

                    for (int j = 0; j < 50; j++)
                    {
                        if (strcmp(topic[i][j], "") == 0)
                        {
                            strcpy(topic[i][j], retrievedTopic);
                            printf("Client %d subscribes topic %s\n", fds[i].fd, retrievedTopic);
                            break;
                        }
                    }
                }
                else if (strncmp(buf, "UNSUB ", 6) == 0)
                {
                    char retrievedTopic[20];
                    sscanf(buf, "UNSUB %s", retrievedTopic);

                    for (int j = 0; j < 50; j++)
                    {
                        if (strncmp(topic[i][j], retrievedTopic, strlen(retrievedTopic)) == 0)
                        {
                            strcpy(topic[i][j], "");
                            printf("Client %d unsubscribes topic %s\n", fds[i].fd, retrievedTopic);
                            break;
                        }
                    }
                }
                else if (strncmp(buf, "PUB ", 4) == 0)
                {
                    char retrievedTopic[20];
                    char retrievedMsg[2048];

                    sscanf(buf, "PUB %s %[^\n]", retrievedTopic, retrievedMsg);
                    retrievedMsg[strlen(retrievedMsg)] = '\0';
                    strcat(retrievedMsg, "\n");
                    printf("Receive a new message in topic %s from client %d: %s", retrievedTopic, fds[i].fd, retrievedMsg);

                    // Send topic msg to all subscribed clients
                    for (int j = 1; j < nfds; j++)
                    {
                        if (i != j)
                        {
                            for (int k = 0; k < 50; k++)
                            {
                                if (strncmp(topic[j][k], retrievedTopic, strlen(retrievedTopic)) == 0)
                                {
                                    send(fds[j].fd, retrievedMsg, strlen(retrievedMsg), 0);
                                    break;
                                }
                            }
                        }
                    }
                }
                else
                {
                    send(fds[i].fd, msgWrongFormat, strlen(msgWrongFormat), 0);
                }
            }
        }
    }

    // Close listener socket
    close(listener);

    return 0;
}