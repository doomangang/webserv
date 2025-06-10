#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Webserv.hpp"
#include "Connection.hpp"
#include "CgiHandler.hpp"

class Connection;
class CgiHandler;

class Response {
private:
    /* member attributes */
    static std::map<int, std::vector<std::string> > status;
    Connection* _connection;
    int         _status_code;
    std::string _status_description;
    std::map<std::string, std::string> _headers;
    TransferType    _transfer_type;
    std::string     _content;

public:
    /* Orthodox Canonical Form (OCF) */
    Response();
    Response(Connection*, int status_code, const std::string& body);
    Response(const Response& other);
    ~Response();

    /* getter & setter */
    Connection*                         getConnection()         const;
    int                                 getStatusCode()         const;
    std::string                         getStatusDescription()  const;
    std::map<std::string, std::string>  getHeaders()            const;
    TransferType                        getTransferType()       const;
    std::string                         getContent()            const;
    std::string getHeaderValue(const std::string& key) const;

    
    void setStatusCode(int code);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    /* additional methods */
    void                                addHeader(const std::string& key, const std::string& value);
    void                                makeStatus(int code);
    std::string                         toString() const;
    /* exception classes */
    std::string     getRes();
    size_t      getLen() const;
    int         getCode() const;

    void        setRequest(Request &);
    void        setServer(Server &);

    void        buildResponse();
    void        clear();
    void        handleCgi(Request&);
    void        cutRes(size_t);
    int         getCgiState();
    void        setCgiState(int);
    void        setErrorResponse(short code);

    CgiHandler		_cgi_obj;

    std::string removeBoundary(std::string &body, std::string &boundary);
    std::string     _response_content;

    Request     request;
private:
    Server    _server;
    std::string     _target_file;
    std::vector<uint8_t> _body;
    size_t          _body_length;
    std::string     _response_body;
    std::string     _location;
    short           _code;
    char            *_res;
    int				_cgi;
    int				_cgi_fd[2];
    size_t			_cgi_response_length;
    bool            _auto_index;

    int     buildBody();
    size_t  file_size();
    void    setStatusLine();
    void    setHeaders();
    void    setServerDefaultErrorPages();
    int     readFile();
    void    contentType();
    void    contentLength();
    void    connection();
    void    server();
    void    location();
    void    date();
    int     handleTarget();
    void    buildErrorBody();
    bool    reqError();
    int     handleCgi(std::string &);
    int     handleCgiTemp(std::string &);
};

/* operators */
#endif