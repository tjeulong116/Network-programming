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

void signalHandler(int signo)
{
    int status;
    int pid = wait(&status);
    printf("Child process %d finished with status %d", pid, status);
}

int reduceYearLength(int year)
{
    int reduceYear = year % 10;
    year /= 10;
    reduceYear += 10 * (year % 10);

    return reduceYear;
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
    serverAddr.sin_port = htons(9200);

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

    char *msgWrongArguments = "Wrong arguments. Please enter again\n";
    char *msgWrongCommand = "Wrong command. Please enter again\n";
    char *msgWrongFormat = "Wrong format. Please enter again\n";
    signal(SIGCHLD, signalHandler);

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        printf("Client %d connected to the server\n", client);

        if (fork() == 0)
        {
            // Child process
            close(listener);
            char buf[2048];

            while (1)
            {
                int retRecv = recv(client, buf, sizeof(buf), 0);
                if (retRecv <= 0)
                {
                    printf("Client %d from child process %d disconnected\n", client, getpid());
                    break;
                }

                buf[retRecv] = '\0';

                char cmd[200];
                char format[200];
                char extra[1000];
                int words = sscanf(buf, "%s %s %s", cmd, format, extra);

                if (words != 2)
                {
                    send(client, msgWrongArguments, strlen(msgWrongArguments), 0);
                    continue;
                }
                else if (strcmp(cmd, "GET_TIME") != 0)
                {
                    send(client, msgWrongCommand, strlen(msgWrongCommand), 0);
                    continue;
                }
                else if (strcmp(format, "[dd/mm/yyyy]") != 0 && strcmp(format, "[dd/mm/yy]") != 0 && strcmp(format, "[mm/dd/yyyy]") != 0 && strcmp(format, "[mm/dd/yy]") != 0)
                {
                    send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
                    continue;
                }

                time_t t = time(NULL);
                struct tm date = *localtime(&t);
                char timeResponse[1000] = "";

                if (strcmp(format, "[dd/mm/yyyy]") == 0)
                {
                    sprintf(timeResponse, "%02d/%02d/%d\n", date.tm_mday, date.tm_mon + 1, date.tm_year + 1900);
                }
                else if (strcmp(format, "[dd/mm/yy]") == 0)
                {
                    sprintf(timeResponse, "%02d/%02d/%02d\n", date.tm_mday, date.tm_mon + 1, (reduceYearLength(date.tm_year + 1900)));
                }
                else if (strcmp(format, "[mm/dd/yyyy]") == 0)
                {
                    sprintf(timeResponse, "%02d/%02d/%d\n", date.tm_mon + 1, date.tm_mday, date.tm_year + 1900);
                }
                else if (strcmp(format, "[mm/dd/yy]") == 0)
                {
                    sprintf(timeResponse, "%02d/%02d/%02d\n", date.tm_mon + 1, date.tm_mday, reduceYearLength(date.tm_year + 1900));
                }

                send(client, timeResponse, strlen(timeResponse), 0);
            }

            close(client);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(client);
        }
    }

    // Close listener socket
    close(listener);

    return 0;
}