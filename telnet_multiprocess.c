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
    serverAddr.sin_port = htons(9000);

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

    char *msgInquiry = "Enter username, password: ";
    char *msgWrongFormat = "Wrong format for sign in. Try again: ";
    char *msgLoginSuccessfully = "Log in successfully\n";
    char *msgWrongUsernameOrPassword = "Wrong username or password. Try again: ";

    signal(SIGCHLD, signalHandler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);

        if (fork() == 0)
        {
            close(listener);
            int isLogin = 0;
            char buf[2048];
            char line[2048];
            // processing login
            send(client, msgInquiry, strlen(msgInquiry), 0);

            while (1)
            {
                int retRecv = recv(client, buf, sizeof(buf), 0);

                if (retRecv <= 0)
                {
                    break;
                }

                buf[retRecv] = '\0';

                if (isLogin == 0)
                {
                    // verify login
                    char tmpUsername[200];
                    char tmpPassword[200];

                    char dbUsername[200];
                    char dbPassword[200];
                    char tmpExtra[1000];

                    int words = sscanf(buf, "%s %s %s", tmpUsername, tmpPassword, tmpExtra);
                    if (words != 2)
                    {
                        send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
                        continue;
                    }

                    FILE *fp1 = fopen("database.txt", "r");

                    while (fgets(line, sizeof(line), fp1) != NULL)
                    {
                        if (line[strlen(line) - 1] == '\n')
                        {
                            line[strlen(line) - 1] = '\0';
                        }

                        sscanf(line, "%s %s", dbUsername, dbPassword);

                        if (strcmp(tmpUsername, dbUsername) == 0)
                        {
                            if (strcmp(tmpPassword, dbPassword) == 0)
                            {
                                isLogin = 1;
                                break;
                            }
                        }
                    }

                    fclose(fp1);

                    if (isLogin == 0)
                    {
                        send(client, msgWrongUsernameOrPassword, strlen(msgWrongUsernameOrPassword), 0);
                    }
                    else
                    {
                        send(client, msgLoginSuccessfully, strlen(msgLoginSuccessfully), 0);
                    }
                }
                else if (isLogin == 1)
                {
                    if (strncmp(buf, "exit", 4) == 0)
                    {
                        break;
                    }

                    // receive command
                    char cmd[2100];
                    strcpy(cmd, buf);
                    strcat(cmd, " > out.txt");
                    system(cmd);

                    FILE *fp2 = fopen("out.txt", "r");
                    while (fgets(line, sizeof(line), fp2) != NULL)
                    {
                        send(client, line, strlen(line), 0);
                    }
                    fclose(fp2);
                }
            }

            close(client);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(client);
        }
    }

    // Free resources
    close(listener);

    return 0;
}