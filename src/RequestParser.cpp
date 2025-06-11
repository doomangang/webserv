#include "../inc/RequestParser.hpp"

// 마지막에 남는 조각도 lines.back()에 들어갈 수 있다

RequestParser::RequestParser() 
    :   _parse_state(NONE),
        _max_body_size(1048576),      // 1MB 기본값
        _max_header_size(8192),
        _expected_body_size(0),
        _received_body_size(0),
        _header_end_pos(0) {
    _raw_buffer.reserve(8192);  // 초기 버퍼 크기 설정
}

RequestParser::~RequestParser() {
    _raw_buffer.clear();
}

RequestParser::RequestParser(const RequestParser& copy) 
    : _parse_state(copy._parse_state),
      _max_body_size(copy._max_body_size),
      _max_header_size(copy._max_header_size),
      _raw_buffer(copy._raw_buffer),
      _is_chunked(copy._is_chunked),
      _chunk_state(copy._chunk_state),
      _current_chunk_size(copy._current_chunk_size),
      _current_chunk_received(copy._current_chunk_received),
      _expected_body_size(copy._expected_body_size),
      _received_body_size(copy._received_body_size),
      _header_end_pos(copy._header_end_pos) {
}

RequestParser& RequestParser::operator=(const RequestParser& rhs) {
    if (this != &rhs) {
        _parse_state = rhs._parse_state;
        _max_body_size = rhs._max_body_size;
        _max_header_size = rhs._max_header_size;
        _raw_buffer = rhs._raw_buffer;
        _is_chunked = rhs._is_chunked;
        _chunk_state = rhs._chunk_state;
        _current_chunk_size = rhs._current_chunk_size;
        _current_chunk_received = rhs._current_chunk_received;
        _expected_body_size = rhs._expected_body_size;
        _received_body_size = rhs._received_body_size;
        _header_end_pos = rhs._header_end_pos;
    }
    return *this;
}

void RequestParser::setMaxBodySize(size_t size) {
    _max_body_size = size;
}

void RequestParser::setMaxHeaderSize(size_t size) {
    _max_header_size = size;
}

std::string& RequestParser::getRawBuffer() {
    return _raw_buffer;
}

ParseState RequestParser::getParseState() const {
    return _parse_state;
}

bool RequestParser::isRequestLineComplete() const {
    return _parse_state >= REQUEST_LINE_COMPLETE;
}

bool RequestParser::isHeadersComplete() const {
    return _parse_state >= HEADERS_COMPLETE;
}

bool RequestParser::isParsingComplete() const {
    return _parse_state == COMPLETE;
}

bool RequestParser::isBadRequest() const {
    return _parse_state == BAD_REQUEST;
}

void RequestParser::parseRequestLine(Request& request) {
    size_t pos = _raw_buffer.find("\r\n");
    if (pos == std::string::npos) {
        _parse_state = REQUEST_LINE_INCOMPLETE;
        return;
    }

    std::string line = _raw_buffer.substr(0, pos);
    std::vector<std::string> parts = HttpUtils::splitWords(line);
    
    if (parts.size() != 3
        || parts[1].empty()
        || parts[2].empty()
        || parts[2].find("HTTP/") != 0)
    {
        _parse_state = BAD_REQUEST;
        return;
    }

    // HTTP 메서드 파싱
    Method method = HttpUtils::stringToMethod(parts[0]);
    if (method == UNKNOWN_METHOD) {
        _parse_state = BAD_REQUEST;
        return;
    }
    request.setMethod(parts[0]);

    // URL 파싱
    request.setUrl(parts[1]);
    request.parseUri();

    _raw_buffer.erase(0, pos + 2);
    _parse_state = REQUEST_LINE_COMPLETE;
}

void RequestParser::parseHeaders(Request& request) {
    size_t pos = _raw_buffer.find("\r\n\r\n");
    if (pos == std::string::npos) {
        _parse_state = HEADERS_INCOMPLETE;
        return;
    }

    std::string headers_str = _raw_buffer.substr(0, pos);
    std::vector<std::string> lines = HttpUtils::splitByCRLF(headers_str);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        if (!request.parseHeaderFields(lines[i])) {
            _parse_state = BAD_REQUEST;
            return;
        }
    }

    _raw_buffer.erase(0, pos + 4);
    _header_end_pos = pos + 4;

    if (_header_end_pos > _max_header_size) {
        request.setErrorCode(431); // Request Header Fields Too Large
        _parse_state = BAD_REQUEST;
        return;
    }
    
    if (!request.hasHeader("host")) {
        request.setErrorCode(400);
        _parse_state = BAD_REQUEST;
        return;
    }
    
    validateHeaderValues(request);
    if (_parse_state == BAD_REQUEST) {
        return;
    }
    _parse_state = HEADERS_COMPLETE;

    std::string transfer_encoding = request.getHeaderValue("transfer-encoding");
    if (transfer_encoding == "chunked") {
        _is_chunked = CHUNKED;
        _parse_state = HEADERS_COMPLETE;
        return;
    }

    // Content-Length 헤더가 있으면 본문 크기 설정
    std::string content_length = request.getHeaderValue("Content-Length");
    if (!content_length.empty()) {
        try {
            _expected_body_size = std::atoi(content_length.c_str());
            if (_expected_body_size > _max_body_size) {
                _parse_state = BAD_REQUEST;
                return;
            }
        } catch (const std::exception&) {
            _parse_state = BAD_REQUEST;
            return;
        }
    }
}

