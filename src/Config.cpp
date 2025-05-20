#include "../inc/Config.hpp"

Config::Config() {}

Config::Config(std::string& config_block, char* envp[]) : _base_env(envp){ (void)config_block; }

Config::Config(const Config& other) :_base_env(other._base_env){}

Config::~Config() {}

Config& Config::operator=(const Config& other) {
    std::cout << YELLOW << "Config assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}