# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdint.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <errno.h>

# define LISTEN_NUM 1

int sigint_flag = 0;

struct RQTPDU{
    char pdu[20];
};
struct RQTPDU_in{
    uint32_t op;
    int64_t op1;
    int64_t op2;
};

struct REPPDU{
    char pdu[8];
};
struct REPPDU_in{
    int64_t res;
};

uint64_t htonll(uint64_t value){
    return (((uint64_t)htonl(value)) << 32) + htonl(value >> 32);
}
uint64_t ntohll(uint64_t value){
    return (((uint64_t)ntohl(value)) << 32) + ntohl(value >> 32);
}

int64_t compute(uint32_t op, int64_t op1, int64_t op2, char* opchar){
    switch(op){
        case 0x00000001:
            *opchar = '+';
            return op1+op2;
        case 0x00000002:
            *opchar = '-';
            return op1-op2;
        case 0x00000004:
            *opchar = '*';
            return op1*op2;
        case 0x00000008:
            *opchar = '/';
            return op1/op2;
        case 0x00000010:
            *opchar = '%';
            return op1%op2;
    }
}

void handle_sigint(int sig){
    printf("[srv] SIGINT is coming!\n");
    sigint_flag = 1;
}

int main(int argc, char* argv[]){
    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket create failed\n");
        return -1;
    }

    //create sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)atoi(argv[2]));//port from terminal to inet
    int pton = inet_pton(AF_INET, argv[1], &(addr.sin_addr.s_addr));//ip from to inet
        if(pton == 0){
            perror("invalid chars\n");
        }else if(pton == -1){
            perror("pton failed\n");
        }

    //bind
    int binder = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(binder == -1){
        perror("socket bind failed\n");
        return -1;
    }
    
    //listen
    int listenner = listen(fd, LISTEN_NUM);
    if(listenner == -1){
        perror("start listen failed\n");
        return -1;
    }
    printf("[srv] server[%s:%s] is initializing!\n",argv[1],argv[2]);

    //sigint
    

    while(1){
        struct sockaddr_in caddr;
        int caddrlen = sizeof caddr;
        //accept connection
        int cfd = accept(fd, (struct sockaddr*)&caddr, &caddrlen);
            if(cfd == -1){
                if(errno == EINTR){
                    continue;
                }
                else{
                    perror("accept connection failed\n");
                    return -1;
                }
            }
        char *cip = inet_ntoa(caddr.sin_addr);
        short cport = ntohs(caddr.sin_port);
        printf("[srv] client[%s:%hu] is accepted!\n", cip, cport);

        while(1){
            struct RQTPDU_in recvbuf;
            struct REPPDU_in sendbuf;
            int recvmsg = read(cfd, &recvbuf, sizeof(recvbuf));
            if(recvmsg < 0){
                perror("receive failed\n");
                return -1;
            }
            else if(recvmsg ==0){
                printf("disconnected\n");
                break;
            }
            else {
                recvbuf.op = ntohl(recvbuf.op);
                recvbuf.op1 = (int64_t)ntohll(recvbuf.op1);
                recvbuf.op2 = (int64_t)ntohll(recvbuf.op2);
                
                //printf("server recv: %d, %ld %ld\n",recvbuf.op, recvbuf.op1, recvbuf.op2);
                char opchar;
                sendbuf.res = compute(recvbuf.op, recvbuf.op1, recvbuf.op2, &opchar);
                printf("[rqt_res] %ld %c %ld = %ld\n", recvbuf.op1, opchar, recvbuf.op2, sendbuf.res);
                //printf("server send: %ld\n",sendbuf.res);
                sendbuf.res = (int64_t)htonll(sendbuf.res);
                //printf("server send: %ld\n",sendbuf.res);
                write(cfd, &sendbuf, sizeof(sendbuf));
            }
        }
        close(cfd);
    }
    close(fd);
    return 0;
}