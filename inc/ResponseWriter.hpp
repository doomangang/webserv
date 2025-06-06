// ResponseWriter.hpp (새로 생성)
#ifndef RESPONSEWRITER_HPP
#define RESPONSEWRITER_HPP

#include "Response.hpp"
#include <queue>

class ResponseWriter {
private:
    int _fd;
    std::queue<std::string> _send_queue;
    size_t _current_offset;
    bool _keep_alive;
    
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
};

#endif