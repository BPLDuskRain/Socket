# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

# define LISTEN_NUM 1

int main(){
    //create socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        perror("socket create failed\n");
        return -1;
    }

    //create sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(22222);
    addr.sin_addr.s_addr = INADDR_ANY;

    //bind
    if(bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1){
        perror("socket bind failed\n");
        return -1;
    }
    
    //listen
    if(listen(fd, LISTEN_NUM) == -1){
        perror("start listen failed\n");
        return -1;
    }

    while(1){
        struct sockaddr_in caddr;
        int caddrlen = sizeof caddr;
        //accept connection
        int cfd = accept(fd, (struct sockaddr*)&caddr, &caddrlen);
        if(cfd == -1){
            perror("accept connection failed\n");
            return -1;
        }
        while(1){
            char buf[1000];
            int msglen = recv(cfd, buf, sizeof(buf), 0);
            if(msglen < 0){
                perror("receive failed\n");
                return -1;
            }
            else if(msglen > 0){
                printf("%s\n",buf);
                send(cfd, buf, msglen, 0);
            }
            else {
                printf("disconnected\n");
                break;
            }
        }
        close(cfd);
    }
    close(fd);
    return 0;
}