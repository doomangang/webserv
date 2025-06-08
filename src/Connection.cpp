#include "../inc/Connection.hpp"

#include "../inc/Connection.hpp"

Connection::Connection() 
    : _fd(-1), 
      _request(), 
      _parser(),
      _progress(FROM_CLIENT),
      _bytes_sent(0),
      _client_port(0), 
      _config_ptr(NULL), 
      _server_ptr(NULL), 
      _location_ptr(NULL),
      _response()      {
    timeval tv = {0, 0};
    _last_request_at = tv;
}

Connection::Connection(int client_fd, const std::string& client_ip, int client_port)
    : _fd(client_fd), 
      _request(), 
      _parser(),
      _progress(FROM_CLIENT),
      _bytes_sent(0), 
      _client_ip(client_ip), 
      _client_port(client_port),
      _config_ptr(NULL), 
      _server_ptr(NULL),
      _location_ptr(NULL),
      _response() {
    timeval tv = {0, 0};
    _last_request_at = tv;
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
        _parser.setMaxHeaderSize(_server_ptr->getRequestHeaderLimitSize());
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

// Connection.cpp - processRequest 메서드 확장
void Connection::processRequest() {
    // 요청 처리 로직
    if (_request.hasError()) {
        prepareErrorResponse(_request.getErrorCode());
        _progress = TO_CLIENT;
        return;
    }

    // 1. 메서드 검증
    if (!isMethodAllowed()) {
        prepareErrorResponse(405);
        _progress = TO_CLIENT;
        return;
    }
    
    // 2. 리다이렉트 체크
    if (_location_ptr && _location_ptr->hasRedirect()) {
        handleRedirect();
        _progress = TO_CLIENT;
        return;
    }
    
    // 3. CGI 체크
    if (isCGIRequest()) {
        handleCGI();
        _progress = TO_CLIENT;
        return;
    }
    
    // 4. 정적 파일/디렉토리 처리
    std::string file_path = resolveFilePath();
    struct stat file_stat;
    
    if (stat(file_path.c_str(), &file_stat) == 0) {
        if (S_ISDIR(file_stat.st_mode)) {
            handleDirectoryListing();
        } else if (S_ISREG(file_stat.st_mode)) {
            handleStaticFile();
        }
    } else {
        prepareErrorResponse(404);
    }

    _progress = TO_CLIENT;
}

// 새로 추가할 메서드들
bool Connection::isMethodAllowed() const {
    if (!_location_ptr) return false;
    
    std::set<Method> allowed = _location_ptr->getAllowMethods();
    return allowed.find(_request.getMethod()) != allowed.end();
}

bool Connection::isCGIRequest() const {
    if (!_location_ptr) return false;
    
    std::string path = _request.getPath();
    size_t dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos) return false;
    
    std::string extension = path.substr(dot_pos);
    std::set<std::string> cgi_exts = _location_ptr->getCgiExtensions();
    
    return cgi_exts.find(extension) != cgi_exts.end();
}

std::string Connection::resolveFilePath() const {
    std::string path = _request.getPath();
    std::string root = "";
    
    if (_location_ptr && !_location_ptr->getRootPath().empty()) {
        root = _location_ptr->getRootPath();
    } else if (_server_ptr && !_server_ptr->getRootPath().empty()) {
        root = _server_ptr->getRootPath();
    }
    
    // Remove location prefix from path
    if (_location_ptr) {
        std::string loc_uri = _location_ptr->getUri();
        if (path.find(loc_uri) == 0) {
            path = path.substr(loc_uri.length());
        }
    }
    
    // Ensure path starts with /
    if (!path.empty() && path[0] != '/') {
        path = "/" + path;
    }
    
    return root + path;
}

void Connection::handleStaticFile() {
    std::string file_path = resolveFilePath();
    std::ifstream file(file_path.c_str(), std::ios::binary);
    
    if (!file) {
        prepareErrorResponse(404);
        return;
    }
    
    // 파일 크기 확인
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 파일 내용 읽기
    std::string content(file_size, '\0');
    file.read(&content[0], file_size);
    
    // Response 생성
    _response.setStatusCode(200);
    _response.setHeader("Content-Type", getMimeType(file_path));
    _response.setHeader("Content-Length", HttpUtils::getMimeType(file_size));
    _response.setBody(content);
    
    // Connection 헤더 처리
    std::string connection = _request.getHeaderValue("connection");
    if (connection == "close") {
        _response.setHeader("Connection", "close");
    } else {
        _response.setHeader("Connection", "keep-alive");
    }
    
    _response_buf = _response.toString();
}

void Connection::handleDirectoryListing() {
    // TODO: 디렉토리 리스팅 구현
    prepareErrorResponse(403); // 일단 Forbidden
}

void Connection::handleCGI() {
    // TODO: CGI 처리 구현
    prepareErrorResponse(501); // Not Implemented
}

void Connection::handleRedirect() {
    if (!_location_ptr) return;
    
    _response.setStatusCode(_location_ptr->getRedirectCode());
    _response.setHeader("Location", _location_ptr->getRedirectUrl());
    _response.setBody("");
    
    _response_buf = _response.toString();
}

std::string Connection::getMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = path.substr(dot + 1);
    // C++98에서는 transform 사용
    for (size_t i = 0; i < ext.length(); ++i) {
        ext[i] = std::tolower(ext[i]);
    }
    
    // C++98에서는 초기화 리스트 사용 불가, 수동으로 초기화
    if (ext == "html" || ext == "htm") return "text/html";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "json") return "application/json";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "png") return "image/png";
    if (ext == "gif") return "image/gif";
    if (ext == "txt") return "text/plain";
    if (ext == "pdf") return "application/pdf";
    
    return "application/octet-stream";
}

void Connection::writeClient() {
    if (_response_buf.empty()) {
        prepareResponse();
    }
    
    size_t to_send = _response_buf.size() - _bytes_sent;
    ssize_t sent = send(_fd, _response_buf.c_str() + _bytes_sent, to_send, 0);
    
    if (sent > 0) {
        _bytes_sent += sent;
        if (_bytes_sent >= _response_buf.size()) {
            // 전송 완료
            std::string conn_header = _response.getHeaderValue("Connection");
            if (conn_header == "close") {
                _progress = END_CONNECTION;
            } else {
                // keep-alive: 다음 요청 대기
                resetConnection();
            }
        }
    } else if (sent < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            _progress = END_CONNECTION;
        }
    }
}

void Connection::resetConnection() {
    _request.cleaner();
    _response = Response();
    _parser = RequestParser();
    _response_buf.clear();
    _bytes_sent = 0;
    _progress = FROM_CLIENT;
}


int Connection::getFd() const {
    return _fd;
}

timeval Connection::getLastRequestAt() const {
    return _last_request_at;
}

std::string Connection::getClientIp() const {
    return _client_ip;
}

int Connection::getClientPort() const {
    return _client_port;
}

Progress Connection::getProgress() const {
    return _progress;
}

void Connection::setLastRequestAt(const timeval& tv) {
    _last_request_at = tv;
}

bool Connection::isComplete() const {
    return _progress == END_CONNECTION;
}

bool Connection::needsRead() const {
    return _progress == FROM_CLIENT || _progress == READ_CONTINUE;
}

bool Connection::needsWrite() const {
    return _progress == TO_CLIENT;
}
