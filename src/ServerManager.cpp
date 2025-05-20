#include "ServerManager.hpp"

ServerManager::ServerManager() {
    std::cout << GREEN << "ServerManager default constructor called\n" << RESET << std::endl;
}

ServerManager::ServerManager(const Config&) {
    
}

ServerManager::ServerManager(const ServerManager& other) {
    *this = other;
    std::cout << GREEN << "ServerManager copy constructor called\n" << RESET << std::endl;
}

ServerManager::~ServerManager() {
    std::cout << RED << "ServerManager destructor called\n" << RESET << std::endl;
}

ServerManager& ServerManager::operator=(const ServerManager& other) {
    std::cout << YELLOW << "ServerManager assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}