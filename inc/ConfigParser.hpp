#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <vector>
#include "Server.hpp"

/* Color Sets */
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define GREY    "\033[38;5;250m"

class Server;

class ConfigParser {
private:
    /* member attributes */
    ConfigParser(const ConfigParser& other);
    ConfigParser& operator=(const ConfigParser& other);

    /* private methods */
    std::vector<std::string> extractBlocks(const std::string& config_block, 
                                            const std::string& block_type);
    
public:
    /* Orthodox Canonical Form (OCF) */
    ConfigParser();
    ~ConfigParser();

    /* getter & setter */

    /* additional methods */
    Config parseConfigFile(const std::string& filename);
};

/*
in main(), parseConfigFile() is called to read the configuration file.

*/