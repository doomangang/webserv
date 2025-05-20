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