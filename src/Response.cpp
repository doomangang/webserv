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