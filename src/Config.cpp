#include "Config.hpp"

Config::Config() {
    std::cout << GREEN << "Config default constructor called\n" << RESET << std::endl;
}

Config::Config(std::string& config_block, char* envp[]) {

}


Config::Config(const Config& other) {
    *this = other;
    std::cout << GREEN << "Config copy constructor called\n" << RESET << std::endl;
}

Config::~Config() {
    std::cout << RED << "Config destructor called\n" << RESET << std::endl;
}

Config& Config::operator=(const Config& other) {
    std::cout << YELLOW << "Config assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}