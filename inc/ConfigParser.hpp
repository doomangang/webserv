#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserv.hpp"
#include "HttpUtils.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "Server.hpp"

class Server;

class ConfigParser {
private:
    /* member attributes */
    ConfigParser(const ConfigParser& other);
    ConfigParser& operator=(const ConfigParser& other);

    /* private methods */
    std::string                 readConfigFile(std::string);
    std::string                 preprocessConfig(std::string);
    std::vector<std::string>    extractBlocks(const std::string config_block, 
                                            const std::string block_type);
    Server                      parseServerBlock(std::string);
    Location                    parseLocationBlock(const std::string& locText);
    std::string                 extractLocationUri(const std::string& text);
    std::string                 extractBlockBody(const std::string& text);
    std::vector<std::string>    splitStatements(const std::string& body);
    void                        parseMethodsDirective(Location& loc, const std::string& stmt);
    void                        parseRootDirective(Location& loc, const std::string& stmt);
    void                        parseIndexDirective(Location& loc, const std::string& stmt);
    void                        parseAutoindexDirective(Location& loc, const std::string& stmt);
    void                        parseCgiDirective(Location& loc, const std::string& stmt);
    void                        parseUploadStoreDirective(Location& loc, const std::string& stmt);
    void                        parseReturnDirective(Location& loc, const std::string& stmt);

    void validatePort(int port);
    void validatePath(const std::string& path);
    void validateBodySize(size_t size);
    void validateServerBlock(const Server& srv);

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

    class InvalidPortException : public std::runtime_error {
    public:
        InvalidPortException(int port) 
            : std::runtime_error("Invalid port number: " + HttpUtils::toString(port) + 
                               " (must be between 1-65535)") {}
    };
    
    class InvalidPathException : public std::runtime_error {
    public:
        InvalidPathException(const std::string& path) 
            : std::runtime_error("Invalid path: " + path + " (does not exist or not accessible)") {}
    };
    
    class MissingDirectiveException : public std::runtime_error {
    public:
        MissingDirectiveException(const std::string& directive) 
            : std::runtime_error("Missing required directive: " + directive) {}
    };
};

/*
in main(), parseConfigFile() is called to read the configuration file.

*/

#endif