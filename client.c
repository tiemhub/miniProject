#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

struct sending_packet {
    char sender[1024];
    char receiver[1024];
    char msg[1024];
};

void *receive_thread(void *arg);

int main() {
    struct sockaddr_in s_addr;
    int sock_fd;
    char *buf = NULL;
    struct sending_packet pck;
    int check;
    int port = 8080;
    char nickname[20];

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd <= 0) {
        perror("socket failed: ");
        exit(1);
    }

    memset(&s_addr, '0', sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
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

    printf("Input nickname: ");
    scanf("%s", nickname);
    printf("%s has entered.\n", nickname);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_thread, (void *)&sock_fd);

    int flag = 0;
    size_t line_len = 0;
    while (1) {
        getline(&buf, &line_len, stdin);
        buf[line_len - 1] = '\0';

        if (strcmp(buf, "quit\n") == 0) {
            flag = -1;
        }

        sprintf(pck.msg, "%s", buf);
        sprintf(pck.sender, "%s", nickname);
        sprintf(pck.receiver, "Server");

        send(sock_fd, (struct sending_packet *)&pck, sizeof(pck), 0);

        if (flag == -1) {
            break;
        }
    }

    shutdown(sock_fd, SHUT_WR);
    pthread_join(recv_thread, NULL);
    free(buf);
    return 0;
}

void *receive_thread(void *arg) {
    int sock_fd = *((int *)arg);
    struct sending_packet pck;

    while (1) {
        if (recv(sock_fd, &pck, sizeof(pck), 0) > 0) {
            printf("[%s]: %s\n", pck.sender, pck.msg);
        }
    }

    pthread_exit(NULL);
}
