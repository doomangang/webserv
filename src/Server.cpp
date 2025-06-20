// Server.cpp

#include "../inc/Server.hpp"

/* OCF: 기본 생성자 */
Server::Server()
    : _manager(NULL),
      _server_names(),
      _host(""),
      _port(0),
      _fd(-1),
      _request_uri_limit_size(0),
      _request_header_limit_size(0),
      _limit_client_body_size(0),
      _root_path(""),
      _index_files(),
      _autoindex(false),
      _upload_store(""),
      _has_upload_store(false),
      _default_error_page(""),
      _error_pages(),
      _config(NULL),
      _locations()
{
}

/* 파라미터 생성자 */
Server::Server(ServerManager* manager,
               const std::string& server_block,
               const std::string& location_blocks,
               Config* config)
    : _manager(manager),
      _server_names(),
      _host(""),
      _port(0),
      _fd(-1),
      _request_uri_limit_size(0),
      _request_header_limit_size(0),
      _limit_client_body_size(0),
      _root_path(""),
      _index_files(),
      _autoindex(false),
      _upload_store(""),
      _has_upload_store(false),
      _default_error_page(""),
      _error_pages(),
      _config(config),
      _locations()
{
    (void)server_block;
    (void)location_blocks;
    // TODO: server_block과 location_blocks 파싱 로직을 여기에 추가
}

/* 복사 생성자 */
Server::Server(const Server& other)
    : _manager(other._manager),
      _server_names(other._server_names),
      _host(other._host),
      _port(other._port),
      _fd(other._fd),
      _request_uri_limit_size(other._request_uri_limit_size),
      _request_header_limit_size(other._request_header_limit_size),
      _limit_client_body_size(other._limit_client_body_size),
      _root_path(other._root_path),
      _index_files(other._index_files),
      _autoindex(other._autoindex),
      _upload_store(other._upload_store),
      _has_upload_store(other._has_upload_store),
      _default_error_page(other._default_error_page),
      _error_pages(other._error_pages),
      _config(other._config),
      _locations(other._locations)
{
}

Server::~Server() {}

/* 대입 연산자 */
Server& Server::operator=(const Server& other) {
    if (this != &other) {
        _manager                    = other._manager;
        _server_names               = other._server_names;
        _host                       = other._host;
        _port                       = other._port;
        _fd                         = other._fd;
        _request_uri_limit_size     = other._request_uri_limit_size;
        _request_header_limit_size  = other._request_header_limit_size;
        _limit_client_body_size     = other._limit_client_body_size;
        _root_path                  = other._root_path;
        _index_files                = other._index_files;
        _autoindex                  = other._autoindex;
        _upload_store               = other._upload_store;
        _has_upload_store           = other._has_upload_store;
        _default_error_page         = other._default_error_page;
        _error_pages                = other._error_pages;
        _config                     = other._config;
        _locations                  = other._locations;
    }
    return *this; }

/* getter & setter */

void Server::setupServer() {
    int opt = 1;
    struct sockaddr_in serv_addr;

    Logger::logMsg(INFO, CONSOLE_OUTPUT, "Setting up server on %s:%d", _host.c_str(), _port);

    // Create socket
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1) {
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "socket() failed: %s", strerror(errno));
        throw std::runtime_error("socket() failed");
    }
    Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Created socket fd: %d", _fd);

    // Set socket options to reuse address and port
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "setsockopt(SO_REUSEADDR) failed: %s", strerror(errno));
        close(_fd);
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

#ifdef SO_REUSEPORT
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        Logger::logMsg(INFO, CONSOLE_OUTPUT, "setsockopt(SO_REUSEPORT) not available: %s", strerror(errno));
        // SO_REUSEPORT 실패는 치명적이지 않으므로 계속 진행
    }
