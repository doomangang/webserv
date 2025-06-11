#include "../inc/Client.hpp"
#include <cstring>

void	Client::readAndParse() {
	char buf[MESSAGE_BUFFER];
	memset(buf, 0, MESSAGE_BUFFER);

    ssize_t n = recv(_fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        if (n < 0)
			request.setErrorCode(500);
		parser.reset();
		request.setStatus(BAD_REQUEST);
        return ;
    }

	setLastRequestAt();
    parser.getRawBuffer().append(buf, n);

    try {
        if (!parser.isRequestLineComplete())
            parser.parseRequestLine(request);
        
        if (parser.isRequestLineComplete() && !parser.isHeadersComplete())
            parser.parseHeaders(request);

        
        if (parser.isHeadersComplete()) {
            if (parser.isChunked() == CHUNKED)
				parser.parseChunkedBody(request);
			else
				parser.parseBody(request);
        }
                
    } catch (const std::exception& e) {
        request.setErrorCode(400);
        request.setStatus(BAD_REQUEST);
    }
	request.setStatus(parser.getParseState());
}

void	Client::findSetConfigs(const std::vector<Server>& servers) {
	if (servers.empty()) {
        server = Server();
        return ;
    }
	
	const Server* matched = &servers[0];
	std::string host = request.getHeaderValue("host");

	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos)
		host = host.substr(0, colon_pos);

	bool found = false;
    for (size_t i = 0; i < servers.size(); ++i) {
        const std::vector<std::string>& server_names = servers[i].getServerNames();
        for (size_t j = 0; j < server_names.size(); ++j) {
            if (server_names[j] == host) {
                matched = &servers[i];
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }

	server = *matched;

	// Location 
}

bool Client::isParseComplete() const {
	ParseState status = parser.getParseState();
	return status == COMPLETE || status == BAD_REQUEST;
}

void Client::processRequest() {
    // 요청 처리 로직
    if (request.hasError()) {
        prepareErrorResponse(request.getErrorCode());
        return;
    }

    // 1. 메서드 검증
    if (!isMethodAllowed()) {
        prepareErrorResponse(405);
        return;
    }

	std::string path = request.getPath();
	const Location& loc = server.getMatchingLocation(path);
    
    // 2. 리다이렉트 체크
    if (loc.hasRedirect()) {
        handleRedirect();
        return;
    }
    
    // 3. CGI 체크
    if (isCGIRequest(loc)) {
        handleCGI();
        return;
    }
    
    // 4. 정적 파일/디렉토리 처리
    std::string file_path = resolveFilePath(loc);
    struct stat file_stat;
    
    if (stat(file_path.c_str(), &file_stat) == 0) {
        if (S_ISDIR(file_stat.st_mode)) {
            handleDirectoryListing(loc, file_path);
        } else if (S_ISREG(file_stat.st_mode)) {
            handleStaticFile(file_path);
        }
		else
			prepareErrorResponse(403);
    } else {
        prepareErrorResponse(404);
    }
}

void Client::prepareErrorResponse(int error_code) {
    response.setStatusCode(error_code);
    response.setHeader("Content-Type", "text/html");
    
    std::string error_body = "<html><body><h1>Error " + 
                           HttpUtils::toString(error_code) + 
                           "</h1></body></html>";
    response.setBody(error_body);
}

bool Client::isMethodAllowed() const
{
    std::string path = request.getPath();
    const Location& location = server.getMatchingLocation(path);
    const std::set<Method>& allowed = location.getAllowMethods();
    
    for (std::set<Method>::const_iterator it = allowed.begin(); it != allowed.end(); ++it) {
        if (*it == *(request.getMethod())) {
            return true;
        }
    }
    return false;
}

void Client::handleRedirect()
{
    std::string path = request.getPath();
    const Location& location = server.getMatchingLocation(path);

    response.setStatusCode(location.getRedirectCode());
    response.setHeader("Location", location.getRedirectUrl());
    response.setBody("");
}

bool Client::isCGIRequest(const Location& location) const {    
    const std::set<std::string>& cgi_exts = location.getCgiExtensions();
	if (cgi_exts.empty())
		return false;
	
	std::string path = request.getPath();
    size_t dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos) 
		return false;
    
    std::string extension = path.substr(dot_pos);
    return cgi_exts.count(extension) > 0;
}

void Client::handleCGI() {response.handleCgi(request);}

std::string Client::resolveFilePath(const Location& location) const {
    std::string path = request.getPath();
    std::string root = "";
    
    if ( !location.getRootPath().empty()) {
        root = location.getRootPath();
    } else if (!server.getRootPath().empty()) {
        root = server.getRootPath();
    }
    
    // Remove location prefix from path
	std::string loc_uri = location.getUri();
	if (path.find(loc_uri) == 0) {
		path = path.substr(loc_uri.length());
	}

    if (!root.empty() && root[root.length() - 1] == '/')
        root.erase(root.length() - 1);

    // Ensure path starts with /
    if (!path.empty() && path[0] != '/') {
        path = "/" + path;
    }
    
    return root + path;
}

