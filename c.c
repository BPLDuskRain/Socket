# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <stdint.h>
# include <string.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <errno.h>

int exitFlag;

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

uint32_t opRead(char* opchar){
    if(exitFlag == 1) return 0;
    int32_t op;
    char opString[5];
    scanf("%s", opString);
    getchar();
    if(strcmp(opString, "ADD") == 0){
        *opchar = '+';
        op = 0x00000001;
    }
    else if(strcmp(opString, "SUB") == 0){
        *opchar = '-';
        op = 0x00000002;
    }
    else if(strcmp(opString, "MUL") == 0){
        *opchar = '*';
        op = 0x00000004;
    }
    else if(strcmp(opString, "DIV") == 0){
        *opchar = '/';
        op = 0x00000008;
    }
    else if(strcmp(opString, "MOD") == 0){
        *opchar = '%';
        op = 0x00000010;
    }
    else if(strcmp(opString, "EXIT") == 0){
        exitFlag = 1;
        return 0;
    }
    else{
        printf("opRead failed\n");
        return -1;
    }
    return op;
}
int64_t numRead(){
    if(exitFlag == 1) return 0;
    int64_t op;
    char opString[10];
    char* end;
    scanf("%s", opString);
    getchar();
    if(strcmp(opString, "EXIT") == 0){
        exitFlag = 1;
        return 0;
    }
    else{
        op = (int64_t)strtol(opString, &end, 10);
        if(*end != '\0'){
            printf("invalid input\n");
            return -1;
        }
        return op;
    }
    
}

int cli_biz(int fd){
    struct RQTPDU_in sendbuf;
    
    while(1){
        //receive client input
        int32_t op;
        int64_t op1;
        int64_t op2;
        char opchar;
        while(1){
            exitFlag = 0;
            
            op = opRead(&opchar);
            if (op == -1) continue;
            op1 = numRead();
            if (op1 == -1) continue;
            op2 = numRead();
            if (op2 == -1) continue;

            if(exitFlag == 1){
                printf("[cli] command EXIT received\n");
                return 0;
            }
            break;
        }
        sendbuf.op = htonl(op);
        sendbuf.op1 = (int64_t)htonll(op1);
        sendbuf.op2 = (int64_t)htonll(op2);

        //printf("%d,%ld %ld was sent\n",sendbuf.op,sendbuf.op1,sendbuf.op2);

        int sendmsg = write(fd, (struct RQTPDU*)&sendbuf, sizeof(sendbuf));
            if(sendmsg < 0){
                perror("send failed\n");
                return -1;
                
            }
            else if(sendmsg == 0){
                perror("disconnected\n");
                break;
            }
            else{
                struct REPPDU_in recvbuf;
                read(fd, (struct REPPDU*)&recvbuf, sizeof(struct REPPDU));
                //printf("client recv: %ld\n", recvbuf.res);
                recvbuf.res = (int64_t)ntohll(recvbuf.res);
                //printf("client recv: %ld\n", recvbuf.res);
                printf("[rep_rcv] %ld %c %ld = %ld\n", op1, opchar, op2, recvbuf.res);
            }
    }
    return 0;
}

int main(int argc, char* argv[]){
    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    //create sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &addr.sin_addr.s_addr);

    //connecting
    int connecter = connect(fd, (struct sockaddr*)&addr, sizeof(addr));
        if(connecter == -1){
            perror("connect failed\n");
            return -1;
        }
    printf("[cli] server[%s:%s] is connected!\n", argv[1], argv[2]);
    
    //cli_biz
    int flag = cli_biz(fd);
    if(flag == 0){
        close(fd);
        printf("[cli] connfd is closed!\n");
        printf("[cli] client is going to exit!\n");
        return 0;
    }else{
        return -1;
    }
}