#include "../inc/Server.hpp"

Server::Server() {
}

Server::Server(ServerManager* manager, std::string& server_block, std::string& location_blocks, Config* config) 
: _manager(manager), _config(config)
{
    (void)server_block;
    (void)location_blocks;
}

Server::Server(const Server& other) {
    *this = other;
}

Server::~Server() {
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        // assignment code here
    }
    return *this;
}


/* getter & setter */
std::vector<std::string> Server::getServerNames() const { return _server_names; }
void Server::setServerNames(std::vector<std::string>& serverNames) { _server_names = serverNames; }
void Server::addServerName(const std::string& name) { _server_names.push_back(name); }

std::string Server::getHost() const { return _host; }
void Server::setHost(const std::string& host) { _host = host; }

int Server::getPort() const { return _port; }
void Server::setPort(int port) { _port = port; }

int Server::getFd() const { return _fd; }
void Server::setFd(int fd) { _fd = fd; }

int Server::getRequestUriLimitSize() const { return _request_uri_limit_size; }
void Server::setRequestUriLimitSize(int size) { _request_uri_limit_size = size; }

int Server::getRequestHeaderLimitSize() const { return _request_header_limit_size; }
void Server::setRequestHeaderLimitSize(int size) { _request_header_limit_size = size; }

int Server::getLimitClientBodySize() const { return _limit_client_body_size; }
void Server::setLimitClientBodySize(int size) { _limit_client_body_size = size; }

std::string Server::getDefaultErrorPage() const { return _default_error_page; }
void Server::setDefaultErrorPage(const std::string& page) { _default_error_page = page; }

void Server::addErrorPage(int code, const std::string& path) { _error_pages[code] = path; }
std::string Server::getErrorPage(int code) const { 
    std::map<int, std::string>::const_iterator it = _error_pages.find(code);
    if (it != _error_pages.end())
        return it->second;
    return  _default_error_page;
}

Config* Server::getConfig() const { return _config; }
void Server::setConfig(Config* config) { _config = config; }

const std::vector<Location>& Server::getLocations() const { return _locations; }
void Server::setLocations(const std::vector<Location>& locations) { _locations = locations; }
void Server::addLocation(Location loc) { _locations.push_back(loc); }
// std::map<int, Connection>& Server::getConnections() { return _connections; }
// void Server::setConnections(const std::map<int, Connection>& connections) { _connections = connections; }

// std::queue<Response>& Server::getResponses() { return _responses; }
// void Server::setResponses(const std::queue<Response>& responses) { _responses = responses; }

ServerManager* Server::getManager() const { return _manager; }
void Server::setManager(ServerManager* manager) { _manager = manager; }
