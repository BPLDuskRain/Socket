#include <iostream>
#include <fstream>
#include <ctime>
# include <netinet/in.h>
# include <arpa/inet.h>

#include "http.h"
#include "log.h"

AccLog::AccLog(struct sockaddr_in caddr, HttpReq* httpreq){
    url = httpreq->config.acclogPath;
    cip = inet_ntoa(caddr.sin_addr);
    cport = ntohs(caddr.sin_port);
    http = httpreq;
}

void AccLog::write(){
    std::ofstream file(url, std::ios::app);
    std::tm* nt = getTime();
    file << (nt->tm_year + 1900) << '-' << (nt->tm_mon + 1) << '-' << nt->tm_mday << ' ' << nt->tm_hour << ':' << nt->tm_min << ':' << nt->tm_sec; 
    file << '[' << cip << ':' << cport << ']' << http->method << ' ' << http->url << std::endl;
    file.close();
}

SysLog::SysLog(std::string path){
    url = path;
}

void SysLog::write(std::string sth){
    std::ofstream file(url, std::ios::app);
    std::tm* nt = getTime();
    file << (nt->tm_year + 1900) << '-' << (nt->tm_mon + 1) << '-' << nt->tm_mday << ' ' << nt->tm_hour << ':' << nt->tm_min << ':' << nt->tm_sec << ' ';
    file << sth;
    file.close();
}

std::tm* getTime(){
    std::time_t t = std::time(0);
    std::tm* nowtime = std::localtime(&t);
    return nowtime;
}
