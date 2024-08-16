#ifndef HTTP_H
#define HTTP_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "pro.h"

class HttpReq{
public:
    std::string method;
    std::string url;
    std::string ver;
    std::map<std::string, std::string> reqMap;
    std::string body;

    Config config;

    HttpReq();
    HttpReq(const Config& config);

    void readhttp(const std::string& buffer);
    void print();

    std::map<std::string, std::string> readbody();
    std::string infoSearch(const std::map<std::string, std::string>& table);

    bool quesCheck();
    void getinfo();

    bool isProtected();
    bool haveBasic();
    bool basicPassed();

    // HttpReq& operator=(const HttpReq& other);
};

// enum TYPE{text, bin};

class HttpRes{
public:
    std::string ver;
    std::string statu;
    // TYPE Type;

    std::string WWW_Authenticate;

    std::string type;
    std::string length;
    std::string body;

    std::string gethead();
    void mimeChange(const std::filesystem::path& filepath);
};

bool sourceExist(const std::filesystem::path& filepath);
std::string readTextFile(const std::string& url);
// std::string readBinFile(const std::string& url);
#endif