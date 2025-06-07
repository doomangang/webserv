#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "Response.hpp"

class Client {
private:
    /* member attributes */
    int         _fd;
    time_t     _last_request_at;
    std::string _ip;
    int         _port;
    int                 _client_socket;
    struct sockaddr_in  _client_address;
    
public:
    /* Orthodox Canonical Form (OCF) */
    Client();
    Client(int client_fd, std::string& _ip, int _port);
    Client(const Client& other);
    ~Client();
    Client& operator=(const Client& other);
    Response response; // 클라이언트에 대한 응답 객체
    std::string cgiInputBuffer; // CGI 입력 버퍼
    std::string cgiOutputBuffer; // CGI 출력 버퍼
    std::string requestBuffer; // 클라이언트 요청 버퍼
    /* Orthodox Canonical Form (OCF) */

    Response            response;
    Request             request;
    Server              server;

    // Getter
    int getFd() const { return _fd; }
    time_t getLastRequestAt() const { return _last_request_at; }
    const std::string& getIp() const { return _ip; }
    int getPort() const { return _port; }
    int getClientSocket() const { return _client_socket; }
    const struct sockaddr_in& getClientAddress() const { return _client_address; }

    // Setter
    void setClientSocket(int sock) { _client_socket = sock; }
    void setClientAddress(const struct sockaddr_in& addr) { _client_address = addr; }
    void setFd(int fd) { _fd = fd; }
    void setLastRequestAt(const time_t& tv) { _last_request_at = tv; }
    void setIp(const std::string& ip) { _ip = ip; }
    void setPort(int port) { _port = port; }

    /* getter & setter */
    int         getFd() const;
    time_t     getLastRequestAt() const;
    std::string getClientIp() const;
    int         getClientPort() const;
    void        setLastRequestAt();
    /* additional methods */

    /* exception classes */
};

/* operators */
#endif
