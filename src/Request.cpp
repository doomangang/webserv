#include "../inc/Request.hpp"

void Request::addHeader(const std::string &key, const std::string &value) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    _headers[lower_key] = value;
}

void Request::cutBody(ssize_t size) {
    if (size >= 0 && static_cast<size_t>(size) <= _body.length()) {
        _body = _body.substr(0, size);
    }
}

void Request::addBodyPos(size_t n) {
    _body_pos += n;
}

bool Request::hasHeader(const std::string& key) const {
    return _headers.find(key) != _headers.end();
}

std::string Request::getHeaderValue(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

void Request::reserveBody(ssize_t size_hint) {
    _body.reserve(size_hint);
}

void Request::addBodyChunk(const std::string& chunk) {
    _body += chunk;
}

const std::string& Request::getPath() const { return _path; }
const std::string& Request::getQueryString() const { return _query_string; }
const std::string& Request::getFragment() const { return _fragment; }

bool Request::hasError() const {
    return _status == BAD_REQUEST || _error_code != 0;
}

bool Request::keepAlive() const {
    std::map<std::string, std::string>::const_iterator it = _headers.find("connection");
    if (it != _headers.end()) {
        if (it->second == "close") {
            return false;
        }
        if (it->second == "keep-alive") {
            return true;
        }
    }
    // If no Connection header, HTTP/1.1 defaults to keep-alive, while HTTP/1.0 and older default to close.
    if (_version == "HTTP/1.1") {
        return true;
    }
    return false;
}

void Request::cleaner() {
    _method = GET;
    _url.clear();
    _version.clear();
    _headers.clear();
    _body.clear();
    _body_pos = 0;
    _status = NONE;
    _error_code = 0;
    _bytes_to_read = 0;
}

void Request::parseUri() {
    // URL이 비어있으면 기본값 설정
    if (_url.empty()) {
        _path = "/";
        return;
    }
    
    std::string uri = _url;
    
    // 프래그먼트 추출 (#)
    size_t fragment_pos = uri.find('#');
    if (fragment_pos != std::string::npos) {
        _fragment = uri.substr(fragment_pos + 1);
        uri = uri.substr(0, fragment_pos);
    }
    
    // 쿼리 스트링 추출 (?)
    size_t query_pos = uri.find('?');
    if (query_pos != std::string::npos) {
        _query_string = uri.substr(query_pos + 1);
        _path = uri.substr(0, query_pos);
        
        // 쿼리 파라미터 파싱
        parseQueryString();
    } else {
        _path = uri;
    }
    
    // 경로가 비어있으면 기본값
    if (_path.empty()) {
        _path = "/";
    }
}

void Request::parseQueryString() {
    if (_query_string.empty()) return;
    
    size_t start = 0;
    size_t pos = 0;
    
    while (pos <= _query_string.length()) {
        // '&' 또는 문자열 끝 찾기
        if (pos == _query_string.length() || _query_string[pos] == '&') {
            if (pos > start) {
                std::string pair = _query_string.substr(start, pos - start);
                
                // '=' 위치 찾기
                size_t eq_pos = pair.find('=');
                if (eq_pos != std::string::npos) {
                    std::string key = pair.substr(0, eq_pos);
                    std::string value = pair.substr(eq_pos + 1);
                    
                    // URL 디코딩
                    _query_params[HttpUtils::urlDecode(key)] = HttpUtils::urlDecode(value);
                }
            }
            start = pos + 1;
        }
        pos++;
    }
}

bool Request::parseHeaderFields(const std::string& line) {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }

    std::string key = line.substr(0, colon_pos);
    std::string value = line.substr(colon_pos + 1);
    
    // 공백 제거
    HttpUtils::trim(key);
    HttpUtils::trim(value);
    
    this->addHeader(key, value);
    return true;
}

std::string Request::getQueryParam(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _query_params.find(key);
    if (it != _query_params.end()) {
        return it->second;
    }
    return "";
}

// setters
void                                    Request::setMethod(const std::string& method_str)     { _method          = HttpUtils::stringToMethod(method_str); }
void                                    Request::setUrl(const std::string& url_str)           { _url             = url_str; }
void                                    Request::setVersion(const std::string& version_str)   { _version         = version_str; }
void                                    Request::setErrorCode(int code)                       { _error_code      = code; }
void                                    Request::setStatus(ParseState state)                  { _status          = state; }
void                                    Request::setBody(std::string& body)                   { _body            = body; }
void                                    Request::setBytesToRead(ssize_t bytes)                { _bytes_to_read   = bytes; }

// getters
Method                                  Request::getMethod()             const { return _method; }
const std::string&                      Request::getUrl()                const { return _url; }
const std::string&                      Request::getVersion()            const { return _version; }
const std::map<std::string,std::string>&Request::getHeaders()            const { return _headers; }
const std::string&                      Request::getBody()               const { return _body; }
size_t                                  Request::getBodyPos()            const { return _body_pos; }
ssize_t                                 Request::getBytesToRead()        const { return _bytes_to_read; }
ParseState                              Request::getStatus()             const { return _status; }
int                                     Request::getErrorCode()          const { return _error_code; }


// CGI 관련 메서드 구현
std::string Request::getQuery() const {
    return getQueryString();
}

std::string Request::getMethodStr() const {
    switch (_method) {
        case GET: return "GET";
        case POST: return "POST";
        case DELETE: return "DELETE";
        case UNKNOWN_METHOD: return "UNKNOWN";
        default: return "UNKNOWN";
    }
}

Request::Request()
: _method(GET)
, _url()
, _version()
, _headers()
, _body()
, _status(REQUEST_LINE_INCOMPLETE)
, _error_code(0)
, _body_pos(0)
, _bytes_to_read(0)
{}

Request::Request(const Request& other)
: _method(other._method)
, _url(other._url)
, _version(other._version)
, _headers(other._headers)
, _body(other._body)
, _status(other._status)
, _error_code(other._error_code)
, _body_pos(other._body_pos)
, _bytes_to_read(other._bytes_to_read)
{}

Request::~Request() {}

Request& Request::operator=(const Request& other) { 
    if (this != &other) { 
        _method = other._method; 
        _url = other._url;
        _version = other._version;
        _headers = other._headers;
        _body = other._body;
        _status = other._status;
        _error_code = other._error_code;
        _body_pos = other._body_pos;
        _bytes_to_read = other._bytes_to_read;
    } 
    return *this; 
}