#endif

    // Set server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    
    if (_host == "0.0.0.0" || _host.empty()) {
        serv_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_aton(_host.c_str(), &serv_addr.sin_addr) == 0) {
            Logger::logMsg(ERROR, CONSOLE_OUTPUT, "Invalid host address: %s", _host.c_str());
            close(_fd);
            throw std::runtime_error("Invalid host address");
        }
        Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Binding to address %s", _host.c_str());
    }
    
    serv_addr.sin_port = htons(_port);

    Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Attempting to bind socket %d to %s:%d", 
                   _fd, _host.c_str(), _port);

    // Bind the socket to the address
    if (bind(_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        int saved_errno = errno;
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "bind() failed for %s:%d: %s (errno=%d)", 
                      _host.c_str(), _port, strerror(saved_errno), saved_errno);
        close(_fd);
        
        if (saved_errno == EADDRINUSE) {
            throw std::runtime_error("Address already in use");
        } else if (saved_errno == EACCES) {
            throw std::runtime_error("Permission denied (need root for ports < 1024?)");
        } else {
            throw std::runtime_error("bind() failed");
        }
    }
    
    Logger::logMsg(INFO, CONSOLE_OUTPUT, "Server successfully bound to %s:%d (fd: %d)", 
                   _host.c_str(), _port, _fd);
}

// // server_names
std::vector<std::string> Server::getServerNames() const { return _server_names; }
void Server::setServerNames(const std::vector<std::string>& serverNames){ _server_names = serverNames; }
void Server::addServerName(const std::string& name){ _server_names.push_back(name); }

// // host
// std::string Server::getHost() const { return _host; }
// void Server::setHost(const std::string& host){ _host = host; }

// // port
// int Server::getPort() const { return _port; }
// void Server::setPort(int port){ _port = port; }

// // fd
// int Server::getFd() const { return _fd; }
// void Server::setFd(int fd){ _fd = fd; }

// // request URI limit size
// int Server::getRequestUriLimitSize() const { return _request_uri_limit_size; }
// void Server::setRequestUriLimitSize(int size){ _request_uri_limit_size = size; }

// // request header limit size
// int Server::getRequestHeaderLimitSize() const { return _request_header_limit_size; }
// void Server::setRequestHeaderLimitSize(int size){ _request_header_limit_size = size; }

// // client body limit size
// int Server::getLimitClientBodySize() const { return _limit_client_body_size; }
// void Server::setLimitClientBodySize(int size){ _limit_client_body_size = size; }

// // root_path
// std::string Server::getRootPath() const { return _root_path; }
// void Server::setRootPath(const std::string& path){ _root_path = path; }

// // index_files
// std::vector<std::string> Server::getIndexFiles() const { return _index_files; }
// void Server::setIndexFiles(const std::vector<std::string>& files){ _index_files = files; }
void Server::addLocation(const Location& loc){ _locations.push_back(loc); }

// // autoindex
// bool Server::getAutoindex() const { return _autoindex; }
// void Server::setAutoindex(bool onoff){ _autoindex = onoff; }

// // upload_store 플래그
// bool Server::hasUploadStore() const { return _has_upload_store; }
// void Server::setHasUploadStore(bool has){ _has_upload_store = has; }

// // upload_store 경로
// std::string Server::getUploadStore() const { return _upload_store; }
// void Server::setUploadStore(const std::string& path){ _upload_store = path;
//     _has_upload_store = true; }

// // default_error_page
// // std::string Server::getDefaultErrorPage() const { return _default_error_page; }
// void Server::setDefaultErrorPage(const std::string& page){ _default_error_page = page; }

// error_pages
void Server::addErrorPage(int code, const std::string& path){ _error_pages[code] = path; }
std::string Server::getErrorPage(int code) const {
    std::map<int, std::string>::const_iterator it = _error_pages.find(code);
    if (it != _error_pages.end()) {
        return it->second;
    }
    return _default_error_page; 
}
// std::map<int, std::string> Server::getErrorPages() const { return _error_pages; }

// // config 포인터
// Config* Server::getConfig() const { return _config; }
// void Server::setConfig(Config* config){ _config = config; }

// // locations 벡터
// const std::vector<Location>& Server::getLocations() const { return _locations; }
// void Server::setLocations(const std::vector<Location>& locations){ _locations = locations; }

