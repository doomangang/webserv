#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <iostream>
#include <sys/socket.h>
#include "Enum.hpp"

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

class Connection {
private:
    /* member attributes */
    int         _fd;
    timeval     _last_request_at;
    std::string _client_ip;
    int         _client_port;
    std::string _raw_buffer;
    Progress    _progress;
    Connection();

public:
    /* Orthodox Canonical Form (OCF) */
    Connection(int client_fd, const std::string& client_ip, int client_port);
    Connection(const Connection& other);
    ~Connection();
    Connection& operator=(const Connection& other);

    /* getter & setter */
    int         getFd() const;
    timeval     getLastRequestAt() const;
    std::string getClientIp() const;
    int         getClientPort() const;
    void        setLastRequestAt(const timeval&);
    /* additional methods */
    void        readClient();
    /* exception classes */
};

/* operators */
#endif  
