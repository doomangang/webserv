#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <iostream>
#include <vector>
#include "ConfigParser.hpp"
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

class ConfigParser;

class Config {
private:
    /* member attributes */
    std::vector<Server> _servers;
    std::string _software_name;
    std::string _software_version;
    std::string _http_version;
    std::string _cgi_version;
    char**      _base_env;
    
public:
    /* Orthodox Canonical Form (OCF) */
    Config();
    Config(std::vector<Server>, char* envp[]);
    Config(const Config& other);
    ~Config();
    Config& operator=(const Config& other);

    /* getter & setter */
    std::string getSoftwareName()   const;
    std::string getSoftwareVersion()const;
    std::string getHttpVersion()    const;
    std::string getCgiVersion()     const;
    char**      getBaseEnv()        const;
    std::vector<Server> getServers()const;
    Server      getMatchingServer(std::string&)const;
    Server      getDefaultServer() const;
    
    /* additional methods */

    /* exception classes */
};

/* operators */
#endif