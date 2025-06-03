#include "../inc/Connection.hpp"

Connection::Connection() {
    std::cout << GREEN << "Connection default constructor called\n" << RESET << std::endl;
}

Connection::Connection(int client_fd, const std::string& client_ip, int client_port) {

}

Connection::Connection(const Connection& other) {
    *this = other;
    std::cout << GREEN << "Connection copy constructor called\n" << RESET << std::endl;
}

Connection::~Connection() {
    std::cout << RED << "Connection destructor called\n" << RESET << std::endl;
}

Connection& Connection::operator=(const Connection& other) {
    std::cout << YELLOW << "Connection assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}