#include "../inc/RequestParser.hpp"

// 마지막에 남는 조각도 lines.back()에 들어갈 수 있다
static std::vector<std::string> splitByCRLF(const std::string &raw) {
    std::vector<std::string> lines;
    size_t start = 0;

    while (start < raw.size()) {
        size_t pos = raw.find("\r\n", start);
        if (pos == std::string::npos) {
            lines.push_back(raw.substr(start));
            break;
        }
        lines.push_back(raw.substr(start, pos - start));
        start = pos + 2;
    }
    return lines;
}

RequestParser::RequestParser() 
    : _parse_state(NONE),
      _max_body_size(0),
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

std::string& RequestParser::getRawBuffer() {
    return _raw_buffer;
}

Incomplete RequestParser::getParseState() const {
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
    std::vector<std::string> parts = Utils::splitWords(line);
    
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
    _parse_state = HEADERS_COMPLETE;

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
        Utils::trim(start_line);
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