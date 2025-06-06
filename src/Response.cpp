#include "../inc/Response.hpp"

Response::Response() {
    std::cout << GREEN << "Response default constructor called\n" << RESET << std::endl;
}

Response::Response(Connection*, int status_code, const std::string& body = "") {
    
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
        // assignment code here
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