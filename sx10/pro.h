#ifndef PRO_H
#define PRO_H
#include <yaml-cpp/yaml.h>

class Config{
public:
    std::string ip;
    unsigned short port;

    std::string acclogPath;
    std::string syslogPath;

    std::string indexPath;
    std::string blogPath;

    std::string password;

    std::vector<std::string> secure;

    std::string error401;
    std::string error403;
    std::string error404;

    
    void readYaml(std::string path);

};

#endif