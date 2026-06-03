#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 9000
#define BUF_SIZE 4096
#define ROOT ""

const char *get_content_type(const char *filename)
{
    char *ext = strrchr(filename, '.');

    if (ext == NULL)
        return "text/plain";

    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0)
        return "text/html; charset=utf-8";

    if (strcmp(ext, ".txt") == 0 || strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0)
        return "text/plain; charset=utf-8";

    if (strcmp(ext, ".css") == 0)
        return "text/css";

    if (strcmp(ext, ".js") == 0)
        return "application/javascript";

    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0)
        return "image/jpeg";

    if (strcmp(ext, ".png") == 0)
        return "image/png";

    if (strcmp(ext, ".gif") == 0)
        return "image/gif";

    if (strcmp(ext, ".pdf") == 0)
        return "application/pdf";

    return "text/plain; charset=utf-8";
}

void send_404(int client)
{
    char body[] = "<html><body><h1>404 Not Found</h1></body></html>";
    char header[512];

    sprintf(header,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            strlen(body));

    send(client, header, strlen(header), 0);
    send(client, body, strlen(body), 0);
}

void send_file(int client, const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL)
    {
        send_404(client);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[512];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "Content-Disposition: inline\r\n"
            "\r\n",
            get_content_type(filepath),
            filesize);

    send(client, header, strlen(header), 0);

    char buffer[BUF_SIZE];
    int bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        send(client, buffer, bytes, 0);
    }

    fclose(fp);
}

void send_directory(int client, const char *dirpath, const char *url_path)
{
    DIR *dir = opendir(dirpath);

    if (dir == NULL)
    {
        send_404(client);
        return;
    }

    char body[20000];
    char line[2000];

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0)
            continue;

        char link[1000];

        if (strcmp(url_path, "/") == 0)
        {
            sprintf(link, "/%s", entry->d_name);
        }
        else
        {
            sprintf(link, "%s/%s", url_path, entry->d_name);
        }

        sprintf(line,
                "<li><a href=\"%s\">%s</a></li>",
                link,
                entry->d_name);

        strcat(body, line);
    }

    strcat(body, "</ul></body></html>");

    closedir(dir);

    char header[512];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            strlen(body));

    send(client, header, strlen(header), 0);
    send(client, body, strlen(body), 0);
}

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listener == -1)
    {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("bind() failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    if (listen(listener, 10) == -1)
    {
        perror("listen() failed");
        close(listener);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        char buf[BUF_SIZE];
        memset(buf, 0, sizeof(buf));

        int ret = recv(client, buf, sizeof(buf) - 1, 0);

        if (ret <= 0)
        {
            close(client);
            continue;
        }

        buf[ret] = '\0';

        printf("Request:\n%s\n", buf);

        char method[16];
        char path[512];

        sscanf(buf, "%s %s", method, path);

        if (strcmp(method, "GET") != 0)
        {
            send_404(client);
            close(client);
            continue;
        }

        char filepath[1024];

        if (strcmp(path, "/") == 0)
        {
            sprintf(filepath, "%s", ROOT);
        }
        else
        {
            sprintf(filepath, "%s%s", ROOT, path);
        }

        struct stat st;

        if (stat(filepath, &st) == -1)
        {
            send_404(client);
            close(client);
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            send_directory(client, filepath, path);
        }
        else if (S_ISREG(st.st_mode))
        {
            send_file(client, filepath);
        }
        else
        {
            send_404(client);
        }

        close(client);
    }

    close(listener);
    return 0;
}