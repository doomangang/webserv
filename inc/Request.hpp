// Request.hpp
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include <algorithm>  // std::transform
#include "Enum.hpp"
#include "Utils.hpp"

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

class RequestParser;

class Request
{
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

    void                 setMethod(const std::string &method_str);
    void                 setUrl(const std::string &url_str);
    void                 setVersion(const std::string &ver_str);
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

    //header
    bool                 parseHeaderLine(const std::string &header_line);
    bool                 hasHeader(const std::string &key) const;
    std::string          getHeaderValue(const std::string &key) const;
    const std::map<std::string, std::string>& GetAllHeaders() const;
    void                 addHeader(const std::string &key, const std::string &value);

    //body
    void                 setBody(const std::string &body_str);
    void                 reserveBody(ssize_t size_hint);
    void                 setBytesToRead(ssize_t bytes);
    void                 addBodyChunk(const std::string &chunk);
    void                 cutBody(ssize_t size);

    //getter & setter
    Method               getMethod() const;
    const std::string&   getUrl() const;
    const std::string&   getVersion() const;
    size_t               getBodyPos() const;
    const std::string&   getBody() const;
    Incomplete           getStatus() const;
    ssize_t              getBytesToRead() const;
    int                  getErrorCode() const;

    void                 setStatus(Incomplete type);
    void                 setErrorCode(int code);
    bool                 hasError() const;
    void                 addBodyPos(size_t n);

    static Method  stringToMethod(const std::string& method_str);
    static bool    parseHeaderFields(const std::string&, Request& request);
    void                 cleaner();

    void parseUri();
    const std::string& getPath() const;
    const std::string& getQueryString() const;
    const std::string& getFragment() const;
    std::string getQueryParam(const std::string& key) const;

private:
    // member attributes
    Method               _method;
    std::string          _url;
    std::string          _version;

    /**
     *  - key: 모두 소문자로 변환된 헤더 이름 ("host", "content-length", ...)
     *  - value
     */
    std::map<std::string, std::string>  _header;

    std::string          _body;
    size_t               _body_pos;       // 본문에서 이미 처리된 위치
    Incomplete           _status;
    int                  _error_code;
    ssize_t              _bytes_to_read;  // 읽어야 할 남은 본문 바이트 수

    std::string _path;
    std::string _query_string;
    std::string _fragment;
    std::map<std::string, std::string> _query_params;

    std::string urlDecode(const std::string& str);
    void parseQueryString();
};

#endif
