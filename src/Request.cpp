#include "../inc/Request.hpp"

Request::Request() 
    : _method(GET), 
      _body_pos(0), 
      _status(NONE), 
      _error_code(0), 
      _bytes_to_read(0) {
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

int Request::getErrorCode() const {
    if (_status == BAD_REQUEST && _error_code == 0) {
        return 400;  // 기본 Bad Request
    }
    return _error_code;
}

const std::string& Request::getBody() const {
    return _body;
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