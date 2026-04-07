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

int removeClient(int *clients, int *nClients, int i)
{
    if (i < *nClients - 1)
    {
        clients[i] = clients[*nClients - 1];
    }

    *nClients -= 1;
}

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

    fd_set fdread;
    int max_dp = 0;

    int clients[1020];
    int numClient = 0;
    int state[1020];
    memset(state, 0, sizeof(state));

    struct timeval tv;
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
        // Reset fd_set
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        max_dp = listener + 1;
        for (int i = 0; i < numClient; i++)
        {
            FD_SET(clients[i], &fdread);

            if (max_dp < clients[i] + 1)
            {
                max_dp = clients[i] + 1;
            }
        }

        // Reset timeval struct
        tv.tv_sec = 120;
        tv.tv_usec = 0;

        // Select
        int retSelect = select(max_dp, &fdread, NULL, NULL, &tv);

        if (retSelect < 0)
        {
            perror("Select() failed\n");
            break;
        }

        if (retSelect == 0)
        {
            printf("Time out\n");
            continue;
        }

        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            if (client == -1)
            {
                perror("Accept failed\n");
            }
            else
            {
                if (numClient < 1020)
                {
                    printf("Client %d has now connected\n", client);
                    clients[numClient] = client;
                    numClient++;
                    send(client, msgInquiry, strlen(msgInquiry), 0);
                }
                else
                {
                    send(client, msgOutSlot, strlen(msgOutSlot), 0);
                    close(client);
                }
            }
        }

        for (int i = 0; i < numClient; i++)
        {
            if (FD_ISSET(clients[i], &fdread))
            {
                int retRecv = recv(clients[i], buf, sizeof(buf), 0);

                // Client disconnects
                if (retRecv <= 0)
                {
                    printf("Client %d has now disconnected\n", clients[i]);
                    removeClient(clients, &numClient, i);
                    state[i] = 0;
                    i--;
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
                        send(clients[i], msgWrongFormat, strlen(msgWrongFormat), 0);
                        continue;
                    }
                    strcpy(tempUserName, token);

                    token = strtok(NULL, " ");
                    if (token == NULL)
                    {
                        send(clients[i], msgWrongFormat, strlen(msgWrongFormat), 0);
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
                        send(clients[i], msgWrongFormat, strlen(msgWrongFormat), 0);
                        printf("%d\n", cnt);
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
                                send(clients[i], msgLoginSuccessfully, strlen(msgLoginSuccessfully), 0);
                                break;
                            }
                        }
                    }

                    if (state[i] == 0)
                    {
                        send(clients[i], msgWrongUsernameOrPassword, strlen(msgWrongUsernameOrPassword), 0);
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
                        send(clients[i], fileBuf, strlen(fileBuf), 0);
                    }
                }
            }
        }
    }

    // close listener socket
    close(listener);
    return 0;
}