void Client::handleStaticFile(const std::string& file_path)
{
    std::ifstream file(file_path.c_str(), std::ios::binary);
    
    if (!file) {
        prepareErrorResponse(403);
        return;
    }
    
    // 파일 크기 확인
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 파일 내용 읽기
    std::string content(static_cast<size_t>(file_size), '\0');
    file.read(&content[0], file_size);
    
    // Response 생성
    response.setStatusCode(200);
    response.setHeader("Content-Type", HttpUtils::getMimeType(file_path));
    response.setHeader("Content-Length", HttpUtils::toString(file_size));
    response.setBody(content);
    
    // Connection 헤더 처리 (keep-alive 또는 close)
    std::string connection_header = request.getHeaderValue("connection");
    if (connection_header == "close") {
        response.setHeader("Connection", "close");
    } else {
        response.setHeader("Connection", "keep-alive");
    }
}

void Client::handleDirectoryListing(const Location& location, const std::string& dir_path)
{
    // 1. Index 파일 찾아보기
    const std::set<std::string>& loc_indexes = location.getIndexFiles();
    const std::vector<std::string>& srv_indexes_vec = server.getIndexFiles();
    const std::set<std::string> srv_indexes(srv_indexes_vec.begin(), srv_indexes_vec.end());

    const std::set<std::string>& index_files = loc_indexes.empty() ? srv_indexes : loc_indexes;

    for (std::set<std::string>::const_iterator it = index_files.begin(); it != index_files.end(); ++it) {
        std::string index_path = dir_path;
        if (index_path[index_path.length() - 1] != '/') {
            index_path += "/";
        }
        index_path += *it;
        
        struct stat file_stat;
        if (stat(index_path.c_str(), &file_stat) == 0) {
            if (S_ISREG(file_stat.st_mode)) {
                handleStaticFile(index_path); // 찾았으면 정적 파일로 처리
                return;
            }
        }
    }

    bool autoindex_on = location.getAutoindex();
    if (location.getAutoindex() == false) { 
        autoindex_on = server.getAutoindex();
    }
    
    if (autoindex_on) {
        prepareAutoindexPage(dir_path);
    }
    else {
        prepareErrorResponse(403); // Forbidden
    }
}

void Client::prepareAutoindexPage(const std::string& dir_path) {
    std::string request_path = request.getPath();
    if (request_path.empty() || request_path[request_path.length() - 1] != '/') {
        request_path += "/";
    }

    std::stringstream html;
    html << "<html>\r\n<head><title>Index of " << request_path << "</title></head>\r\n";
    html << "<body>\r\n<h1>Index of " << request_path << "</h1><hr><pre>\r\n";

    DIR *dir = opendir(dir_path.c_str());
    if (!dir) {
        prepareErrorResponse(500); // Internal Server Error
        return;
    }

    html << "<a href=\"../\">../</a>\r\n";

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;

        std::string full_path = dir_path + "/" + name;
        struct stat st;
        if (stat(full_path.c_str(), &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            html << "<a href=\"" << name << "/\">" << name << "/</a>\r\n";
        } else {
            html << "<a href=\"" << name << "\">" << name << "</a>\r\n";
        }
    }
    closedir(dir);

    html << "</pre><hr></body>\r\n</html>\r\n";

    response.setStatusCode(200);
    response.setHeader("Content-Type", "text/html");
    response.setBody(html.str());
    response.setHeader("Content-Length", HttpUtils::toString(html.str().length()));
}

void Client::clearClient() {
    response.clear();
    request.cleaner();
    parser.reset();
    writer.reset();
}

void Client::updateTime() {
    setLastRequestAt();
}

Client::Client() : _fd(-1), writer(-1) { setLastRequestAt(); }

Client::Client(int client_fd) : _fd(client_fd), writer(client_fd) {
    setLastRequestAt();
}

Client::Client(const Client& other)
    : _fd(other._fd),
      _last_request_at(other._last_request_at),
      response(other.response),
      request(other.request),
      server(other.server),
      parser(other.parser), 
        writer(other._fd)
{}

Client::~Client() {}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        _fd = other._fd;
        _last_request_at = other._last_request_at;
        response = other.response;
        request = other.request;
        server = other.server;
        parser = other.parser;
        writer = other.writer;
    }
    return *this;
}

int Client::getFd() const { return _fd; }
time_t Client::getLastRequestAt() const { return _last_request_at; }
std::string Client::getIp() const { return _ip; }
int Client::getPort() const { return _port; }
Server Client::getServer() const { return server; }
Request& Client::getRequest() { return request; }
Response& Client::getResponse() { return response; }
ResponseWriter& Client::getWriter() { return writer; }
RequestParser& Client::getParser() { return parser; }

void Client::setLastRequestAt() { _last_request_at = time(NULL); }
void Client::setFd(int fd) { _fd = fd; }
void Client::setIp(const std::string& ip) { _ip = ip; }
void Client::setPort(int port) { _port = port; }
void Client::setServer(const Server& server) { this->server = server; }
void Client::setRequest(const Request& request) { this->request = request; }
void Client::setResponse(const Response& response) { this->response = response; }
void Client::setWriter(const ResponseWriter& writer) { this->writer = writer; }
void Client::setParser(const RequestParser& parser) { this->parser = parser; }
