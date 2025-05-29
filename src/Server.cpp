#include "../inc/Server.hpp"

Server::Server() {}

Server::Server(ServerManager* manager, Config* config) 
    : _manager(manager), _config(config) {}

Server::Server(const Server& other) {
    *this = other;
}

Server::~Server() {
}

Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _manager = other._manager;
    }
    return *this;
}