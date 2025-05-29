#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <sys/select.h>
#include <fstream>
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

class ServerManager {
public:
    enum SetType { WRITE_SET, WRITE_COPY_SET,
                    READ_SET, READ_COPY_SET,
                    ERROR_SET, ERROR_COPY_SET };

private:
    /* member attributes */
    std::vector<Server> _servers;
    Config              _config;
    int                 _max_fd;
    fd_set              _read_set,
                        _read_copy_set,
                        _write_set,
                        _write_copy_set,
                        _error_set,
                        _error_copy_set;

    ServerManager();
public:
    /* Orthodox Canonical Form (OCF) */
    ServerManager(const Config&);
    ServerManager(const ServerManager& other);
    ~ServerManager();
    ServerManager& operator=(const ServerManager& other);

    /* getter & setter */
    Config  getConfig() const;
    int     getMaxFd() const;
    void    setConfig(const Config&);
    void    setMaxFd(int);

    /* additional methods */
    // void    createServer();
    // void    runServer();
    // void    exitServer(const std::string& msg);

    // void    fdSet(int fd, SetType);
    // void    fdZero(SetType);
    // void    fdClear(int fd, SetType);
    // bool    fdIsset(int fd, SetType) const;
    // void    fdCopy(SetType origin, SetType copy);
    /* exception classes */
};

/* operators */
#endif
