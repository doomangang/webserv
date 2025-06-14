#include "../inc/Config.hpp"
#include "../inc/Server.hpp"

Config::Config() 
    : _software_name("webserv"),
      _software_version("1.0"),
      _http_version("HTTP/1.1"),
      _cgi_version("CGI/1.1"),
      _base_env(NULL) {
}

Config::Config(std::vector<Server> servers, char* envp[]) 
    : _servers(servers),
      _software_name("webserv"),
      _software_version("1.0"),
      _http_version("HTTP/1.1"),
      _cgi_version("CGI/1.1"),
      _base_env(envp) {
}

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

const Server* Config::getMatchingServer(const std::string& host) const {
    std::string hostname = host;
    size_t colon_pos = hostname.find(':');
    if (colon_pos != std::string::npos)
        hostname = hostname.substr(0, colon_pos);
        
    for (size_t i = 0; i < _servers.size(); ++i) {
        std::vector<std::string> names = _servers[i].getServerNames();
        for (size_t j = 0; j < names.size(); ++j) {
            if (names[j] == host) {
                return &_servers[i];
            }
        }
    }
    return getDefaultServer();
}

const Server* Config::getDefaultServer() const {
    if (!_servers.empty()) {
        return &_servers[0];
    }
    throw std::runtime_error("No servers configured");
}