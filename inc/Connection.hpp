#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "Enum.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "RequestParser.hpp"
#include "Config.hpp"
#include "Server.hpp"
#include "Location.hpp"

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
    int             _fd;
    //parser related
    Request         _request;
    Response        _response;
    RequestParser   _parser;

    Progress        _progress;
    //response
    std::string     _response_buf;
    size_t          _bytes_sent;

    timeval         _last_request_at;
    std::string     _client_ip;
    int             _client_port;
    //config
    Config*         _config_ptr;          
    Server*         _server_ptr;
    Location*       _location_ptr;

    //Occf
    Connection();

    void            setupServerAndLocation();
    void            setServerData();
    void            setLocationData();
    void            updateProgress();
    void            handleParsingError();
    void            cleanUp();

    void            resetConnection();

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
    Progress    getProgress() const;

    void        setLastRequestAt(const timeval&);

    bool        isComplete() const;
    bool        needsRead()  const;
    bool        needsWrite() const;

    /* additional methods */
    void        readClient();
    void        writeClient();
    void        processRequest();

    void        prepareResponse();
    void        prepareErrorResponse(int error_code);

    /* exception classes */
};

/* operators */
#endif  
