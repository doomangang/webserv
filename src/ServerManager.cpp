#include "../inc/ServerManager.hpp"

//OCF

ServerManager::ServerManager() {}

ServerManager::ServerManager(const Config& config) : _config(config){}

ServerManager::ServerManager(const ServerManager& other) : _config(other._config){}

ServerManager::~ServerManager() {}

ServerManager& ServerManager::operator=(const ServerManager& other) {
    if (this != &other) {
        _config = other._config;
        _servers = other._servers;
        _max_fd = other._max_fd;
        _read_set = other._read_set;
        _read_copy_set = other._read_copy_set;
        _write_set = other._write_set;
        _write_copy_set = other._write_copy_set;
        _error_set = other._error_set;
        _error_copy_set = other._error_copy_set;
    }
    return *this;
}

Config ServerManager::getConfig() const { return _config; }
int     ServerManager::getMaxFd() const { return _max_fd; }
void    ServerManager::setConfig(const Config& config) { _config = config; }
void    ServerManager::setMaxFd(int max_fd) { _max_fd = max_fd; }

/*예를 들어 splitConfigString 은:

전체 문자열에서 "server" 키워드로 블록 시작점 찾기

중괄호 {} 레벨 카운터로 블록 끝까지 자르기

그 사이 문자열을 servers.push_back(...)

남은 부분은 outConfig = ...*/