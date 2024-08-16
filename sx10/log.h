#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <fstream>
#include <ctime>
# include <netinet/in.h>
# include <arpa/inet.h>

#include "http.h"

class AccLog{
public:
    std::string url;
    std::string cip;
    unsigned short cport;
    HttpReq* http;

    AccLog(struct sockaddr_in caddr, HttpReq* httpreq);
    void write();

    friend std::tm* getTime();
};

class SysLog{
public:
    std::string url;

    SysLog(std::string path);
    void write(std::string sth);

    friend std::tm* getTime();
};

std::tm* getTime();

#endif