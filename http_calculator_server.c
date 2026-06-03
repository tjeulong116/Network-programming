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
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

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

    char buf[4096];
    while (1)
    {
        int client = accept(listener, NULL, NULL);

        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }

        memset(buf, 0, sizeof(buf));
        char ops[100];
        int p1, p2, p3 = 0;
        
        int retRecv = recv(client, buf, sizeof(buf), 0);
        if (retRecv <= 0)
        {
            close(client);
            printf("recv() failed\n");
            continue;
        }

        buf[retRecv] = '\0';
        printf("%s\n", buf);

        char method[16], path[512];
        char body[1000] = "", response[2000] = "";
        int len = 0;
        char invalid_response[] = "HTTP/1.1 404 Not Found\r\n"
                                      "Content-Type: text/html\r\n"
                                      "Content-Length: 122\r\n"
                                      "\r\n"
                                      "<html><body><h1>Invalid params</h1></body></html>";

        sscanf(buf, "%s %s", method, path);
        if (strcmp(method, "GET") == 0)
        {
            len = sscanf(path, "/get?operator=%99[^&]&param1=%d&param2=%d", ops, &p1, &p2);
        }
        else if (strcmp(method, "POST") == 0)
        {
            char* post_body = strstr(buf, "\r\n\r\n");
            if(post_body == NULL) {
               printf("Body not found\n");
               close(client);
               continue; 
            }

            post_body += 4;
            
            len = sscanf(post_body, "operator=%99[^&]&param1=%d&param2=%d", ops, &p1, &p2);
        }

        if (len != 3)
        {
            send(client, invalid_response, strlen(invalid_response), 0);
            close(client);
            continue;
        }

        if (strcmp(ops, "add") == 0)
        {
            p3 = p1 + p2;
        }
        else if (strcmp(ops, "sub") == 0)
        {
            p3 = p1 - p2;
        }
        else if (strcmp(ops, "mul") == 0)
        {
            p3 = p1 * p2;
        }
        else if (strcmp(ops, "div") == 0)
        {
            if (p2 == 0)
            {
                send(client, invalid_response, strlen(invalid_response), 0);
                close(client);
                continue;
            }
            else
            {
                p3 = p1 / p2;
            }
        }
        else
        {
            send(client, invalid_response, strlen(invalid_response), 0);
            close(client);
            continue;
        }

        sprintf(body, "<html><body><h1>Answer is: %d</h1></body></html>", p3);
        sprintf(response, "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html\r\n"
                          "Content-Length: %ld\r\n"
                          "\r\n"
                          "%s",
                strlen(body), body);
        send(client, response, strlen(response), 0);

        close(client);
    }

    // close socket
    close(listener);
    return 0;
}
