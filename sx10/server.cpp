# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
#include <iostream>
#include <string>
#include <filesystem>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/epoll.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <errno.h>

#include "http.h"
#include "log.h"
#include "pro.h"

Config config;

int main(int argc, char* argv[]){
    config.readYaml("./config.yaml");
    SysLog syslog(config.syslogPath);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        perror("socket");
        syslog.write("[ERROR]socket create error\n");
        return -1;
    }
    // printf("socket created\n");
    syslog.write("[INFO]socket created\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = (in_port_t)htons((uint16_t)config.port);
    int pton = inet_pton(AF_INET, config.ip.c_str(), &(addr.sin_addr.s_addr));
    if(pton != 1){
        perror("pton");
        return -1;
    }

    int binder = bind(fd, (struct sockaddr* )&addr, (socklen_t)sizeof(addr));
    if(binder == -1){
        perror("bind");
        syslog.write("[INFO]socket bind error\n");
        return -1;
    }
    // printf("socket binded\n");
    syslog.write("[INFO]socket binded\n");

    int listener = listen(fd, 2);
    if(listener == -1){
        perror("listen");
        syslog.write("[ERROR]socket listen failed\n");
        return -1;
    }
    // printf("start listen\n");
    syslog.write("[INFO]start listen\n");

    int epfd1 = epoll_create(1);
    int epfd2 = epoll_create(2);
    int confd[1024] = {0};
    
    int count = 0;

    struct epoll_event event;
    struct epoll_event events[1024];
    struct epoll_event events2[1024];

    for(int i = 0; ; ){
        event.data.fd = fd;
        event.events = EPOLLIN;
        epoll_ctl(epfd1, EPOLL_CTL_ADD, fd, &event);
        
        int res = epoll_wait(epfd1, &event, 1, 0);
        struct sockaddr_in caddr;
        if(res > 0){
            if(event.data.fd == fd){
                socklen_t caddrlen = sizeof(caddr);
                confd[i] = accept(fd, (struct sockaddr*)&caddr, &caddrlen);
                if(confd[i] == -1){
                    perror("connect");
                    syslog.write("[ERROR]connection create error\n");
                }
                events[i].data.fd = confd[i];
                events[i].events = EPOLLIN;
                epoll_ctl(epfd2, EPOLL_CTL_ADD, confd[i], &events[i]);

                i = (i + 1) % 1024;
                count++;
                char *cip = inet_ntoa(caddr.sin_addr);
                unsigned short cport = ntohs(caddr.sin_port);
                printf("connected to client [%s:%hu]\n", cip, cport);
                printf("client num: %d\n", count);
            }
        }

        int res2 = epoll_wait(epfd2, events2, 1024, 0);
        
        if(res2 > 0){
            for(int j = 0; j < i; j++){
                if(events2[j].data.fd == -1) continue;
                if(events2[j].events == EPOLLIN){
                    char buf[4096] = {0};
                    int recvmsg = recv(events2[j].data.fd, buf, sizeof(buf), 0);
                    if(recvmsg <= 0){
                        if(recvmsg == 0){
                            close(events2[j].data.fd);
                            count--;
                            printf("client num: %d\n", count);
                            syslog.write("[INFO]connection disconnected\n");
                            perror("disconnect");
                            break;
                        }
                        if(recvmsg == -1){
                            syslog.write("[ERROR]connection error\n");
                            perror("recv failed");
                            exit(-1);
                        }
                    }
                    std::string buffer = buf;
                    // std::cout << buffer << std::endl;

                    HttpReq httpreq(config);
                    httpreq.readhttp(buffer);
                    //"/" -> "./"

                    AccLog acclog(caddr, &httpreq);
                    acclog.write();

                    // httpreq.print();
                    HttpRes httpres;
                    httpres.ver = httpreq.ver;

                    bool ques = httpreq.quesCheck();
                    if(ques){
                        httpreq.getinfo();
                    }

                    std::filesystem::path filepath = httpreq.url;
                    if(sourceExist(filepath)){
                        //private by basic
                        if(httpreq.isProtected()){
                            //have?
                            if(httpreq.haveBasic()){
                                //pass?
                                if(httpreq.basicPassed()){
                                    httpres.statu = "200 OK";
                                    httpres.WWW_Authenticate = "";
                                }
                                else{
                                    httpres.statu = "403 Forbidden";
                                    httpreq.url = config.error403;
                                    filepath = httpreq.url;
                                }
                            }
                            else{
                                httpres.statu = "401 Unauthorized";
                                httpres.WWW_Authenticate = "Basic";
                                httpreq.url = "";
                            }
                        }
                        //public
                        else{
                            httpres.statu = "200 OK";
                            httpres.WWW_Authenticate = "";
                        }
                    }
                    //not found
                    else{
                        httpres.statu = "404 Not Found";
                        httpreq.url = config.error404;
                        filepath = httpreq.url;
                    }
                    
                    if(httpreq.method == "GET" && !ques){
                        httpres.mimeChange(filepath);
                        
                        httpres.body = readTextFile(httpreq.url);
                        httpres.length = std::to_string(httpres.body.size());
                        // httpres.length = std::to_string(sizeof httpres.body);
                    }else if (httpreq.method == "POST" || ques){
                        std::map<std::string, std::string> table = httpreq.readbody();

                        httpres.type = "text/txt";
                        httpres.body = httpreq.infoSearch(table);
                        if(httpres.body == ""){
                            httpres.body = readTextFile(config.error404);
                        }
                        httpres.length = std::to_string(httpres.body.size());
                    }
                    
                    std::string tmp = httpres.gethead();
                    // std::string tmp = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 17\n\nHello from server";
                    int sendmsg = send(events2[j].data.fd, tmp.c_str(), tmp.size(), 0);
                    if(sendmsg == -1){
                        perror("sendmsg");
                        break;
                    }
                    // std::cout << tmp << std::endl;
                    
                }
            }
        }
    }
    close(epfd1);
    close(epfd2);
    int closer = close(fd);
    syslog.write("[INFO]socket closed\n");
    if(closer == -1){
        perror("close fd");
        syslog.write("[ERROR]socket close error\n");
        return -1;
    }
    return 0;
}