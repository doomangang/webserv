#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Webserv.hpp"

class Connection;
class Server;
class Location;

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
    time_t                             _start_at;
    Method                              _method;
    std::string                         _uri;
    URIType                             _uri_type;
    std::map<std::string, std::string>  _headers;
    TransferType                        _transfer_type;
    std::string                         _content;
    std::string                         _origin;
    Request();

public:
    /* Orthodox Canonical Form (OCF) */
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
    bool        isOverTime(const time_t& now)const;
    void        addContent(const std::string&);
    void        addOrigin(const std::string&);
    void        addHeader(const std::string& header_line);
    bool        isValidHeader(const std::string& header_line) const;

    /* exception classes */
};

/* operators */
#endif