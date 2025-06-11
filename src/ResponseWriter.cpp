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

    if (sent > 0) {
        _current_offset += sent;
        
        if (_current_offset >= data.length()) {
            _send_queue.pop();
            _current_offset = 0;
        }
        return sent;
    }
    
    if (sent == 0) {
        // send()가 0을 반환하는 경우는 없어야 함
        Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "send() returned 0 on fd %d", _fd);
        return 0;
    }
    
    // sent < 0인 경우
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Non-blocking 소켓에서 송신 버퍼가 가득 참
        Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "send() would block on fd %d", _fd);
        return 0;
    }
    
    // 다른 에러들 (EPIPE, ECONNRESET 등)
    Logger::logMsg(ERROR, CONSOLE_OUTPUT, "send() failed on fd %d: %s", _fd, strerror(errno));
    return sent; // -1 반환하여 상위에서 연결 종료 처리
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
