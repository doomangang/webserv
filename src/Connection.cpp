#include "../inc/Connection.hpp"

void Connection::readClient() {
    char buf[8192];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n > 0) {
        
    }
}






//ocf
Connection::Connection() {}

Connection::Connection(int client_fd, const std::string& client_ip, int client_port) {
    (void)client_fd; (void)client_ip; (void)client_port;
}

Connection::Connection(const Connection& other) { *this = other; }

Connection::~Connection() {}

Connection& Connection::operator=(const Connection& other) {
    if (this != &other) {
        this->_fd = other.getFd();
    }
    return *this;
}