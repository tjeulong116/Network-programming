#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

struct sinhvien
{
    char mssv[50];
    char hoTen[50];
    char dob[50];
    double gpa;
};

int main(int argc, char *argv[])
{
    // Check if user input valid arguments
    if (argc != 3)
    {
        printf("Wrong format\n");
        exit(1);
    }

    // Create socket for server
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Create socket failed");
        exit(1);
    }

    // Initialize server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));

    // Bind socket to server address
    if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind failed");
        exit(1);
    }

    // Set socket to listening state
    if (listen(listener, 5) == -1)
    {
        perror("Listen failed");
    }

    // Accept a client
    int client = accept(listener, NULL, NULL);
    if (client == -1)
    {
        perror("Accept failed");
    }

    // Receive struct message from client

    while (1)
    {
        struct sinhvien sv;

        int ret = recv(client, &sv, sizeof(sv), 0);
        if (ret <= 0)
        {
            break;
        }

        printf("Server nhan duoc thong tin 1 sinh vien:\n");
        printf("MSSV: %s\n", sv.mssv);
        printf("Ho va ten: %s\n", sv.hoTen);
        printf("Ngay sinh: %s\n", sv.dob);
        printf("Diem trung binh: %.2lf\n", sv.gpa);
        printf("\n");
    }

    // close socket
    close(client);

    return 0;
}