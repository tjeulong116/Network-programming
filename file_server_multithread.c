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
#include <dirent.h>

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

    // Accept client
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
    int numFiles = 0;

    char header[100];
    char content_list[50000];
    char buf[2048];
    memset(header, 0, sizeof(header));
    memset(content_list, 0, sizeof(content_list));

    DIR *FD = opendir(".");
    struct dirent *in_file;

    if (FD == NULL)
    {
        perror("Open directory failed");
        close(client);
        pthread_exit(NULL);
    }

    while ((in_file = readdir(FD)) != NULL)
    {
        numFiles++;
        strcat(content_list, in_file->d_name);
        strcat(content_list, "\r\n");
    }
    strcat(content_list, "\r\n\r\n");

    if (numFiles == 0)
    {
        char *msgNoFiles = "ERROR No files to download\r\n";
        send(client, msgNoFiles, strlen(msgNoFiles), 0);
        closedir(FD);
        close(client);
        pthread_exit(NULL);
    }
    else
    {
        sprintf(header, "OK %d\r\n", numFiles);
        send(client, header, strlen(header), 0);
        send(client, content_list, strlen(content_list), 0);
    }

    while (1)
    {
        int retRecv = recv(client, buf, sizeof(buf), 0);

        if (retRecv <= 0)
        {
            printf("Client %d has disconnected from server\n", client);
            closedir(FD);
            close(client);
            pthread_exit(NULL);
        }

        buf[retRecv] = '\0';
        buf[strcspn(buf, "\n")] = '\0';

        if (strncmp(buf, "exit", 4) == 0)
        {
            printf("Client %d has disconnected from server\n", client);
            closedir(FD);
            close(client);
            pthread_exit(NULL);
        }

        // Check if a file exists
        if (access(buf, F_OK) == 0)
        {
            memset(header, 0, sizeof(header));

            FILE *fp = fopen(buf, "rb");

            fseek(fp, 0, SEEK_END);
            long size = ftell(fp);
            rewind(fp);
            sprintf(header, "OK %ld\r\n", size);
            send(client, header, strlen(header), 0);

            int len;
            while ((len = fread(buf, 1, sizeof(buf), fp)) > 0)
            {
                send(client, buf, len, 0);
            }

            fclose(fp);
            break;
        }
        else
        {
            char *fileNotFound = "Cannot find this file in the current directory. Please send again\n";
            send(client, fileNotFound, strlen(fileNotFound), 0);
            continue;
        }
    }

    closedir(FD);
    close(client);
}