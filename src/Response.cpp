#include "../inc/Response.hpp"

Response::Response()
    : _body_length(0), _code(200), _res(NULL), _cgi(0), _cgi_response_length(0), _auto_index(false)
{}

Response::Response(Request& req)
    : request(req), _body_length(0), _code(200), _res(NULL), _cgi(0), _cgi_response_length(0), _auto_index(false)
{}

Response::~Response() {
    if (_res)
        delete[] _res;
#include "../inc/Connection.hpp"

Response::Response() 
    : _connection(NULL),
      _status_code(200),
      _status_description("OK"),
      _transfer_type(GENERAL),
      _content("") {
}

std::string Response::getRes() {
    return _response_content;
}

size_t Response::getLen() const {
    return _body_length;
}

int Response::getCode() const {
    return _code;
}

void Response::setRequest(Request &req) {
    request = req;
}

void Response::setServer(Server &server) {
    _server = server;
}

void Response::buildResponse() {
    // TODO: 실제 응답 빌드 로직 구현
}

void Response::clear() {
    _response_content.clear();
    _body.clear();
    _body_length = 0;
    _code = 200;
    _location.clear();
    _response_body.clear();
    if (_res) {
        delete[] _res;
        _res = NULL;
    }
    _cgi = 0;
    _cgi_response_length = 0;
    _auto_index = false;
}

void Response::handleCgi(Request& req) {
    // TODO: CGI 처리 로직 구현
}

void Response::cutRes(size_t len) {
    if (len < _response_content.size())
        _response_content = _response_content.substr(len);
    else
        _response_content.clear();
}

int Response::getCgiState() {
    return _cgi;
}

void Response::setCgiState(int state) {
    _cgi = state;
Response::Response(Connection* conn, int status_code, const std::string& body) 
    : _connection(conn),
      _status_code(status_code),
      _status_description(""),
      _transfer_type(GENERAL),
      _content(body) {
    makeStatus(status_code);
}

Response::Response(const Response& other) {
    *this = other;
    std::cout << GREEN << "Response copy constructor called\n" << RESET << std::endl;
}

void Response::setErrorResponse(short code) {
    _code = code;
    buildErrorBody();
}

std::string Response::removeBoundary(std::string &body, std::string &boundary) {
    // TODO: 멀티파트 바운더리 제거 로직 구현
    return body;
}

// --- Private methods ---

int Response::buildBody() {
    // TODO: 바디 빌드 로직 구현
    return 0;
}

size_t Response::file_size() {
    // TODO: 파일 크기 반환
    return 0;
}

Response& Response::operator=(const Response& other) {
    std::cout << YELLOW << "Response assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        _connection = other._connection;
        _status_code = other._status_code;
        _status_description = other._status_description;
        _headers = other._headers;
        _transfer_type = other._transfer_type;
        _content = other._content;
    }
    return *this;
void Response::setStatusLine() {
    // TODO: 상태 라인 설정
}

void Response::setHeaders() {
    // TODO: 헤더 설정
}

void Response::setServerDefaultErrorPages() {
    // TODO: 기본 에러 페이지 설정
}

int Response::readFile() {
    // TODO: 파일 읽기
    return 0;
}

void Response::contentType() {
    // TODO: Content-Type 설정
}

void Response::contentLength() {
    // TODO: Content-Length 설정
}

void Response::connection() {
    // TODO: Connection 헤더 설정
}

void Response::server() {
    // TODO: Server 헤더 설정
}

void Response::location() {
    // TODO: Location 헤더 설정
}

void Response::date() {
    // TODO: Date 헤더 설정
}

int Response::handleTarget() {
    // TODO: 요청 타겟 처리
    return 0;
}

void Response::buildErrorBody() {
    // TODO: 에러 바디 빌드
}

bool Response::reqError() {
    // TODO: 요청 에러 체크
    return false;
}

int Response::handleCgi(std::string &) {
    // TODO: CGI 처리
    return 0;
}

int Response::handleCgiTemp(std::string &) {
    // TODO: 임시 CGI 처리
    return 0;
}

void Response::setStatusCode(int code) {
    _status_code = code;
    makeStatus(code);
}

void Response::setHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}

void Response::setBody(const std::string& body) {
    _content = body;
    _headers["Content-Length"] = HttpUtils::toString(body.size());
}

std::string Response::toString() const {
    std::ostringstream oss;
    
    // Status line
    oss << "HTTP/1.1 " << _status_code << " " << _status_description << "\r\n";
    
    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
         it != _headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    
    // Empty line
    oss << "\r\n";
    
    // Body
    oss << _content;
    
    return oss.str();
}

Connection* Response::getConnection() const {
    return _connection;
}

int Response::getStatusCode() const {
    return _status_code;
}

std::string Response::getStatusDescription() const {
    return _status_description;
}

std::map<std::string, std::string> Response::getHeaders() const {
    return _headers;
}

Response::TransferType Response::getTransferType() const {
    return _transfer_type;
}

std::string Response::getContent() const {
    return _content;
}

std::string Response::getHeaderValue(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

void Response::addHeader(const std::string& key, const std::string& value) {
    _headers[key] = value;
}