#include "../inc/ResponseWriter.hpp"

ResponseWriter::ResponseWriter() : _fd(-1) {}
ResponseWriter::ResponseWriter(int fd) : _fd(fd), _current_offset(0), _keep_alive(false) {}
ResponseWriter::~ResponseWriter() {}
ResponseWriter::ResponseWriter(const ResponseWriter &copy) 
    : _fd(copy._fd), _send_queue(copy._send_queue), 
      _current_offset(copy._current_offset), _keep_alive(copy._keep_alive) {}
ResponseWriter& ResponseWriter::operator=(const ResponseWriter &rhs) {
    if (this != &rhs) {
        _fd = rhs._fd;
        _send_queue = rhs._send_queue;
        _current_offset = rhs._current_offset;
        _keep_alive = rhs._keep_alive;
    }
    return *this;
}
