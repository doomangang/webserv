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

/* 소멸자 */
Server::~Server() {
    // 특별히 해제해야 할 리소스가 있으면 여기에 추가
}

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

// server_names
std::vector<std::string> Server::getServerNames() const { return _server_names; }
void Server::setServerNames(const std::vector<std::string>& serverNames){ _server_names = serverNames; }
void Server::addServerName(const std::string& name){ _server_names.push_back(name); }

// host
std::string Server::getHost() const { return _host; }
void Server::setHost(const std::string& host){ _host = host; }

// port
int Server::getPort() const { return _port; }
void Server::setPort(int port){ _port = port; }

// fd
int Server::getFd() const { return _fd; }
void Server::setFd(int fd){ _fd = fd; }

// request URI limit size
int Server::getRequestUriLimitSize() const { return _request_uri_limit_size; }
void Server::setRequestUriLimitSize(int size){ _request_uri_limit_size = size; }

// request header limit size
int Server::getRequestHeaderLimitSize() const { return _request_header_limit_size; }
void Server::setRequestHeaderLimitSize(int size){ _request_header_limit_size = size; }

// client body limit size
int Server::getLimitClientBodySize() const { return _limit_client_body_size; }
void Server::setLimitClientBodySize(int size){ _limit_client_body_size = size; }

// root_path
std::string Server::getRootPath() const { return _root_path; }
void Server::setRootPath(const std::string& path){ _root_path = path; }

// index_files
std::vector<std::string> Server::getIndexFiles() const { return _index_files; }
void Server::setIndexFiles(const std::vector<std::string>& files){ _index_files = files; }
void Server::addLocation(const Location& loc){ _locations.push_back(loc); }

// autoindex
bool Server::getAutoindex() const { return _autoindex; }
void Server::setAutoindex(bool onoff){ _autoindex = onoff; }

// upload_store 플래그
bool Server::hasUploadStore() const { return _has_upload_store; }
void Server::setHasUploadStore(bool has){ _has_upload_store = has; }

// upload_store 경로
std::string Server::getUploadStore() const { return _upload_store; }
void Server::setUploadStore(const std::string& path){ _upload_store = path;
    _has_upload_store = true; }

// default_error_page
std::string Server::getDefaultErrorPage() const { return _default_error_page; }
void Server::setDefaultErrorPage(const std::string& page){ _default_error_page = page; }

// error_pages
void Server::addErrorPage(int code, const std::string& path){ _error_pages[code] = path; }
std::string Server::getErrorPage(int code) const {
    std::map<int, std::string>::const_iterator it = _error_pages.find(code);
    if (it != _error_pages.end()) {
        return it->second;
    }
    return _default_error_page; 
}
std::map<int, std::string> Server::getErrorPages() const { return _error_pages; }

// config 포인터
Config* Server::getConfig() const { return _config; }
void Server::setConfig(Config* config){ _config = config; }

// locations 벡터
const std::vector<Location>& Server::getLocations() const { return _locations; }
void Server::setLocations(const std::vector<Location>& locations){ _locations = locations; }

// manager 포인터
ServerManager* Server::getManager() const { return _manager; }
void Server::setManager(ServerManager* manager){ _manager = manager; }
