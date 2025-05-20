#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include "Connection.hpp"
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

class Request {
public:
    enum Method { GET, HEAD, POST, PUT, DELETE, OPTIONS, TRACE };
    enum URIType { DIRECTORY, FILE, CGI_PROGRAM };
    enum TransferType { GENERAL, CHUNKED };
private:
    /* member attributes */
    Connection*                         _connection;
    Server*                             _server;
    Location*                           _location;
    timeval                             _start_at;
    Method                              _method;
    std::string                         _uri;
    URIType                             _uri_type;
    std::map<std::string, std::string>  _headers;
    TransferType                        _transfer_type;
    std::string                         _content;
    std::string                         _origin;

public:
    /* Orthodox Canonical Form (OCF) */
    Request();
    Request(Connection*, Server*, std::string& start_line);
    Request(const Request& other);
    ~Request();
    Request& operator=(const Request& other);

    /* getter & setter */
    Connection*                         getConnection()const;
    Server*                             getServer()const;
    Location*                           getLocation()const;
    Method*                             getMethod()const;
    std::string                         getUri()const;
    URIType                             getUriType()const;
    std::map<std::string,std::string>   getHeaders()const;
    TransferType                        getTransferType()const;
    std::string                         getContent()const;
    std::string                         getOrigin()const;
    
    /* additional methods */
    bool        isOverTime(const timeval& now)const;
    void        addContent(const std::string&);
    void        addOrigin(const std::string&);
    void        addHeader(const std::string& header_line);
    bool        isValidHeader(const std::string& header_line) const;

    /* exception classes */
};

/* operators */
#endif