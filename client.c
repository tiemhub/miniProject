#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

struct sending_packet {
    char sender[1024];
    char receiver[1024];
    char msg[1024];
};

void *receive_message(void *arg);
void printUpdate(int port,char *nickname, char *group);
char nickname[20]; //쓰레드에서도 접근을 위해 main 밖에서 정의
char group[20];
//닉네임을 쓰레드에서도 사용하므로, 상호배제를 위한 mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
    struct sockaddr_in s_addr;
    int sock_fd;
    char *buf = NULL;
    struct sending_packet pck;
    int check;
    int port = 8080;
    sprintf(group,"Server");//기본 그룹 설정
    

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

    printf("<<<< Chat client >>>>\n");
    printf("Server Port: %d\n", port);
    printf("======= Mode =======\n");
    printf("  0:  change Nickname\n");
    printf("  1:  whisper mode\n");
    printf("  2:  change Group\n");
    printf("  3:  clear&update\n");
    printf("<<<<     Log     >>>>\n");

    printf("Input nickname: ");
    scanf("%s", nickname);
    printf("%s has entered.\n", nickname);

    pthread_t recv_thread;
    pthread_create(&recv_thread, NULL, receive_message, (void *)&sock_fd);
    
    pthread_mutex_lock(&mutex);
    sprintf(pck.msg, "has entred");
    sprintf(pck.sender, "%s", nickname);
    sprintf(pck.receiver, "Server");
    pthread_mutex_unlock(&mutex);
    

    send(sock_fd, (struct sending_packet *)&pck, sizeof(pck), 0);

    int flag = 0;
    size_t line_len = 0;
    while (1) {
        getline(&buf, &line_len, stdin);
        buf[strcspn(buf, "\n")] = '\0';

        sprintf(pck.msg, "%s", buf);
        sprintf(pck.sender, "%s", nickname);
        sprintf(pck.receiver, "%s", group); //자신이 속한 그룹으로만 보낸다.
        
        if (strcmp(buf, "quit") == 0) {
            flag = -1;
        }

        if (strcmp(buf, "0") == 0) { //0을 입력할 경우, 닉네임 변경
            pthread_mutex_lock(&mutex);
            sprintf(pck.msg, "%s change name to this name", nickname);
            printf("Input nickname: ");
            scanf("%s", nickname);
            sprintf(pck.sender, "%s", nickname);
            pthread_mutex_unlock(&mutex);
        } else if (strcmp(buf, "1") == 0) { //1을 입력할 경우, 귓속말을 보냄
            printf("whisper to: ");
            scanf("%s",pck.receiver);
            getchar();
            getline(&buf, &line_len, stdin);
            buf[strcspn(buf, "\n")] = '\0';
            sprintf(pck.msg,"(whisper) %s",buf);
        } else if (strcmp(buf, "2") == 0) {
            //기존 그룹에 나갔다는 메세지 송신
            sprintf(pck.msg,"left the group %s",group);
            send(sock_fd, (struct sending_packet *)&pck, sizeof(pck), 0);
            printf("change group to: ");
            scanf("%s",group);
            sprintf(pck.msg,"join the group %s",group);
            sprintf(pck.receiver,"%s",group);
        } else if (strcmp(buf, "3") == 0) {
            system("clear");
            printUpdate(port,nickname,group);
            continue;
        }


        if (strcmp(pck.msg,"") == 0) {
            continue;
        }

        
    
        send(sock_fd, (struct sending_packet *)&pck, sizeof(pck), 0);

        if (flag == -1) {
            break;
        }
    }

    shutdown(sock_fd, SHUT_WR);
    free(buf);
    return 0;
}

void *receive_message(void *arg) {
    int sock_fd = *((int *)arg);
    struct sending_packet pck;

    while (1) {
        if (recv(sock_fd, &pck, sizeof(pck), 0) > 0) {
            pthread_mutex_lock(&mutex);
            //자신이 속한 그룹으로 오거나, 나에게 온 메세지만 출력
            if ((strcmp(pck.receiver,group)==0) || (strcmp(pck.receiver,nickname)==0)){
                printf("[%s]: %s\n", pck.sender, pck.msg);
            }
            pthread_mutex_unlock(&mutex);

        }
    }

    pthread_exit(NULL);
}

void printUpdate(int port,char *nickname, char *group) {
    printf("<<<< Chat client >>>>\n");
    printf("Server Port: %d\n", port);
    printf("======= Mode =======\n");
    printf("  0:  change Nickname\n");
    printf("  1:  whisper mode\n");
    printf("  2:  change Group\n");
    printf("  3:  clear&update\n");
    printf("======= Info =======\n");
    printf("Nickname : %s\n", nickname);
    printf("Group: %s\n", group);
    printf("<<<<     Log     >>>>\n");
}
