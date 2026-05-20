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

int reduceYearLength(int year)
{
    int reduceYear = year % 10;
    year /= 10;
    reduceYear += 10 * (year % 10);

    return reduceYear;
}

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

    // Accepting client
    while (1)
    {
        pthread_t thread_id;
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            perror("Accept() failed");
            continue;
        }

        printf("Client %d has conencted to server\n", client);

        pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
        pthread_detach(thread_id);
    }

    // Close socket
    close(listener);
    return 0;
}

void *thread_proc(void *param)
{
    int client = *(int *)param;
    char *msgWrongFormat = "Incorrect format. Please send again (GET_TIME [format])\n";
    char buf[2048];

    while (1)
    {
        int retRecv = recv(client, buf, sizeof(buf), 0);

        if (retRecv <= 0)
        {
            break;
        }

        buf[retRecv] = '\0';

        if (strncmp(buf, "exit", 4) == 0)
        {
            break;
        }

        char cmd[1000], format[1000], extra[1000];
        int len = sscanf(buf, "%s %s %s", cmd, format, extra);

        time_t t = time(NULL);
        struct tm date = *localtime(&t);
        char timeResponse[1000] = "";

        if (len != 2 || strcmp(cmd, "GET_TIME") != 0)
        {
            send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
            continue;
        }

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
        else
        {
            send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
            continue;
        }

        send(client, timeResponse, strlen(timeResponse), 0);
    }

    printf("Client %d has disconnected from server\n", client);
    close(client);
}
