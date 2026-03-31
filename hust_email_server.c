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

struct SinhVien
{
    char hoTen[100];
    char mssv[20];
    char email[100];
    int state;
};

int main()
{ // create listener socket
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    // Change to non-blocking
    unsigned long ul1 = 1;
    ioctl(listener, FIONBIO, &ul1);

    // declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(9100);

    // Binding
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed\n");
        close(listener);
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(listener, 10) == -1)
    {
        perror("Listen failed\n");
        close(listener);
        exit(EXIT_FAILURE);
    }

    int clients[100];
    struct SinhVien arr[100];
    int numClient = 0;

    for (int i = 0; i < 100; i++)
    {
        strcpy(arr[i].email, "");
        arr[i].state = 0;
    }

    char hoTenRequest[] = "Please input your student name: ";
    char mssvRequest[] = "Please input your student id: ";

    while (1)
    {
        // Accepting
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                // server is waiting
            }
            else
            {
                continue;
            }
        }
        else
        {
            printf("New client accepted: %d\n", client);
            clients[numClient] = client;
            unsigned long ul2 = 1;
            ioctl(clients[numClient], FIONBIO, &ul2);
            numClient++;
        }

        // Working with all clients progressively
        for (int i = 0; i < numClient; i++)
        {
            // Request student name
            if (arr[i].state == 0)
            {
                int retSend1 = send(clients[i], hoTenRequest, strlen(hoTenRequest), 0);
                if (retSend1 == -1)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        // skip
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (retSend1 == 0)
                {
                    continue;
                }
                else
                {
                    arr[i].state = 1;
                }
            }

            // Recv student name
            if (arr[i].state == 1)
            {
                int retRecv1 = recv(clients[i], arr[i].hoTen, sizeof(arr[i].hoTen), 0);
                if (retRecv1 == -1)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        // skip
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (retRecv1 == 0)
                {
                    continue;
                }
                else
                {
                    arr[i].hoTen[retRecv1 - 1] = '\0';
                    arr[i].state = 2;
                }
            }

            // Request student id
            if (arr[i].state == 2)
            {
                int retSend2 = send(clients[i], mssvRequest, strlen(mssvRequest), 0);
                if (retSend2 == -1)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        // skip
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (retSend2 == 0)
                {
                    continue;
                }
                else
                {
                    arr[i].state = 3;
                }
            }

            // Recv student id
            if (arr[i].state == 3)
            {
                int retRecv2 = recv(clients[i], arr[i].mssv, sizeof(arr[i].mssv), 0);
                if (retRecv2 == -1)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        // skip
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (retRecv2 == 0)
                {
                    continue;
                }
                else
                {
                    arr[i].mssv[retRecv2 - 1] = '\0';
                    arr[i].state = 4;
                }
            }

            // Create student email
            if (arr[i].state == 4)
            {
                int idx = 0;
                char *token[100];

                token[0] = strtok(arr[i].hoTen, " ");

                while (token[idx] != NULL)
                {
                    idx++;
                    token[idx] = strtok(NULL, " ");
                }

                for (int j = 0; j < strlen(token[idx - 1]); j++)
                {
                    token[idx - 1][j] = tolower(token[idx - 1][j]);
                }
                strcat(arr[i].email, token[idx - 1]);

                for (int j = 0; j < idx - 1; j++)
                {
                    char temp[2] = {(char)tolower(token[j][0]), '\0'};
                    strcat(arr[i].email, temp);
                }

                strcat(arr[i].email, arr[i].mssv + 2);
                strcat(arr[i].email, "@sis.hust.edu.vn");

                arr[i].state = 5;
            }

            // Send email
            if (arr[i].state == 5)
            {
                int retSend3 = send(clients[i], arr[i].email, strlen(arr[i].email), 0);
                if (retSend3 == -1)
                {
                    if (errno == EWOULDBLOCK)
                    {
                        // skip
                    }
                    else
                    {
                        continue;
                    }
                }
                else if (retSend3 == 0)
                {
                    continue;
                }
                else
                {
                    arr[i].state = 6;
                    printf("Completed a session with client %d\n", clients[i]);
                    close(clients[i]);
                }
            }
        }
    }

    // close listener socket
    close(listener);
    return 0;
}