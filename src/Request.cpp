#include "../inc/Request.hpp"

Request::Request() {
    std::cout << GREEN << "Request default constructor called\n" << RESET << std::endl;
}

Request::Request(Connection*, Server*, const std::string& start_line) {
    
}

Request::Request(const Request& other) {
    *this = other;
    std::cout << GREEN << "Request copy constructor called\n" << RESET << std::endl;
}

Request::~Request() {
    std::cout << RED << "Request destructor called\n" << RESET << std::endl;
}

Request& Request::operator=(const Request& other) {
    std::cout << YELLOW << "Request assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}