void RequestParser::validateHeaderValues(Request& request) {
    // Content-Length 검증
    if (request.hasHeader("content-length")) {
        std::string cl = request.getHeaderValue("content-length");
        for (size_t i = 0; i < cl.length(); ++i) {
            if (!std::isdigit(cl[i])) {
                request.setErrorCode(400);
                _parse_state = BAD_REQUEST;
                return;
            }
        }
    }
    
    // Host 헤더 형식 검증
    std::string host = request.getHeaderValue("host");
    if (host.empty() || host.find_first_of("\r\n") != std::string::npos) {
        request.setErrorCode(400);
        _parse_state = BAD_REQUEST;
        return;
    }
    
    // HTTP 버전 검증
    if (request.getVersion() != "HTTP/1.1" && request.getVersion() != "HTTP/1.0") {
        request.setErrorCode(505); // HTTP Version Not Supported
        _parse_state = BAD_REQUEST;
        return;
    }
}

void RequestParser::parseChunkedBody(Request& request) {
    while (!_raw_buffer.empty()) {
        if (_chunk_state == CHUNK_SIZE) {
            size_t pos = _raw_buffer.find("\r\n");
            if (pos == std::string::npos) {
                return; // 더 많은 데이터 필요
            }
            
            std::string size_line = _raw_buffer.substr(0, pos);
            if (!parseChunkSize(size_line)) {
                _parse_state = BAD_REQUEST;
                return;
            }
            
            _raw_buffer.erase(0, pos + 2);
            
            if (_current_chunk_size == 0) {
                _chunk_state = CHUNK_TRAILER;
            } else {
                _chunk_state = CHUNK_DATA;
            }
        }
        else if (_chunk_state == CHUNK_DATA) {
            size_t available = _raw_buffer.size();
            size_t needed = _current_chunk_size - _current_chunk_received;
            size_t to_read = std::min(available, needed);
            
            if (_received_body_size + to_read > _max_body_size) {
                request.setErrorCode(413); // Payload Too Large
                _parse_state = BAD_REQUEST;
                return;
            }

            request.addBodyChunk(_raw_buffer.substr(0, to_read));
            _raw_buffer.erase(0, to_read);

            _received_body_size += to_read;
            _current_chunk_received += to_read;
            
            if (_current_chunk_received >= _current_chunk_size) {
                // 청크 데이터 완료, CRLF 확인
                if (_raw_buffer.size() >= 2 && 
                    _raw_buffer[0] == '\r' && _raw_buffer[1] == '\n') {
                    _raw_buffer.erase(0, 2);
                    _current_chunk_received = 0;
                    _chunk_state = CHUNK_SIZE;
                } else {
                    _parse_state = BAD_REQUEST;
                    return;
                }
            }
        }
        else if (_chunk_state == CHUNK_TRAILER) {
            // 트레일러 헤더 처리
            size_t pos = _raw_buffer.find("\r\n\r\n");
            if (pos != std::string::npos) {
                _raw_buffer.erase(0, pos + 4);
                _parse_state = COMPLETE;
                return;
            }
            // 트레일러가 없는 경우
            if (_raw_buffer.size() >= 2 && 
                _raw_buffer[0] == '\r' && _raw_buffer[1] == '\n') {
                _parse_state = COMPLETE;
            }
            return;
        }
    }
}

bool RequestParser::parseChunkSize(const std::string& line) {
    char* end;
    _current_chunk_size = std::strtoul(line.c_str(), &end, 16);
    if (end == line.c_str()) {
        return false; // 파싱 실패
    }
    return true;
}

void RequestParser::parseBody(Request& request) {
    if (_expected_body_size == 0) {
        _parse_state = COMPLETE;
        return;
    }

    _received_body_size += _raw_buffer.size();
    request.addBodyChunk(_raw_buffer);
    _raw_buffer.clear();

    if (_received_body_size >= _expected_body_size) {
        _parse_state = COMPLETE;
    } else {
        _parse_state = BODY_INCOMPLETE;
    }
}

void RequestParser::reset() {
    _parse_state = NONE;
    _expected_body_size = 0;
    _received_body_size = 0;
    _header_end_pos = 0;
    _raw_buffer.clear();
}

TransferType RequestParser::isChunked() const {return _is_chunked; }