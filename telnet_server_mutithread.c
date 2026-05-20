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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

        printf("Client %d has connected to server\n", client);

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
    int signInState = 0;
    char *msgQuery = "Please send your username and password in this format - username password\n";
    char *msgWrongFormat = "Incorrect format. Please send again (username password)\n";
    char *signInSuccessfully = "You have signed in successfully\n";
    char *signInUnsuccessfully = "Incorrect username or password input\n";

    send(client, msgQuery, strlen(msgQuery), 0);

    char buf[2048];
    while (1)
    {
        memset(buf, 0, sizeof(buf));
        int retRecv = recv(client, buf, sizeof(buf), 0);

        if (retRecv <= 0)
        {
            break;
        }

        buf[retRecv] = '\0';
        buf[strcspn(buf, "\n")] = '\0';

        if (strncmp(buf, "exit", 4) == 0)
        {
            break;
        }

        if (signInState == 0)
        {
            char username[1000], password[1000], extra[1000];
            int len = sscanf(buf, "%s %s %s", username, password, extra);

            if (len != 2)
            {
                send(client, msgWrongFormat, strlen(msgWrongFormat), 0);
                continue;
            }

            FILE *f1 = fopen("database.txt", "r");
            char line[3000];
            char dbUsername[1000], dbPassword[1000];

            while (fgets(line, sizeof(line), f1) != NULL)
            {
                sscanf(line, "%s %s", dbUsername, dbPassword);

                if (strcmp(username, dbUsername) == 0)
                {
                    if (strcmp(password, dbPassword) == 0)
                    {
                        send(client, signInSuccessfully, strlen(signInSuccessfully), 0);
                        signInState = 1;
                        break;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            fclose(f1);

            if (signInState == 0)
            {
                send(client, signInUnsuccessfully, strlen(signInUnsuccessfully), 0);
            }
        }
        else
        {
            char cmd[3000];
            char line[3000];
            strcpy(cmd, buf);
            strcat(cmd, " > out.txt");

            // Interact with file
            pthread_mutex_lock(&mutex);
            system(cmd);

            FILE *f2 = fopen("out.txt", "r");
            while (fgets(line, sizeof(line), f2) != NULL)
            {
                printf("%s", line);
                send(client, line, strlen(line), 0);
            }
            fclose(f2);

            pthread_mutex_unlock(&mutex);
        }
    }

    signInState = 0;
    printf("Client %d has disconnected from server\n", client);
    close(client);
}