#define _DEFAULT_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

int main()
{
    // Create client socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(8000);

    // Connecting
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    // get and send dir info
    char cwd[2048];
    getcwd(cwd, sizeof(cwd));

    if (send(client, cwd, strlen(cwd), 0) <= 0)
    {
        printf("Send directory name failed\n");
        exit(EXIT_FAILURE);
    }

    DIR *d = opendir(".");
    struct dirent *dir;
    struct stat st;
    long numFiles = 0;

    // count num files
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG)
        {
            numFiles++;
        }
    }

    // Send numFiles
    if (send(client, &numFiles, sizeof(long), 0) <= 0)
    {
        printf("Send number of files failed\n");
        exit(EXIT_FAILURE);
    }

    rewinddir(d);

    // Prepare a file to encapsulate data
    FILE *fp = fopen("new_file", "w");

    // Send file info
    while ((dir = readdir(d)) != NULL)
    {
        if (dir->d_type == DT_REG)
        {
            stat(dir->d_name, &st);

            long nameSize = strlen(dir->d_name);
            if (send(client, &nameSize, sizeof(long), 0) <= 0)
            {
                printf("Send nameSize failed\n");
                break;
            }

            if (send(client, dir->d_name, nameSize, 0) <= 0)
            {
                printf("Send nameFile failed\n");
                break;
            }

            long sz = st.st_size;
            if (send(client, &sz, sizeof(long), 0) <= 0)
            {
                printf("Send fileSize failed\n");
                break;
            }
        }
    }

    close(client);
    return 0;
}