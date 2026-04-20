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
    serverAddr.sin_port = htons(9500);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed");
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

    struct pollfd fds[1021];
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    int nfds = 1;

    int clients[1020];
    int state[1020];
    memset(state, 0, sizeof(state));

    char buf[2048];
    char fileBuf[4096];

    FILE *fp1, *fp2;
    fp1 = fopen("database.txt", "r");

    char *msgInquiry = "Enter username, password: ";
    char *msgWrongFormat = "Wrong format for sign in. Try again: ";
    char *msgLoginSuccessfully = "Log in successfully\n";
    char *msgWrongUsernameOrPassword = "Wrong username or password. Try again: ";
    char *msgOutSlot = "Sorry, server is out of slots right now\n";

    while (1)
    {
        // poll
        int retPoll = poll(fds, nfds, 30000);

        if (retPoll < 0)
        {
            perror("poll() failed\n");
            break;
        }

        if (retPoll == 0)
        {
            printf("Time out\n");
            continue;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(listener, NULL, NULL);
            if (client == -1)
            {
                perror("Accept failed\n");
            }
            else
            {
                if (nfds < 1021)
                {
                    printf("Client %d has now connected\n", nfds);
                    fds[nfds].fd = client;
                    fds[nfds].events = POLLIN;
                    nfds++;
                    send(client, msgInquiry, strlen(msgInquiry), 0);
                }
                else
                {
                    send(client, msgOutSlot, strlen(msgOutSlot), 0);
                    close(client);
                }
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                int retRecv = recv(fds[i].fd, buf, sizeof(buf), 0);

                // Client disconnects
                if (retRecv <= 0)
                {
                    printf("Client %d has now disconnected\n", i);

                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--;
                    state[i] = state[nfds - 1];
                    continue;
                }

                buf[retRecv - 1] = '\0';

                if (state[i] == 0)
                { // Haven't sign in
                    char tempUserName[100];
                    char tempUserPassword[100];
                    int cnt = 1;
                    char *token;

                    token = strtok(buf, " ");
                    if (token == NULL)
                    {
                        send(fds[i].fd, msgWrongFormat, strlen(msgWrongFormat), 0);
                        continue;
                    }
                    strcpy(tempUserName, token);

                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        send(fds[i].fd, msgWrongFormat, strlen(msgWrongFormat), 0);
                        continue;
                    }
                    strcpy(tempUserPassword, token);

                    while (token != NULL)
                    {
                        token = strtok(NULL, " ");
                        cnt++;
                    }

                    // Check format
                    if (cnt != 2)
                    {
                        send(fds[i].fd, msgWrongFormat, strlen(msgWrongFormat), 0);
                        // printf("%d\n", cnt);
                        continue;
                    }

                    // Check if account exists in database
                    rewind(fp1);
                    while (fgets(fileBuf, sizeof(fileBuf), fp1) != NULL)
                    {
                        token = strtok(fileBuf, " ");

                        if (strcmp(token, tempUserName) == 0)
                        {
                            token = strtok(NULL, " ");
                            token[strlen(token) - 2] = '\0';

                            if (strcmp(token, tempUserPassword) == 0)
                            {
                                state[i] = 1;
                                send(fds[i].fd, msgLoginSuccessfully, strlen(msgLoginSuccessfully), 0);
                                break;
                            }
                        }
                    }

                    if (state[i] == 0)
                    {
                        send(fds[i].fd, msgWrongUsernameOrPassword, strlen(msgWrongUsernameOrPassword), 0);
                    }
                }
                else if (state[i] == 1)
                {
                    // Recv control data
                    strcat(buf, " > out.txt");
                    system(buf);

                    // Send data from file to client
                    fp2 = fopen("out.txt", "r");

                    while (fgets(fileBuf, sizeof(fileBuf), fp2) != NULL)
                    {
                        send(fds[i].fd, fileBuf, strlen(fileBuf), 0);
                    }
                }
            }
        }
    }

    // close listener socket
    close(listener);
    return 0;
}