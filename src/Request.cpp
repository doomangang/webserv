#include "../inc/Request.hpp"

Request::Request() 
    : _method(GET),
      _body_pos(0), 
      _status(NONE), 
      _error_code(0), 
      _bytes_to_read(0) {
}

Request::~Request() {}

Request::Request(const Request& copy)
    : _method(copy._method),
      _url(copy._url),
      _version(copy._version),
      _header(copy._header),
      _body(copy._body),
      _body_pos(copy._body_pos),
      _status(copy._status),
      _error_code(copy._error_code),
      _bytes_to_read(copy._bytes_to_read) {
}

Request& Request::operator=(const Request& rhs) {
    if (this != &rhs) {
        _method = rhs._method;
        _url = rhs._url;
        _version = rhs._version;
        _header = rhs._header;
        _body = rhs._body;
        _body_pos = rhs._body_pos;
        _status = rhs._status;
        _error_code = rhs._error_code;
        _bytes_to_read = rhs._bytes_to_read;
    }
    return *this;
}

void Request::setMethod(const std::string& method_str) {
    _method = stringToMethod(method_str);
}

void Request::setUrl(const std::string& url_str) {
    _url = url_str;
}

void Request::setVersion(const std::string& ver_str) {
    _version = ver_str;
}

bool Request::parseHeaderLine(const std::string& header_line) {
    return parseHeaderFields(header_line, *this);
}

bool Request::hasHeader(const std::string& key) const {
    return _header.find(key) != _header.end();
}

std::string Request::getHeaderValue(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _header.find(key);
    if (it != _header.end()) {
        return it->second;
    }
    return "";
}

const std::map<std::string, std::string>& Request::GetAllHeaders() const {
    return _header;
}

void Request::setBody(const std::string& body_str) {
    _body = body_str;
}

void Request::reserveBody(ssize_t size_hint) {
    _body.reserve(size_hint);
}

void Request::setBytesToRead(ssize_t bytes) {
    _bytes_to_read = bytes;
}

void Request::addBodyChunk(const std::string& chunk) {
    _body += chunk;
}

void Request::cutBody(ssize_t size) {
    if (size >= 0 && static_cast<size_t>(size) <= _body.length()) {
        _body = _body.substr(0, size);
    }
}

Method Request::getMethod() const {
    return _method;
}

const std::string& Request::getUrl() const {
    return _url;
}

const std::string& Request::getVersion() const {
    return _version;
}

size_t Request::getBodyPos() const {
    return _body_pos;
}

const std::string& Request::getBody() const {
    return _body;
}

ParseState Request::getStatus() const {
    return _status;
}

ssize_t Request::getBytesToRead() const {
    return _bytes_to_read;
}

int Request::getErrorCode() const {
    if (_status == BAD_REQUEST && _error_code == 0) {
        return 400;  // 기본 Bad Request
    }
    return _error_code;
}

const std::string& Request::getPath() const { return _path; }
const std::string& Request::getQueryString() const { return _query_string; }
const std::string& Request::getFragment() const { return _fragment; }

void Request::setStatus(ParseState type) {
    _status = type;
}

void Request::setErrorCode(int code) {
    _error_code = code;
    if (code != 0) {
        _status = BAD_REQUEST;
    }
}

bool Request::hasError() const {
    return _status == BAD_REQUEST || _error_code != 0;
}

void Request::addBodyPos(size_t n) {
    _body_pos += n;
}

void Request::cleaner() {
    _method = GET;
    _url.clear();
    _version.clear();
    _header.clear();
    _body.clear();
    _body_pos = 0;
    _status = NONE;
    _error_code = 0;
    _bytes_to_read = 0;
}

void Request::addHeader(const std::string &key, const std::string &value) {
    std::string lower_key = key;
    std::transform(lower_key.begin(), lower_key.end(), lower_key.begin(), ::tolower);
    _header[lower_key] = value;
}

Method Request::stringToMethod(const std::string& method_str) {
    if (method_str == "GET") return GET;
    if (method_str == "POST") return POST;
    if (method_str == "DELETE") return DELETE;
    return UNKNOWN;
}

bool Request::parseHeaderFields(const std::string& line, Request& request) {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }

    std::string key = line.substr(0, colon_pos);
    std::string value = line.substr(colon_pos + 1);
    
    // 공백 제거
    HttpUtils::trim(key);
    HttpUtils::trim(value);
    
    request.addHeader(key, value);
    return true;
}

// Request.cpp - parseUri 메서드 수정
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

// 쿼리 스트링 파싱을 별도 메서드로 분리
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
                    _query_params[urlDecode(key)] = urlDecode(value);
                }
            }
            start = pos + 1;
        }
        pos++;
    }
}

// 쿼리 파라미터 getter
std::string Request::getQueryParam(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _query_params.find(key);
    if (it != _query_params.end()) {
        return it->second;
    }
    return "";
}