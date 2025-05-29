#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <sys/stat.h>
#include <sys/select.h>
#include "Server.hpp"
#include "Config.hpp"

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
class Config;

class ConfigParser {
private:
    /* member attributes */
    ConfigParser(const ConfigParser& other);
    ConfigParser& operator=(const ConfigParser& other);

    /* private methods */
    std::string                 readConfigFile(std::string);
    std::string                 preprocessConfig(std::string);
    void                        trim(std::string& s);
    std::vector<std::string>    extractBlocks(const std::string config_block, 
                                            const std::string block_type);
    Server                      parseServerBlock(std::string);
    std::vector<std::string>    splitBySemicolon(const std::string& s);
    std::vector<std::string>    splitWords(const std::string& s);
    Location                    parseLocationBlock(const std::string& locText);
public:
    /* Orthodox Canonical Form (OCF) */
    ConfigParser();
    ~ConfigParser();

    /* getter & setter */

    /* additional methods */
    void    loadConfigFile(std::string);
    Config  parseConfigFile(const std::string&, char* []);

    /* exception classes */
    class ConfigLoadException : public std::runtime_error {
        public:
            ConfigLoadException(const std::string& msg) 
            : std::runtime_error("Config Load Error: " + msg) {}
    };

    class ConfigReadException : public std::runtime_error {
        public:
            ConfigReadException(const std::string& msg)
            : std::runtime_error("Config Read Error: " + msg) {}
    };
};

/*
in main(), parseConfigFile() is called to read the configuration file.

*/

#endif