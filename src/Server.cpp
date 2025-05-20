#include "Server.hpp"

Server::Server() {
    std::cout << GREEN << "Server default constructor called\n" << RESET << std::endl;
}

Server::Server(ServerManager*, std::string& server_block, std::string& location_blocks, Config*) {
    
}

Server::Server(const Server& other) {
    *this = other;
    std::cout << GREEN << "Server copy constructor called\n" << RESET << std::endl;
}

Server::~Server() {
    std::cout << RED << "Server destructor called\n" << RESET << std::endl;
}

Server& Server::operator=(const Server& other) {
    std::cout << YELLOW << "Server assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}