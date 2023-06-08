#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define MAX_CLIENTS 16

struct sending_packet {
    char sender[1024];
    char receiver[1024];
    char msg[1024];
};

void handle_client(int sock);
struct sending_packet receive_sock(int sock);
void send_sock(int sock, struct sending_packet pck);

int main() {
    int s_sock_fd, new_sock_fd;
    struct sockaddr_in s_address, c_address;
    int addrlen = sizeof(s_address);
    int check;

    s_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sock_fd == -1) {
        perror("socket failed: ");
        exit(1);
    }

    memset(&s_address, '0', addrlen);
    s_address.sin_family = AF_INET;
    s_address.sin_addr.s_addr = INADDR_ANY;
    s_address.sin_port = htons(8080);

    check = bind(s_sock_fd, (struct sockaddr *)&s_address, sizeof(s_address));
    if (check == -1) {
        perror("bind error: ");
        exit(1);
    }

    check = listen(s_sock_fd, MAX_CLIENTS);
    if (check == -1) {
        perror("listen failed ");
        exit(1);
    }

    printf("Server started. Waiting for connections...\n");

    while (1) {
        new_sock_fd = accept(s_sock_fd, (struct sockaddr *)&c_address, (socklen_t *)&addrlen);
        if (new_sock_fd < 0) {
            perror("accept failed: ");
            exit(1);
        }

        if (fork() == 0) {
            close(s_sock_fd);
            handle_client(new_sock_fd);
            exit(0);
        } else {
            close(new_sock_fd);
        }
    }

    close(s_sock_fd);
    return 0;
}

void handle_client(int sock) {
    struct sending_packet pck;
    int flag = 0;

    while (1) {
        pck = receive_sock(sock);
        printf("%s: %s\n", pck.sender, pck.msg);
        if (strcmp(pck.msg, "quit") == 0) {
            flag = -1;
        }

        sprintf(pck.msg, "Message Received");
        sprintf(pck.sender, "Server");
        sprintf(pck.receiver, "Client");

        send_sock(sock, pck);
        if (flag == -1) {
            break;
        }
    }

    shutdown(sock, SHUT_RDWR);
    close(sock);
}

struct sending_packet receive_sock(int sock) {
    struct sending_packet pck;
    recv(sock, (struct sending_packet *)&pck, sizeof(pck), 0);
    return pck;
}

void send_sock(int sock, struct sending_packet pck) {
    send(sock, (struct sending_packet *)&pck, sizeof(pck), 0);
}
