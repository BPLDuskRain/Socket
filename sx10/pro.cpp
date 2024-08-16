#include <yaml-cpp/yaml.h>

#include "pro.h"

void Config::readYaml(std::string path){
    YAML::Node node = YAML::LoadFile(path);
    ip = node["ip"].as<std::string>();
    port = node["port"].as<unsigned short>();


    acclogPath = node["log"]["acclog"].as<std::string>();
    syslogPath = node["log"]["syslog"].as<std::string>();

    indexPath = node["path"]["index"].as<std::string>();
    blogPath = node["path"]["blog"].as<std::string>();

    password = node["password"].as<std::string>();

    for(const auto&i : node["secure"]){
        secure.push_back(i.as<std::string>());
    }
    
    error401 = node["error"]["error401"].as<std::string>();
    error403 = node["error"]["error403"].as<std::string>();
    error404 = node["error"]["error404"].as<std::string>();
}