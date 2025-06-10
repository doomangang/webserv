#ifndef RESPONSEWRITER_HPP
#define RESPONSEWRITER_HPP

#include "Webserv.hpp"
#include "Response.hpp"

class ResponseWriter {
private:
    int _fd;
    std::queue<std::string> _send_queue;
    size_t _current_offset;
    
public:
    ResponseWriter();
    ResponseWriter(int fd);
    ~ResponseWriter();
    ResponseWriter(const ResponseWriter &copy);
    ResponseWriter& operator=(const ResponseWriter &rhs);
    
    void queueResponse(const Response& response);
    bool hasDataToSend() const;
    ssize_t sendData();
    bool isComplete() const;
    void reset();
    void setFd(int fd);
};

#endif