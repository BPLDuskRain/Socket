#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <boost/beast/core/detail/base64.hpp>

#include "http.h"
#include "pro.h"

HttpReq::HttpReq(){

}

HttpReq::HttpReq(const Config& config){
    this->config = config;
}

void HttpReq::readhttp(const std::string& buffer){
    std::istringstream iss(buffer);
    std::string line;

    iss >> method;
    iss >> url;
    if(url == "/"){
        url = config.indexPath;
    }
    else{
        url = config.blogPath + url;
    }
    iss >> ver;
    std::getline(iss, line);
    while(std::getline(iss, line)){
        if(line.empty() || line == "\r"){
            break;
        }

        size_t divide = line.find(':');
        if(divide != std::string::npos){
            std::string key = line.substr(0, divide);
            std::string val = line.substr(divide + 2);
            reqMap[key] = val;
            // std::cout << key << ' ' << val << std::endl;
        }
    }

    while(std::getline(iss, line)){
        body += line;
    }
}

void HttpReq::print(){
    std::cout << method << ' ' << url << ' ' << ver << std::endl;
    for(auto p = reqMap.begin(); p != reqMap.end(); p++){
        std::cout << p->first << ": " << p->second << std::endl;
    }
    std::cout << std::endl << body << std::endl;
}

std::map<std::string, std::string> HttpReq::readbody(){
    std::istringstream iss(body);
    std::string line;
    std::map<std::string, std::string> elms;

    std::getline(iss, line);
    while(true){
        size_t divide = line.find('&');
        if(divide != std::string::npos){
            std::string elm = line.substr(0, divide);
            line = line.substr(divide + 1);

            size_t eq = elm.find('=');
            if(eq != std::string::npos){
                std::string key = elm.substr(0, eq);
                std::string val = elm.substr(eq + 1);
                elms[key] = val;
                // std::cout << key << ' ' << val << std::endl;
            }
        }
        else{
            size_t eq = line.find('=');
            if(eq != std::string::npos){
                std::string key = line.substr(0, eq);
                std::string val = line.substr(eq + 1);
                elms[key] = val;
                // std::cout << key << ' ' << val << std::endl;
            }
            break;
        }
    }
    return elms;
}

std::string HttpReq::infoSearch(const std::map<std::string, std::string>& table){
    auto it1 = table.find("filename");
    if(it1 != table.end()){
        std::string file = config.blogPath + '/' + it1->second;
        std::filesystem::path filepath = file;
        if(!sourceExist(filepath)){
            std::cout << "no such file" << std::endl;
            return "";
        }
        std::string allfile = readTextFile(file);
        std::istringstream iss(allfile);
        std::string line;
        while(std::getline(iss, line)){
            // std::cout << line << std::endl;
            size_t loc = line.find(' ');
            if(loc != std::string::npos){
                std::string id = line.substr(0, loc);
                auto it2 = table.find("id");
                if(it2 != table.end()){
                    if(it2->second == id){
                        // std::cout << line << std::endl;
                        return line;
                    }
                }
            }
        }
        std::cout << "no such id" << std::endl;
        return "";
    }
    return "";
}

bool HttpReq::quesCheck(){
    size_t que = url.find('?');
    if(que == std::string::npos){
        return false;
    }
    return true;
}

void HttpReq::getinfo(){
    size_t que = url.find('?');
    if(que != std::string::npos){
        body = url.substr(que+1);
        url = url.substr(0, que);
        // std::cout << url << std::endl;
        // std::cout << body << std::endl;
    }
}

bool HttpReq::isProtected(){
    for(const auto& i : config.secure){
        if(url == i){
            return true;
        }
    }
    return false;
}

bool HttpReq::haveBasic(){
    auto it = reqMap.find("Authorization");
    if(it == reqMap.end()){
        return false;
    }
    std::string line = it->second;
    size_t blank = line.find(' ');
    std::string ba = line.substr(0, blank);
    if(ba == "Basic"){
        return true;
    }
    else{
        return false;
    }
}

bool HttpReq::basicPassed(){
    std::string line = reqMap["Authorization"];
    size_t blank = line.find(' ');
    std::string ba = line.substr(blank+1);

    std::size_t max_decoded_size = boost::beast::detail::base64::decoded_size(ba.size());
    std::vector<char> ba_decode_chars(max_decoded_size);

    boost::beast::detail::base64::decode(ba_decode_chars.data(), ba.data(), ba.size());

    std::string ba_decode(ba_decode_chars.begin(), ba_decode_chars.end());
    size_t r1 = ba_decode.find('\r');
    if(r1 != std::string::npos){
        ba_decode = ba_decode.substr(0, r1);
    }

    std::string allFile = readTextFile(config.password);
    std::istringstream iss(allFile);
    std::string fileline;
    while(std::getline(iss, fileline)){
        size_t r2 = line.find('\r');
        if(r2 != std::string::npos){
            ba_decode = ba_decode.substr(0, r2);
        }
        if(fileline == ba_decode){
            return true;
        }
    }
    return false;
}

// HttpReq& HttpReq::operator=(const HttpReq& other){
//     return *this;
// }

std::string HttpRes::gethead(){
    if(WWW_Authenticate == ""){
        return ver + ' ' +statu + '\n' + 
            "Content-Type: " + type + '\n' +
            "Content-Length: " + length + '\n' +
            '\n' +
            body;
    } 
    else{
        return ver + ' ' +statu + '\n' + 
            "WWW-Authenticate: " + WWW_Authenticate + '\n' +
            "Content-Type: " + type + '\n' +
            "Content-Length: " + length + '\n' +
            '\n' +
            body;        
    }
}

void HttpRes::mimeChange(const std::filesystem::path& filepath){
    auto extension = filepath.extension();
    if(extension.empty() || extension.string() == ".txt"){
        // Type = text;
        type = "text/plain";
    }
    else if(extension.string() == ".html"){
        // Type = text;
        type = "text/html";
    }
    else if(extension.string() == ".css"){
        // Type = text;
        type = "text/css";
    }
    else if(extension.string() == ".js"){
        // Type = text;
        type = "text/js";
    }
    else if(extension.string() == ".ico"){
        // Type = bin;
        type = "image/x-icon";
    }
    else if(extension.string() == ".jpg" || extension.string() == ".jpeg"){
        // Type = bin;
        type = "image/jpeg";
    }
    else if(extension.string() == ".gif"){
        // Type = bin;
        type = "image/gif";
    }
    else if(extension.string() == ".png"){
        // Type = bin;
        type = "image/png";
    }  
}

bool sourceExist(const std::filesystem::path& filepath){
    if(std::filesystem::exists(filepath))
        return true;
    else
        return false;
}

std::string readTextFile(const std::string& url){
    if(url == ""){
        return "";
    }
    std::ifstream file(url);

    std::ostringstream oss;
    std::string line;
    while(getline(file, line)){
        oss << line << '\n';
    }
    file.close();
    return oss.str();
}

// std::string readBinFile(const std::string url){
//     std::ifstream file(url, std::ios::binary);

//     file.seekg(0, std::ios::end);
//     std::streamsize size = file.tellg();

//     file.seekg(0, std::ios::beg);
//     std::vector<char> buf(size);
//     file.read(buf.data(), size);
//     file.close();

//     std::string buffer(buf.data(), size);
//     return buffer;
// }