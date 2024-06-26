# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
int main(){
    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    //create sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(22222);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    //create buf
    char sendbuf[] = "Hello,world\n";
    char recvbuf[100];

    //connecting
    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == 0){
        int msglen = send(fd, &sendbuf, sizeof(sendbuf), 0);
        if(msglen > 0){
            recv(fd, &recvbuf, sizeof(recvbuf), 0);
            printf("%s", recvbuf);
        }
        else if(msglen < 0){
            perror("send failed\n");
            return -1;
        }
        else{
            perror("disconnected\n");
        }    
    }
    else{
        //error
        perror("connect failed\n");
        return -1;
    }

    close(fd);
    return 0;
}