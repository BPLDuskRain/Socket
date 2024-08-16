# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <errno.h>

int main(int argc, char* argv[]){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        perror("socket");
        return -1;
    }
    printf("socket created\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = (in_port_t)htons((uint16_t)atoi(argv[2]));
    int pton = inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);
    if(pton != 1){
        perror("pton");
        return -1;
    }

    int connecter = connect(fd, (struct sockaddr*)&addr, (socklen_t)sizeof(addr));
    if(connecter == -1){
        perror("connect");
        return -1;
    }
    printf("connected to %s:%d\n", argv[1], atoi(argv[2]));

    while(1){
        char buf[100] = {0};
        scanf("%s", buf);
        int sendmsg = send(fd, buf, sizeof(buf), 0);
        if(sendmsg == -1){
            perror("sendmsg");
            break;
        }
        printf("send: %s\n", buf);
        if(strcmp("exit", buf) == 0){
            printf("exit\n");
            break;
        }
    }
    int closer = close(fd);
    if(closer == -1){
        perror("close");
        return -1;
    }

    return 0;
}