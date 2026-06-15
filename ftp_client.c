#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define USER "user_20235328"
#define PASS "532806"

void send_cmd(int sock, char *cmd)
{
    send(sock, cmd, strlen(cmd), 0);
}

int recv_res(int sock, char *buffer)
{
    int n = recv(sock, buffer, 4095, 0);

    if (n <= 0)
        return -1;

    buffer[n] = '\0';
    printf("%s", buffer);

    return atoi(buffer);
}

int open_pasv(int client)
{
    char buf[4096];
    int h1, h2, h3, h4, p1, p2;

    send_cmd(client, "PASV\r\n");
    recv_res(client, buf);

    if (sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2) != 6)
    {
        printf("Cannot read PASV\n");
        return -1;
    }

    char ip[50];
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    int port = p1 * 256 + p2;

    int data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in dataAddr;
    dataAddr.sin_family = AF_INET;
    dataAddr.sin_addr.s_addr = inet_addr(ip);
    dataAddr.sin_port = htons(port);

    if (connect(data_socket, (struct sockaddr *)&dataAddr, sizeof(dataAddr)) == -1)
    {
        printf("Connect data failed\n");
        return -1;
    }

    return data_socket;
}

int main()
{
    // Create client socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1)
    {
        perror("Create socket failed");
        exit(EXIT_FAILURE);
    }

    struct hostent *host = gethostbyname("lebavui.io.vn");
    if (host == NULL)
    {
        printf("Server not found\n");
        return 1;
    }

    // Declare server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(21);
    memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);

    // Connecting
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connect failed");
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    char list[4096] = "";
    char content[4096] = "";

    recv_res(client, buffer);

    send_cmd(client, "USER " USER "\r\n");
    recv_res(client, buffer);

    send_cmd(client, "PASS " PASS "\r\n");
    recv_res(client, buffer);

    send_cmd(client, "TYPE I\r\n");
    recv_res(client, buffer);

    // List file
    int data_socket = open_pasv(client);
    if (data_socket == -1)
    {
        close(client);
        return 1;
    }

    send_cmd(client, "LIST\r\n");
    recv_res(client, buffer);

    int len = recv(data_socket, list, sizeof(list), 0);
    list[len] = '\0';

    close(data_socket);
    recv_res(client, buffer);
    printf("\nFiles list:\n%s\n", list);

    // Find question_xxxxxx.txt file
    char question[100];
    char *q = strstr(list, "question_");

    if (q == NULL)
    {
        printf("File question_xxxxxx.txt not_found\n");
        close(client);
        return 1;
    }

    sscanf(q, "%99s", question);
    question[strcspn(question, "\r\n")] = '\0';

    printf("File question: %s\n", question);

    // RETR file
    data_socket = open_pasv(client);

    char RETR_cmd[4096] = "";
    sprintf(RETR_cmd, "RETR %s\r\n", question);

    send_cmd(client, RETR_cmd);
    recv_res(client, buffer);

    len = recv(data_socket, content, sizeof(content), 0);
    content[len] = '\0';

    close(data_socket);
    recv_res(client, buffer);
    printf("File content: %s\n", content);

    // Reverse content;
    for (int i = 0; i < len / 2; i++)
    {
        char temp = content[i];
        content[i] = content[len - i - 1];
        content[len - i - 1] = temp;
    }

    // Create answer_xxxxxx.txt file
    char file_name[100] = "";
    char *p2 = strchr(question, '_');
    p2++;
    sprintf(file_name, "answer_%s", p2);

    FILE *fp = fopen(file_name, "wb");
    fwrite(content, 1, len, fp);
    fclose(fp);

    // STOR
    data_socket = open_pasv(client);

    char STOR_cmd[4096] = "";
    sprintf(STOR_cmd, "STOR %s\r\n", file_name);

    send_cmd(client, STOR_cmd);
    recv_res(client, buffer);

    send(data_socket, content, len, 0);
    close(data_socket);
    recv_res(client, buffer);

    printf("Upload file %s\n", file_name);
    send_cmd(client, "QUIT\r\n");
    recv_res(client, buffer);

    // Close socket
    close(client);
    return 0;
}