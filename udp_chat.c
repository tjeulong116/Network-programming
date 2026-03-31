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

int main(int argc, char *argv[])
{
    // create socket
    int source = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (source == -1)
    {
        perror("Create socket failed\n");
        exit(EXIT_FAILURE);
    }

    // declare source address
    struct sockaddr_in srcAddr;
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    srcAddr.sin_port = htons(atoi(argv[1]));

    // declare destination address
    struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_addr.s_addr = inet_addr(argv[2]);
    destAddr.sin_port = htons(atoi(argv[3]));
    int destAddrSize = sizeof(destAddr);

    // Change to non-blocking
    unsigned long ul1 = 1;
    ioctl(source, FIONBIO, &ul1);
    unsigned long ul2 = 1;
    ioctl(STDIN_FILENO, FIONBIO, &ul2);

    // Binding source address to socket
    if (bind(source, (struct sockaddr *)&srcAddr, sizeof(srcAddr)) == -1)
    {
        perror("Bind failed\n");
        close(source);
        exit(EXIT_FAILURE);
    }

    // Send and receive data
    char msg[2048];
    char buf[2048];
    while (1)
    {
        // Get data and send data
        if (fgets(msg, sizeof(msg), stdin) == NULL)
        {
            if (errno == EWOULDBLOCK)
            {
                // skip
            }
            else
            {
                printf("Cannot get message\n");
                break;
            }
        }
        else
        {
            int retSend = sendto(source, msg, strlen(msg), 0, (struct sockaddr *)&destAddr, sizeof(destAddr));
            if (retSend == -1)
            {
                if (errno == EWOULDBLOCK)
                {
                    // skip
                }
                else
                {
                    perror("Send data failed\n");
                }
            }
            else
            {
                printf("Send data to destination successfully: %s\n", msg);
            }
        }

        // Recv data
        int retRecv = recvfrom(source, buf, sizeof(buf), 0, (struct sockaddr *)&destAddr, &destAddrSize);
        if (retRecv == -1)
        {
            if (errno == EWOULDBLOCK)
            {
                // skip
            }
            else
            {
                perror("Receive data failed\n");
            }
        }
        else
        {
            buf[retRecv] = '\0';
            printf("Receive msg successfully: %s\n", buf);
        }
    }

    // close socket
    close(source);

    return 0;
}