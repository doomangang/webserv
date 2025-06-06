#include "../inc/Request.hpp"

Request::Request() 
    : _method(GET),
      _body_pos(0), 
      _status(NONE), 
      _error_code(0), 
      _bytes_to_read(0) {
}

Request::~Request() {
}

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

Incomplete Request::getStatus() const {
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

void Request::setStatus(Incomplete type) {
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
    Utils::trim(key);
    Utils::trim(value);
    
    request.addHeader(key, value);
    return true;
}
