#include "../inc/Client.hpp"
#include <cstring>

Client::Client() : _fd(-1), _client_ip(""), _client_port(0) {}

Client::Client(int client_fd, std::string& client_ip, int client_port)
    : _fd(client_fd), _client_ip(client_ip), _client_port(client_port)
{
    gettimeofday(&_last_request_at, NULL);
}

Client::Client(const Client& other)
    : _fd(other._fd), _last_request_at(other._last_request_at),
      _client_ip(other._client_ip), _client_port(other._client_port)
{}

Client::~Client() {}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        _fd = other._fd;
        _last_request_at = other._last_request_at;
        _client_ip = other._client_ip;
        _client_port = other._client_port;
    }
    return *this;
}

int Client::getFd() const { return _fd; }
time_t Client::getLastRequestAt() const { return _last_request_at; }
std::string Client::getClientIp() const { return _client_ip; }
int Client::getClientPort() const { return _client_port; }
void Client::setLastRequestAt() {
    this->_last_request_at = time(NULL);
}
