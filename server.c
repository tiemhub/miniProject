#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 100 // 최대 접속 가능한 클라이언트 수

int clients[MAX_CLIENTS]; // 클라이언트 소켓 보관을 위한 어레이
int client_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct sending_packet {
    char sender[1024];
    char receiver[1024];
    char msg[1024];
};

void *handle_thread(void *arg);
void send_message(struct sending_packet pck, int sender_sock);

int main() {
    int s_sock_fd, new_sock_fd;
    struct sockaddr_in s_address, c_address;
    int addrlen = sizeof(s_address);
    int check;
    pthread_t t_id;

    int port = 8080;

    s_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (s_sock_fd == -1) {
        perror("socket failed: ");
        exit(1);
    }

    memset(&s_address, '0', addrlen);
    s_address.sin_family = AF_INET;
    s_address.sin_addr.s_addr = INADDR_ANY;
    s_address.sin_port = htons(port);

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

    printf("<<<< Chat server >>>>\n");
    printf("Server Port: %d\n", port);
    printf("Max Client: %d\n", MAX_CLIENTS);
    printf("<<<< Log >>>>\n");

    while (1) {
        new_sock_fd = accept(s_sock_fd, (struct sockaddr *)&c_address, (socklen_t *)&addrlen);
        if (new_sock_fd < 0) {
            perror("accept failed: ");
            exit(1);
        }

        if (pthread_create(&t_id, NULL, handle_thread, (void *)&new_sock_fd) != 0) {
            perror("pthread_create failed: ");
            exit(1);
        }
        printf("chatter (%d/100)\n",client_count+1);
    }

    close(s_sock_fd);
    return 0;
}

void *handle_thread(void *arg) {
    int sock = *((int *)arg);
    struct sending_packet packet;
    int flag = 0;

    pthread_mutex_lock(&mutex);
    clients[client_count++] = sock;
    pthread_mutex_unlock(&mutex);

    while (1) {
        memset(&packet, 0, sizeof(packet));

        int received = recv(sock, &packet, sizeof(packet), 0);
        if (received <= 0) {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == sock) {
                    client_count--;
                    for (int j = i; j < client_count; j++) {
                        clients[j] = clients[j+1];
                    }
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            printf("Client disconnected\n");
            break;
        }
        if (strcmp(packet.msg, "quit\n") == 0) {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == sock) {
                    client_count--;
                    for (int j = i; j < client_count; j++) {
                        clients[j] = clients[j+1];
                    }
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            flag = -1;
        }
        
        printf("(%s)[%s]: %s\n",packet.receiver,packet.sender,packet.msg);
        
        
        send_message(packet, sock);
        if (flag == -1) {
            break;
        }
    }

    close(sock);
    pthread_exit(NULL);
}

void send_message(struct sending_packet pck, int sender_sock) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_sock) {
            send(clients[i], &pck, sizeof(pck), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}
