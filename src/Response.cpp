#include "../inc/Response.hpp"
#include "../inc/Connection.hpp"

Response::Response() {
    std::cout << GREEN << "Response default constructor called\n" << RESET << std::endl;
}

Response::Response(Connection* conn, int status_code, const std::string& body) 
    : _connection(conn),
      _status_code(status_code),
      _status_description(""),
      _transfer_type(GENERAL),
      _content(body) {
    makeStatus(status_code);
}

Response::Response(const Response& other) {
    *this = other;
    std::cout << GREEN << "Response copy constructor called\n" << RESET << std::endl;
}

Response::~Response() {
    std::cout << RED << "Response destructor called\n" << RESET << std::endl;
}

Response& Response::operator=(const Response& other) {
    std::cout << YELLOW << "Response assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        _connection = other._connection;
        _status_code = other._status_code;
        _status_description = other._status_description;
        _headers = other._headers;
        _transfer_type = other._transfer_type;
        _content = other._content;
    }
    return *this;
}

void Response::setStatusCode(int code) {
    _status_code = code;
    makeStatus(code);
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::setBody(const std::string& body) {
    _content = body;
    _headers["Content-Length"] = std::to_string(body.size());
}

void Response::makeStatus(int code) {
    _status_code = code;
    switch (code) {
        case 200: _status_description = "OK"; break;
        case 201: _status_description = "Created"; break;
        case 204: _status_description = "No Content"; break;
        case 301: _status_description = "Moved Permanently"; break;
        case 302: _status_description = "Found"; break;
        case 400: _status_description = "Bad Request"; break;
        case 401: _status_description = "Unauthorized"; break;
        case 403: _status_description = "Forbidden"; break;
        case 404: _status_description = "Not Found"; break;
        case 405: _status_description = "Method Not Allowed"; break;
        case 408: _status_description = "Request Timeout"; break;
        case 413: _status_description = "Payload Too Large"; break;
        case 414: _status_description = "URI Too Long"; break;
        case 500: _status_description = "Internal Server Error"; break;
        case 501: _status_description = "Not Implemented"; break;
        case 502: _status_description = "Bad Gateway"; break;
        case 503: _status_description = "Service Unavailable"; break;
        case 504: _status_description = "Gateway Timeout"; break;
        case 505: _status_description = "HTTP Version Not Supported"; break;
        default:  _status_description = "Unknown"; break;
    }
}

std::string Response::toString() const {
    std::ostringstream oss;
    
    // Status line
    oss << "HTTP/1.1 " << _status_code << " " << _status_description << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    
    // Empty line
    oss << "\r\n";
    
    // Body
    oss << _content;
    
    return oss.str();
}