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

struct User
{
    char userId[50];
    char username[200];
};

int isValid(char *str)
{
    // check strlen
    if (strlen(str) == 0)
    {
        return 0;
    }

    // check characters
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == ' ' || str[i] == ':')
        {
            return 0;
        }
    }

    return 1;
}

int isAlreadyExists(char *id, struct User *arr)
{
    for (int i = 0; i < 1024; i++)
    {
        if (strcmp(id, arr[i].userId) == 0)
        {
            return 1;
        }
    }

    return 0;
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
    serverAddr.sin_port = htons(9200);

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

    //
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);
    FD_SET(listener, &fdread);

    struct timeval tv;
    struct User clientArr[1024];
    char buf[2048];
    char broadcast[2200];
    char *msgNameInquiry = "Please enter id and username in this format (client_id: client_name) : ";
    char *msgWrongFormat = "Wrong id and username format. Please enter again (client_id: client_name) : ";
    char *msgIdAlreadyExists = "There is already a similar id. Please try another id : ";
    char *msgNameSuccessful = "You have set your id and username successfully\n";
    char *msgOutSlot = "Sorry, server is out of slots right now\n";

    // Initialize value for client name array
    for (int i = 0; i < 1024; i++)
    {
        strcpy(clientArr[i].userId, "");
        strcpy(clientArr[i].username, "");
    }

    while (1)
    {
        fdtest = fdread;

        // Reset time struct
        tv.tv_sec = 120;
        tv.tv_usec = 0;

        int retSelect = select(FD_SETSIZE, &fdtest, NULL, NULL, &tv);
        if (retSelect < 0)
        {
            perror("Select() failed");
            break;
        }

        if (retSelect == 0)
        {
            printf("Time out\n");
            continue;
        }

        // Select() return value > 0
        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    int client = accept(listener, NULL, NULL);
                    if (client == -1)
                    {
                        perror("Accept failed\n");
                        continue;
                    }

                    if (client < FD_SETSIZE)
                    {
                        FD_SET(client, &fdread);
                        printf("Client %d has now connected\n", client);

                        send(client, msgNameInquiry, strlen(msgNameInquiry), 0);
                    }
                    else
                    {
                        send(client, msgOutSlot, strlen(msgOutSlot), 0);
                        close(client);
                    }
                }
                else
                {
                    // Communicate with clients
                    int retRecv = recv(i, buf, sizeof(buf), 0);

                    if (retRecv <= 0)
                    {
                        printf("Client %d has now disconnected\n", i);
                        strcpy(clientArr[i].userId, "");
                        strcpy(clientArr[i].username, "");
                        FD_CLR(i, &fdread);
                        close(i);
                        continue;
                    }

                    buf[retRecv - 1] = '\0';

                    // Haven't provide userId and username
                    if (strcmp(clientArr[i].userId, "") == 0)
                    {
                        char tempUserId[50] = "";
                        char tempUsername[200] = "";
                        char *ptr = strchr(buf, ':');
                        int idx = 0;
                        if (ptr)
                        {
                            idx = (int)(ptr - buf);
                            strncpy(tempUserId, buf, idx);
                            tempUserId[idx] = '\0';

                            if (buf[idx + 1] != ' ')
                            {
                                send(i, msgWrongFormat, strlen(msgWrongFormat), 0);
                                continue;
                            }

                            strcpy(tempUsername, buf + idx + 2);
                        }
                        else
                        {
                            send(i, msgWrongFormat, strlen(msgWrongFormat), 0);
                            continue;
                        }

                        // check if both userId and username are valid
                        if (isValid(tempUserId) == 0 || isValid(tempUsername) == 0)
                        {
                            send(i, msgWrongFormat, strlen(msgWrongFormat), 0);
                            continue;
                        }

                        // check if userId already exists
                        if (isAlreadyExists(tempUserId, clientArr) == 1)
                        {
                            send(i, msgIdAlreadyExists, strlen(msgIdAlreadyExists), 0);
                            continue;
                        }

                        // Passed all requirements
                        strcpy(clientArr[i].userId, tempUserId);
                        strcpy(clientArr[i].username, tempUsername);
                        printf("Client %d has set their id and username successfully\n", i);
                        send(i, msgNameSuccessful, strlen(msgNameSuccessful), 0);
                    }
                    else
                    {
                        // send and receive chat data
                        printf("Client %d send a message to server: %s\n", i, buf);

                        strcpy(broadcast, "");
                        strcat(broadcast, clientArr[i].userId);
                        strcat(broadcast, ": ");
                        strcat(broadcast, buf);
                        strcat(broadcast, "\n");
                        for (int j = 0; j < FD_SETSIZE; j++)
                        {
                            if (strcmp(clientArr[j].userId, "") != 0 && j != i)
                            {
                                send(j, broadcast, strlen(broadcast), 0);
                            }
                        }
                    }
                }
            }
        }
    }

    // Close listener socket
    close(listener);
    return 0;
}