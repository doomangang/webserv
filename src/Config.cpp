#include "../inc/Config.hpp"

Config::Config() {}

Config::Config(std::vector<Server> servers, char* envp[]) : _servers(servers), _base_env(envp) {}

Config::Config(const Config& other) :_base_env(other._base_env){}

Config::~Config() {}

Config& Config::operator=(const Config& other) {
    std::cout << YELLOW << "Config assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        _base_env = other.getBaseEnv();
        _servers = other._servers;
    }
    return *this;
}

std::string Config::getSoftwareName() const { return _software_name; }
std::string Config::getSoftwareVersion() const { return _software_version; }
std::string Config::getHttpVersion() const { return _http_version; }
std::string Config::getCgiVersion() const { return _cgi_version; }
char**      Config::getBaseEnv() const { return _base_env; }
std::vector<Server> Config::getServers() const { return _servers; }