// // manager 포인터
ServerManager* Server::getManager() const { return _manager; }
void Server::setManager(ServerManager* manager){ _manager = manager; }

const Location& Server::getMatchingLocation(std::string& uri) const {
    // 가장 긴 매칭 prefix 찾기
    size_t best_match_len = 0;
    size_t best_match_idx = 0;
    
    for (size_t i = 0; i < _locations.size(); ++i) {
        const std::string& loc_uri = _locations[i].getUri();
        if (uri.find(loc_uri) == 0 && loc_uri.length() > best_match_len) {
            best_match_idx = i;
            best_match_len = loc_uri.length();
        }
    }
    
    if (best_match_len > 0) {
        return _locations[best_match_idx];
    }

    return getDefaultLocation();
}

const Location& Server::getDefaultLocation() const{
    if (_default_location.getRootPath().empty() && !_root_path.empty()) {
        _default_location.setRootPath(_root_path);
        
        // 기본 메서드 설정
        std::set<Method> default_methods;
        default_methods.insert(GET);
        default_methods.insert(POST);
        default_methods.insert(DELETE);
        _default_location.setAllowMethods(default_methods);
        
        // 기본 인덱스 파일 설정
        if (!_index_files.empty()) {
            _default_location.setIndexFiles(_index_files);
        }
        
        _default_location.setAutoindex(_autoindex);
    }
    
    return _default_location;
}

// Getter
// const std::string& Server::getServerNames() const { return _server_names; }
const std::string& Server::getHost() const { return _host; }
int Server::getPort() const { return _port; }
int Server::getFd() const { return _fd; }
int Server::getRequestUriLimitSize() const { return _request_uri_limit_size; }
int Server::getRequestHeaderLimitSize() const { return _request_header_limit_size; }
int Server::getLimitClientBodySize() const { return _limit_client_body_size; }
const std::string& Server::getDefaultErrorPage() const { return _default_error_page; }
Config* Server::getConfig() const { return _config; }
const std::vector<Location>& Server::getLocations() const { return _locations; }
std::string Server::getRootPath() const { return _root_path;}
std::vector<std::string> Server::getIndexFiles() const { return  _index_files;}
bool Server::getAutoindex() const { return _autoindex ; }
std::map<int, std::string> Server::getErrorPages() const { return _error_pages; }
// Setter
// void Server::setServerName(const std::string& name) { _server_name = name; }
void Server::setHost(const std::string& host) { _host = host; }
void Server::setPort(int port) { _port = port; }
void Server::setFd(int fd) { _fd = fd; }
void Server::setRequestUriLimitSize(int size) { _request_uri_limit_size = size; }
void Server::setRequestHeaderLimitSize(int size) { _request_header_limit_size = size; }
void Server::setLimitClientBodySize(int size) { _limit_client_body_size = size; }
void Server::setDefaultErrorPage(const std::string& page) { _default_error_page = page; }
void Server::setConfig(Config* config) { _config = config; }
void Server::setLocations(const std::vector<Location>& locations) { _locations = locations; }
void Server::setAutoindex(bool onoff) { _autoindex = onoff; }

bool Server::hasUploadStore() const { return _has_upload_store; }
std::string Server::getUploadStore() const { return _upload_store; }
void Server::setHasUploadStore(bool has) { _has_upload_store = has; }
void Server::setUploadStore(const std::string& path) { 
    _upload_store = path;
    _has_upload_store = true; 
}

// 누락된 메서드들 구현
void Server::setRootPath(const std::string& path) {
    _root_path = path;
}

void Server::setIndexFiles(const std::vector<std::string>& files) {
    _index_files = files;
}

std::string Server::resolveRootPath(const Location& location) const {
    if (!location.getRootPath().empty()) {
        return location.getRootPath();
    }
    
    if (!_root_path.empty()) {
        return _root_path;
    }
    
    throw std::runtime_error("No root path available for location: " + location.getUri());
}