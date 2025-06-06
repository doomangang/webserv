#include "../inc/Connection.hpp"

Connection::Connection() : _fd(-1), _request(), _response(), _progress(FROM_CLIENT),
    _bytes_sent(0), _client_port(0), _config_ptr(nullptr), _server_ptr(nullptr), _location_ptr(nullptr) {
}

Connection::Connection(int client_fd, const std::string& client_ip, int client_port)
    : _fd(client_fd), _request(), _response(), _progress(FROM_CLIENT),
    _bytes_sent(0), _client_ip(client_ip), _client_port(client_port),
    _config_ptr(nullptr), _server_ptr(nullptr), _location_ptr(nullptr) {
}

// (초기 상태)  ──> [요청 줄 파싱 시도] ──> [헤더 파싱 시도] ──> [본문 파싱 시도] ──> [완료]
// |                      |                     |
// +-- 데이터 부족 ──> 대기/추가 recv() ──> 대기/추가 recv() ──> 응답 전송
// ↓                      ↓                     ↓
// 오류 발견 ──> 400(BAD_REQUEST) 응답 & 종료


void Connection::readClient() {
    char buf[8192];
    ssize_t n = recv(_fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        if (n == 0)
            _progress = END_CONNECTION; 
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return ;
            _progress = END_CONNECTION;
        }
        return ;
    }

    std::string& raw_buffer = _parser.getRawBuffer();
    raw_buffer.append(buf, n);

    try {
        if (!_parser.isRequestLineComplete() && 
            raw_buffer.find("\r\n") != std::string::npos) {
            
            _parser.parseRequestLine(_request);
            
            if (_parser.isBadRequest()) {
                _progress = TO_CLIENT;
                return;
            }
        }
        
        if (_parser.isRequestLineComplete() && 
            !_parser.isHeadersComplete() &&
            raw_buffer.find("\r\n\r\n") != std::string::npos) {
            
            _parser.parseHeaders(_request);
            
            if (_parser.isBadRequest()) {
                _progress = TO_CLIENT;
                return;
            }
            
            setupServerAndLocation();
        }
        
        if (_parser.isHeadersComplete()) {
            _parser.parseBody(_request);
            
            if (_parser.isBadRequest()) {
                _progress = TO_CLIENT;
                return;
            }
        }
        
        // 4. 전체 파싱 상태에 따른 진행 상태 업데이트
        updateProgress();
        
    } catch (const std::exception& e) {
        _request.setErrorCode(400);
        _progress = TO_CLIENT;
    }
}

void    Connection::setupServerAndLocation() {
    if (_server_ptr == NULL)
        setServerData();

    if (_location_ptr == NULL)
        setLocationData();

    if (_server_ptr)
        _parser.setMaxBodySize(_server_ptr->getLimitClientBodySize());
}

void    Connection::updateProgress() {
    Incomplete  parse_state = _parser.getParseState();

    switch (parse_state) {
        case COMPLETE:
            cleanUp();
            processRequest();
            break ;

        case BAD_REQUEST:
            handleParsingError();
            break ;

        case REQUEST_LINE_INCOMPLETE:
        case HEADERS_INCOMPLETE:
        case TRAILER_INCOMPLETE:
            _progress = READ_CONTINUE;
            break ;

        default:
            _progress = READ_CONTINUE;
            break ;
    }
}

void Connection::setServerData() {
    if (!_config_ptr) return ;

    std::string host = _request.getHeaderValue("Host");
    if (host.empty())
        host = "localhost";
    
    _server_ptr = _config_ptr->getMatchingServer(host);
    if (!_server_ptr)
        _server_ptr = _config_ptr->getDefaultServer();
}

void Connection::setLocationData() {
    if (!_server_ptr) return ;

    std::string uri = _request.getUrl();
    _location_ptr = &(_server_ptr->getMatchingLocation(uri));
    
    if (!_location_ptr) {
        _location_ptr = &(_server_ptr->getDefaultLocation());
    }
}

void Connection::handleParsingError() {
    int error_code = _request.getErrorCode();
    if (error_code == 0) {
        error_code = 400; // 기본 Bad Request
    }
    
    prepareErrorResponse(error_code);
    _progress = TO_CLIENT;
}

void Connection::prepareResponse() {
    if (_request.hasError()) {
        prepareErrorResponse(_request.getErrorCode());
        return;
    }
    
    // 정상 응답 준비
    _response.setStatusCode(200);
    _response.setHeader("Content-Type", "text/html");
    _response.setBody("<html><body><h1>Hello World</h1></body></html>");
    
    _response_buf = _response.toString();
}

void Connection::prepareErrorResponse(int error_code) {
    _response.setStatusCode(error_code);
    _response.setHeader("Content-Type", "text/html");
    
    std::string error_body = "<html><body><h1>Error " + 
                           std::to_string(error_code) + 
                           "</h1></body></html>";
    _response.setBody(error_body);
    
    _response_buf = _response.toString();
}

void Connection::cleanUp() {
    if (_fd >= 0) {
        close(_fd);
        _fd = -1;
    }
    
    _parser.getRawBuffer().clear();
    _response_buf.clear();
    _bytes_sent = 0;
}

void Connection::resetConnection() {
    _request = Request();
    _response = Response();
    _parser.~RequestParser();  // 기존 객체 소멸
    new (&_parser) RequestParser();  // 새 객체 생성
    _response_buf.clear();
    _bytes_sent = 0;
    _progress = READ_CONTINUE;
}

Connection::Connection(const Connection& other) : _request(other._request) { *this = other; }

Connection::~Connection() {}

Connection& Connection::operator=(const Connection& other) {
    if (this != &other) {
        _fd = other.getFd();
        _request = other._request;
        _response = other._response;
        _parser.~RequestParser();  // 기존 객체 소멸
        new (&_parser) RequestParser();  // 새 객체 생성
        _progress = other._progress;
        _response_buf = other._response_buf;
        _bytes_sent = other._bytes_sent;
        _last_request_at = other._last_request_at;
        _client_ip = other._client_ip;
        _client_port = other._client_port;
        _config_ptr = other._config_ptr;
        _server_ptr = other._server_ptr;
        _location_ptr = other._location_ptr;
    }
    return *this;
}

void Connection::processRequest() {
    // 요청 처리 로직
    if (_request.hasError()) {
        prepareErrorResponse(_request.getErrorCode());
        _progress = TO_CLIENT;
        return;
    }

    // HTTP 메서드에 따른 처리
    switch (_request.getMethod()) {
        case GET:
            // GET 요청 처리
            prepareResponse();
            break;
        case POST:
            // POST 요청 처리
            prepareResponse();
            break;
        case DELETE:
            // DELETE 요청 처리
            prepareResponse();
            break;
        default:
            // 지원하지 않는 메서드
            prepareErrorResponse(405);  // Method Not Allowed
            break;
    }

    _progress = TO_CLIENT;
}

int Connection::getFd() const {
    return _fd;
}