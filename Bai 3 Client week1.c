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

    // Create client socket
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1)
    {
        perror("Create socket failed");
    }

    // Initialize server address
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));

    // Connect to server
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Connect failed");
    }

    // Send struct message to server

    while (1)
    {
        // Nhap vao thong tin sinh vien moi
        struct sinhvien sv;
        printf("Nhap vao thong tin sinh vien:\n");

        printf("MSSV: ");
        fgets(sv.mssv, sizeof(sv.mssv), stdin);
        fflush(stdin);

        printf("Ho va ten: ");
        fgets(sv.hoTen, sizeof(sv.hoTen), stdin);
        fflush(stdin);

        printf("Ngay sinh: ");
        fgets(sv.dob, sizeof(sv.dob), stdin);
        fflush(stdin);

        printf("Diem trung binh: ");
        scanf("%lf", &sv.gpa);
        while ((getchar()) != '\n')
            ;
        printf("\n");

        // Xu ly dau \n do fgets
        sv.mssv[strcspn(sv.mssv, "\n")] = '\0';
        sv.hoTen[strcspn(sv.hoTen, "\n")] = '\0';
        sv.dob[strcspn(sv.dob, "\n")] = '\0';

        // Gui du lieu
        int ret = send(client, &sv, sizeof(sv), 0);
        if (strncmp(sv.mssv, "exit", 4) == 0 || ret <= 0)
        {
            break;
        }
    }

    // close socket
    close(client);

    return 0;
}