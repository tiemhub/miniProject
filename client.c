#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE 1024

struct sending_packet {
    char sender[1024];
    char receiver[1024];
    char msg[1024];
};

int main() {
    struct sockaddr_in s_addr;
    int sock_fd;
    char buffer[BUFFER_SIZE] = {0};
    struct sending_packet pck;
    int check;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd <= 0) {
        perror("socket failed: ");
        exit(1);
    }

    memset(&s_addr, '0', sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(8080);
    check = inet_pton(AF_INET, "127.0.0.1", &s_addr.sin_addr);
    if (check <= 0) {
        perror("inet_pton failed: ");
        exit(1);
    }
    check = connect(sock_fd, (struct sockaddr *)&s_addr, sizeof(s_addr));
    if (check < 0) {
        perror("connect failed ");
        exit(1);
    }

    int flag = 0;
    while (1) {
        scanf("%s", buffer);
        sprintf(pck.msg, "%s", buffer);
        sprintf(pck.sender, "Client");
        sprintf(pck.receiver, "Server");

        if (strcmp(pck.msg, "quit") == 0) {
            flag = -1;
        }

        send(sock_fd, (struct sending_packet *)&pck, sizeof(pck), 0);
        recv(sock_fd, (struct sending_packet *)&pck, sizeof(pck), 0);
        printf("%s: %s\n", pck.sender, pck.msg);

        if (flag == -1) {
            break;
        }
    }

    shutdown(sock_fd, SHUT_WR);
    return 0;
}
