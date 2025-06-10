#include "../inc/ResponseWriter.hpp"


void ResponseWriter::queueResponse(const Response& response) {
    _send_queue.push(response.toString());
}

bool ResponseWriter::hasDataToSend() const {
    return !_send_queue.empty();
}

ssize_t ResponseWriter::sendData() {
    if (_send_queue.empty())
        return 0;

    const std::string& data = _send_queue.front();
    size_t to_send = data.length() - _current_offset;
    
    ssize_t sent = send(_fd, data.c_str() + _current_offset, to_send, 0);

    if (sent > 0)
        _current_offset += sent;

    if (_current_offset >= data.length()) {
        _send_queue.pop();
        _current_offset = 0;
    }

    if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return 0;

    return sent;
}

bool ResponseWriter::isComplete() const { return _send_queue.empty(); }

void ResponseWriter::reset() {
    while (!_send_queue.empty()) {
        _send_queue.pop();
    }
    _current_offset = 0;
}

void ResponseWriter::setFd(int fd) { _fd = fd; }

ResponseWriter::ResponseWriter() : _fd(-1), _current_offset(0) {}

ResponseWriter::ResponseWriter(int fd) : _fd(fd), _current_offset(0) {}

ResponseWriter::~ResponseWriter() {}

ResponseWriter::ResponseWriter(const ResponseWriter &copy)
    : _fd(copy._fd), _send_queue(copy._send_queue),
      _current_offset(copy._current_offset) {}

ResponseWriter& ResponseWriter::operator=(const ResponseWriter &rhs) {
    if (this != &rhs) {
        _fd = rhs._fd;
        _send_queue = rhs._send_queue;
        _current_offset = rhs._current_offset;
    }
    return *this;
}
