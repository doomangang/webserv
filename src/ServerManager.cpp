#include "../inc/ServerManager.hpp"

//OCF

ServerManager::ServerManager() {}

ServerManager::ServerManager(const Config& config) : _config(config){}

ServerManager::ServerManager(const ServerManager& other) : _config(other._config){}

ServerManager::~ServerManager() {}

ServerManager& ServerManager::operator=(const ServerManager& other) {
    if (this != &other) {
        _config = other._config;
    }
    return *this;
}

// methods

bool ServerManager::splitConfigString(const std::string& raw, 
                                        std::string& outConfig, 
                                        std::vector<std::string>& outServers) 
{
    // TODO: implement
    return false;
}

bool ServerManager::splitServerString(const std::string& rawServer, 
                                        std::string& outServerBlock, 
                                        std::vector<std::string>& outLocationBlocks) 
{
    // TODO: implement
    return false;
}


/*예를 들어 splitConfigString 은:

전체 문자열에서 "server" 키워드로 블록 시작점 찾기

중괄호 {} 레벨 카운터로 블록 끝까지 자르기

그 사이 문자열을 servers.push_back(...)

남은 부분은 outConfig = ...*/