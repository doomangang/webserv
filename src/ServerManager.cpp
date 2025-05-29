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

bool ServerManager::loadConfigFile(std::string path) {
    struct stat sb;
    if (stat(path.c_str(), &sb) != 0)
        throw ConfigLoadException("No such file: " + path);
    
    if (!S_ISREG(sb.st_mode))
        throw ConfigLoadException("Directory or unreadable: " + path);
    
    if (sb.st_size == 0)
        throw ConfigLoadException("File is empty: " + path);

    std::ifstream ifs(path);
    if (!ifs.is_open())
        throw ConfigLoadException("Cannot open file: " + path);
    
    if (path.size() < 5 || path.substr(path.size() - 5) != ".conf")
        throw ConfigLoadException("Wrong file format - should be *.conf: " + path);
}

/*예를 들어 splitConfigString 은:

전체 문자열에서 "server" 키워드로 블록 시작점 찾기

중괄호 {} 레벨 카운터로 블록 끝까지 자르기

그 사이 문자열을 servers.push_back(...)

남은 부분은 outConfig = ...*/