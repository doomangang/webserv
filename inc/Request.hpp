// Request.hpp
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include "Enum.hpp"
#include "RequestParser.hpp"

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

class Request
{
public:
    // OCF
    Request();
    Request(const Request &copy);
    Request& operator=(const Request &rhs);
    ~Request();

    void                 setMethod(const std::string &method_str);
    void                 setUrl(const std::string &url_str);
    void                 setVersion(const std::string &ver_str);

    //header
    bool                 parseHeaderLine(const std::string &header_line);
    bool                 hasHeader(const std::string &key) const;
    std::string          getHeaderValue(const std::string &key) const;
    const std::map<std::string, std::string>& GetAllHeaders() const;

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

    void                 cleaner();

private:
    RequestParser        _parser;
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
};

#endif
