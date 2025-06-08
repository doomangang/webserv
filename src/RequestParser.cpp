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
      _expected_body_size(copy._expected_body_size),
      _received_body_size(copy._received_body_size),
      _header_end_pos(copy._header_end_pos),
      _raw_buffer(copy._raw_buffer) {
}

RequestParser& RequestParser::operator=(const RequestParser& rhs) {
    if (this != &rhs) {
        _parse_state = rhs._parse_state;
        _max_body_size = rhs._max_body_size;
        _expected_body_size = rhs._expected_body_size;
        _received_body_size = rhs._received_body_size;
        _header_end_pos = rhs._header_end_pos;
        _raw_buffer = rhs._raw_buffer;
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
    
    if (parts.size() != 3) {
        _parse_state = BAD_REQUEST;
        return;
    }

    // HTTP 메서드 파싱
    Method method = Request::stringToMethod(parts[0]);
    if (method == UNKNOWN) {
        _parse_state = BAD_REQUEST;
        return;
    }
    request.setMethod(parts[0]);  // 문자열로 전달

    // URL 파싱
    request.setUrl(parts[1]);
    request.parseUri(); // URI 파싱 추가

    // HTTP 버전 파싱
    if (parts[2] != "HTTP/1.1") {
        _parse_state = BAD_REQUEST;
        return;
    }

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
    std::vector<std::string> lines = splitByCRLF(headers_str);
    
    for (size_t i = 0; i < lines.size(); ++i) {
        if (!Request::parseHeaderFields(lines[i], request)) {
            _parse_state = BAD_REQUEST;
            return;
        }
    }

    _raw_buffer.erase(0, pos + 4);
    _header_end_pos = pos + 4;

    // 헤더 크기 제한 체크
    if (_header_end_pos > _max_header_size) {
        request.setErrorCode(431); // Request Header Fields Too Large
        _parse_state = BAD_REQUEST;
        return;
    }
    
    // 필수 헤더 검증
    if (!request.hasHeader("host")) {
        request.setErrorCode(400); // Bad Request - Host 헤더 필수
        _parse_state = BAD_REQUEST;
        return;
    }
    
    // 헤더 값 검증
    validateHeaderValues(request);
    if (_parse_state == BAD_REQUEST) {
        return;
    }
    _parse_state = HEADERS_COMPLETE;

    // Transfer-Encoding 체크
    std::string transfer_encoding = request.getHeaderValue("transfer-encoding");
    if (transfer_encoding == "chunked") {
        _is_chunked = true;
        _chunk_state = CHUNK_SIZE;
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
            
            request.addBodyChunk(_raw_buffer.substr(0, to_read));
            _raw_buffer.erase(0, to_read);
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
    // 16진수 파싱
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

bool parseRawRequest(const std::string &raw_request, Request &req) {
    std::vector<std::string> lines = splitByCRLF(raw_request);
    if (lines.empty())
        return false;

    // request line
    {
        std::string start_line = lines[0];
        HttpUtils::trim(start_line);
        // METHOD SP URI SP VERSION
        size_t p1 = start_line.find(' ');
        if (p1 == std::string::npos) return false;

        size_t p2 = start_line.find(' ', p1 + 1);
        if (p2 == std::string::npos) return false;

        std::string method_str = start_line.substr(0, p1);
        std::string url_str    = start_line.substr(p1 + 1, p2 - p1 - 1);
        std::string ver_str    = start_line.substr(p2 + 1);

        if (method_str.empty() || url_str.empty() || ver_str.empty())
            return false;

        req.setMethod(method_str);
        req.setUrl(url_str);
        req.setVersion(ver_str);
    }

    // Header block
    size_t idx = 1;
    for (; idx < lines.size(); ++idx) {
        std::string line = lines[idx];
        if (line.empty())
            break;

        if (!req.parseHeaderLine(line))
            return false;
    }


    ssize_t content_len = 0;
    if (req.hasHeader("content-length")) {
        std::string val = req.getHeaderValue("content-length");
        content_len = std::atoi(val.c_str());
        if (content_len < 0) content_len = 0;
        req.setBytesToRead(content_len);
        req.reserveBody(content_len);  // 미리 reserve 해 두면 효율적
    }

    if (content_len > 0 && idx + 1 < lines.size()) {
        std::string accumulated;
        for (size_t j = idx + 1; j < lines.size(); ++j) {
            accumulated += lines[j];
            if (j + 1 < lines.size()) {
                accumulated += "\r\n";
            }
        }

        if (static_cast<ssize_t>(accumulated.size()) > content_len) {
            accumulated = accumulated.substr(0, content_len);
        }
        req.setBody(accumulated);
        req.addBodyPos(static_cast<size_t>(accumulated.size()));
    }

    req.setStatus(COMPLETE);
    return true;
}

void RequestParser::reset() {
    _parse_state = NONE;
    _expected_body_size = 0;
    _received_body_size = 0;
    _header_end_pos = 0;
    _raw_buffer.clear();
}

bool RequestParser::isChunked() const {return _is_chunked; }