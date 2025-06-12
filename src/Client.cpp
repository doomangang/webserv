#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/CgiHandler.hpp"
#include <cstring>

void	Client::readAndParse() {
	char buf[MESSAGE_BUFFER];
	memset(buf, 0, MESSAGE_BUFFER);

	Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Attempting to read from fd %d", _fd);

    ssize_t n = recv(_fd, buf, sizeof(buf), 0);
    
    Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "recv() returned %zd for fd %d", n, _fd);
    
    if (n <= 0) {
        if (n == 0) {
            Logger::logMsg(INFO, CONSOLE_OUTPUT, "Client fd %d closed connection normally", _fd);
            parser.reset();
            setConnectionState(END_CONNECTION); 
            return;
        }
        
        int saved_errno = errno;
        if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "recv() would block on fd %d (EAGAIN/EWOULDBLOCK)", _fd);
            setConnectionState(READ_CONTINUE);
            return;
        }
        
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "recv() failed on fd %d: %s (errno=%d)", 
                      _fd, strerror(saved_errno), saved_errno);
        request.setErrorCode(500);
        parser.reset();
        request.setStatus(BAD_REQUEST);
        setConnectionState(END_CONNECTION);
        return;
    }

    Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Successfully received %zd bytes from fd %d", n, _fd);
    setConnectionState(FROM_CLIENT);
    
    std::string preview(buf, std::min((size_t)n, (size_t)100));
    for (size_t i = 0; i < preview.length(); ++i) {
        if (preview[i] < 32 || preview[i] > 126) {
            preview[i] = '.';
        }
    }
    Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Data preview: [%s%s]", 
                  preview.c_str(), n > 100 ? "..." : "");
    
    setLastRequestAt();
    parser.getRawBuffer().append(buf, n);

    try {
        Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Starting parse process for fd %d", _fd);
        
        if (!parser.isRequestLineComplete()) {
            Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Parsing request line for fd %d", _fd);
            parser.parseRequestLine(request);
        }
        
        if (parser.isRequestLineComplete() && !parser.isHeadersComplete()) {
            Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Parsing headers for fd %d", _fd);
            parser.parseHeaders(request);
        }

        if (parser.isHeadersComplete()) {
            Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Parsing body for fd %d", _fd);
            if (parser.isChunked() == CHUNKED)
                parser.parseChunkedBody(request);
            else
                parser.parseBody(request);
        }
        
        Logger::logMsg(DEBUG, CONSOLE_OUTPUT, "Parse state for fd %d: %d", _fd, parser.getParseState());
        
        if (parser.getParseState() == COMPLETE) {
            setConnectionState(TO_CLIENT);
        } else if (parser.getParseState() == BAD_REQUEST) {
            // BAD_REQUEST인 경우 에러 응답을 보낸 후 연결 종료
            if (!request.hasError()) {
                request.setErrorCode(400); // 기본 Bad Request
            }
            setConnectionState(TO_CLIENT); // 에러 응답을 보내기 위해 TO_CLIENT로 설정
        } else {
            setConnectionState(READ_CONTINUE);
        }
                
    } catch (const std::exception& e) {
        Logger::logMsg(ERROR, CONSOLE_OUTPUT, "Parse error on fd %d: %s", _fd, e.what());
        request.setErrorCode(400);
        request.setStatus(BAD_REQUEST);
        setConnectionState(END_CONNECTION);  // 파싱 에러로 인한 종료
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
    // Parse 에러가 있거나 request 에러가 있는 경우
    if (parser.getParseState() == BAD_REQUEST || request.hasError()) {
        int error_code = request.getErrorCode();
        if (error_code == 0) error_code = 400; // 기본 Bad Request
        prepareErrorResponse(error_code);
        return;
    }

    // 1. 잘못된 메서드 체크 (UNKNOWN_METHOD인 경우)
    if (request.getMethod() == UNKNOWN_METHOD) {
        prepareErrorResponse(405); // Method Not Allowed
        return;
    }

    // 2. 메서드 허용 여부 체크
    if (!isMethodAllowed()) {
        prepareErrorResponse(405); // Method Not Allowed
        return;
    }

	std::string path = request.getPath();
	const Location& loc = server.getMatchingLocation(path);
    
    // 3. 리다이렉트 체크
    if (loc.hasRedirect()) {
        handleRedirect();
        return;
    }
    
    // 4. CGI 체크
    if (isCGIRequest(loc)) {
        handleCGI();
        return;
    }
    
    // 5. 메서드별 처리
    Method method = request.getMethod();
    
    if (method == GET) {
        handleGetRequest(loc);
    } else if (method == POST) {
        handlePostRequest(loc);
    } else if (method == DELETE) {
        handleDeleteRequest(loc);
    } else {
        prepareErrorResponse(405); // 이론적으로는 여기에 도달하지 않아야 함
    }
}

void Client::prepareErrorResponse(int error_code) {
    response.setStatusCode(error_code);
    response.setHeader("Content-Type", "text/html");
    
    std::string error_body = "<html><body><h1>Error " + 
                           HttpUtils::toString(error_code) + 
                           "</h1></body></html>";
    response.setBody(error_body);
    response.setHeader("Content-Length", HttpUtils::toString(error_body.length()));
}

bool Client::isMethodAllowed() const
{
    std::string path = request.getPath();
    const Location& location = server.getMatchingLocation(path);
    const std::set<Method>& allowed = location.getAllowMethods();
    
    for (std::set<Method>::const_iterator it = allowed.begin(); it != allowed.end(); ++it) {
        if (*it == request.getMethod()) {
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
    response.setHeader("Content-Length", "0");
}

bool Client::isCGIRequest(const Location& location) const {    
    const std::set<std::string>& cgi_exts = location.getCgiExtensions();
    
	std::string path = request.getPath();
	if (cgi_exts.empty()) {
		return false;
    }
	
    size_t dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos) {
		return false;
    }
    
    std::string extension = path.substr(dot_pos);
    std::cout << "[DEBUG] Found extension: " << extension << std::endl;
    
    bool is_cgi = cgi_exts.count(extension) > 0;
    std::cout << "[DEBUG] Is CGI request: " << (is_cgi ? "YES" : "NO") << std::endl;
    
    return is_cgi;
}

void Client::handleCGI() {
    std::string path = request.getPath();
    const Location& location = server.getMatchingLocation(path);

    std::cout << "[DEBUG] handleCGI() - Original path: " << path << std::endl;
    std::cout << "[DEBUG] handleCGI() - Location URI: " << location.getUri() << std::endl;
    std::cout << "[DEBUG] handleCGI() - Location root: " << location.getRootPath() << std::endl;
    std::cout << "[DEBUG] handleCGI() - Server root: " << server.getRootPath() << std::endl;

    response.setCgiState(1);

    std::string file_path = resolveFilePath(location);
    std::cout << "[DEBUG] handleCGI() - Resolved file path: " << file_path << std::endl;
    
    response._cgi_obj.setCgiPath(file_path);

    // 실제 location iterator 구현 (iterator 타입 일치)
    std::vector<Location>& locations = const_cast<std::vector<Location>&>(server.getLocations());
    std::vector<Location>::iterator location_iterator = locations.end();
    for (std::vector<Location>::iterator it = locations.begin(); it != locations.end(); ++it) {
        if (&(*it) == &location) {
            location_iterator = it;
            break;
        }
    }
    if (location_iterator != locations.end()) {
        response._cgi_obj.initEnv(request, location_iterator);
    } else {
        // fallback: 첫 location 사용 (혹은 적절한 예외 처리)
        response._cgi_obj.initEnv(request, locations.begin());
    }
    
    short error_code = 0;
    response._cgi_obj.execute(error_code);
    
    if (error_code != 0) {
        std::cout << "[DEBUG] CGI execute failed with error: " << error_code << std::endl;
        response.setCgiState(2); // CGI 실패
        prepareErrorResponse(error_code);
    } else {
        std::cout << "[DEBUG] CGI execute succeeded, CGI state should be 1" << std::endl;
        // CGI started successfully, state should already be 1
        // Server event loop will handle reading from CGI pipes
    }
}


std::string Client::resolveFilePath(const Location& location) const {
    std::string path = request.getPath();
    std::string root = "";
    
    // CGI 요청에 대한 특별한 처리
    if (location.getUri() == "/cgi-bin") {
        std::cout << "[DEBUG] resolveFilePath() - CGI request detected" << std::endl;
        // CGI 파일들은 ./www/cgi-bin/ 에 있음
        return "./www" + path;
    }
    
    // Upload location에 대한 특별한 처리
    if (location.hasUploadStore()) {
        std::cout << "[DEBUG] resolveFilePath() - Upload request detected" << std::endl;
        std::string upload_dir = location.getUploadStore();
        std::string loc_uri = location.getUri();
        if (path.find(loc_uri) == 0) {
            path = path.substr(loc_uri.length());
        }
        if (!path.empty() && path[0] != '/') {
            path = "/" + path;
        }
        return upload_dir + path;
    }
    
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

Client::Client() : _fd(-1), _connection_state(FROM_CLIENT), writer(-1) { setLastRequestAt(); }

Client::Client(int client_fd) : _fd(client_fd), _connection_state(FROM_CLIENT), writer(client_fd) {
    setLastRequestAt();
}



Client::~Client() {}
Client::Client(const Client& other)
    : _fd(other._fd),
      _last_request_at(other._last_request_at),
      _ip(other._ip),
      _port(other._port),
      _connection_state(other._connection_state),  // 추가
      response(other.response),
      request(other.request),
      server(other.server),
      parser(other.parser), 
      writer(other.writer)
{
    writer.setFd(_fd);
}

Client& Client::operator=(const Client& other)
{
    if (this != &other)
    {
        _fd = other._fd;
        _last_request_at = other._last_request_at;
        _ip = other._ip;
        _port = other._port;
        _connection_state = other._connection_state;  // 추가
        response = other.response;
        request = other.request;
        server = other.server;
        parser = other.parser;
        writer = other.writer;
        
        writer.setFd(_fd);
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
ConnectionState Client::getConnectionState() const { return _connection_state; }

void Client::setLastRequestAt() { _last_request_at = time(NULL); }
void Client::setFd(int fd) { _fd = fd; }
void Client::setIp(const std::string& ip) { _ip = ip; }
void Client::setPort(int port) { _port = port; }
void Client::setServer(const Server& server) { this->server = server; }
void Client::setRequest(const Request& request) { this->request = request; }
void Client::setResponse(const Response& response) { this->response = response; }
void Client::setWriter(const ResponseWriter& writer) { this->writer = writer; }
void Client::setParser(const RequestParser& parser) { this->parser = parser; }
void Client::setConnectionState(ConnectionState state) { _connection_state = state; }

// 메서드별 처리 함수들

void Client::handleGetRequest(const Location& loc) {
    // GET 요청: 기존의 정적 파일/디렉토리 처리 로직
    std::string file_path = resolveFilePath(loc);
    struct stat file_stat;
    
    if (stat(file_path.c_str(), &file_stat) == 0) {
        if (S_ISDIR(file_stat.st_mode)) {
            handleDirectoryListing(loc, file_path);
        } else if (S_ISREG(file_stat.st_mode)) {
            handleStaticFile(file_path);
        } else {
            prepareErrorResponse(403);
        }
    } else {
        prepareErrorResponse(404);
    }
}

void Client::handlePostRequest(const Location& loc) {
    // POST 요청: 파일 업로드 또는 데이터 생성
    std::string path = request.getPath();
    std::string body = request.getBody();
    
    std::cout << "[DEBUG] handlePostRequest() - Path: " << path << std::endl;
    std::cout << "[DEBUG] handlePostRequest() - Body length: " << body.length() << std::endl;
    std::cout << "[DEBUG] handlePostRequest() - Body content: [" << body << "]" << std::endl;
    
    // Location에 upload_store가 설정되어 있는지 확인
    if (loc.hasUploadStore()) {
        handleFileUpload(loc);
        return;
    }
    
    // 일반적인 POST 요청: 데이터 생성/수정
    std::string filename = "post_data_" + getCurrentTimestamp() + ".txt";
    std::string file_path = resolveFilePath(loc) + "/" + filename;
    
    // 요청 바디를 파일로 저장
    std::ofstream file(file_path.c_str());
    if (file.is_open()) {
        file << "POST Request Data\n";
        file << "Timestamp: " << getCurrentTimestamp() << "\n";
        file << "Path: " << path << "\n";
        file << "Content-Length: " << body.length() << "\n";
        file << "Body:\n" << body << "\n";
        file.close();
        
        response.setStatusCode(201); // Created
        response.setHeader("Content-Type", "application/json");
        
        std::string success_body = "{\n"
                                  "  \"status\": \"success\",\n"
                                  "  \"message\": \"Data created successfully\",\n"
                                  "  \"method\": \"POST\",\n"
                                  "  \"path\": \"" + path + "\",\n"
                                  "  \"created_file\": \"" + filename + "\",\n"
                                  "  \"timestamp\": \"" + getCurrentTimestamp() + "\"\n"
                                  "}";
        
        response.setBody(success_body);
        response.setHeader("Content-Length", HttpUtils::toString(success_body.length()));
    } else {
        prepareErrorResponse(500); // Internal Server Error
    }
}

void Client::handleDeleteRequest(const Location& loc) {
    // DELETE 요청: 실제 파일 삭제
    std::string path = request.getPath();
    std::string file_path = resolveFilePath(loc);
    
    std::cout << "[DEBUG] handleDeleteRequest() - Request path: " << path << std::endl;
    std::cout << "[DEBUG] handleDeleteRequest() - Resolved file path: " << file_path << std::endl;
    std::cout << "[DEBUG] handleDeleteRequest() - Location URI: " << loc.getUri() << std::endl;
    std::cout << "[DEBUG] handleDeleteRequest() - Location has upload store: " << (loc.hasUploadStore() ? "yes" : "no") << std::endl;
    if (loc.hasUploadStore()) {
        std::cout << "[DEBUG] handleDeleteRequest() - Upload store path: " << loc.getUploadStore() << std::endl;
    }
    
    // 안전 검사: Location 설정에 따른 삭제 허용 여부 확인
    if (!isDeleteAllowedForLocation(loc, file_path)) {
        // 안전하지 않은 경로의 삭제 요청
        response.setStatusCode(403); // Forbidden
        response.setHeader("Content-Type", "application/json");
        
        std::string error_body = "{\n"
                                "  \"status\": \"error\",\n"
                                "  \"message\": \"Delete operation not allowed on this path\",\n"
                                "  \"method\": \"DELETE\",\n"
                                "  \"path\": \"" + path + "\",\n"
                                "  \"timestamp\": \"" + getCurrentTimestamp() + "\"\n"
                                "}";
        
        response.setBody(error_body);
        response.setHeader("Content-Length", HttpUtils::toString(error_body.length()));
        return;
    }
    
    struct stat file_stat;
    if (stat(file_path.c_str(), &file_stat) == 0) {
        if (S_ISREG(file_stat.st_mode)) {
            // 파일 삭제 시도
            if (unlink(file_path.c_str()) == 0) {
                response.setStatusCode(200); // OK
                response.setHeader("Content-Type", "application/json");
                
                std::string success_body = "{\n"
                                          "  \"status\": \"success\",\n"
                                          "  \"message\": \"File deleted successfully\",\n"
                                          "  \"method\": \"DELETE\",\n"
                                          "  \"path\": \"" + path + "\",\n"
                                          "  \"deleted_file\": \"" + file_path + "\",\n"
                                          "  \"timestamp\": \"" + getCurrentTimestamp() + "\"\n"
                                          "}";
                
                response.setBody(success_body);
                response.setHeader("Content-Length", HttpUtils::toString(success_body.length()));
            } else {
                prepareErrorResponse(403); // Forbidden - 삭제 권한 없음
            }
        } else {
            prepareErrorResponse(400); // Bad Request - 디렉토리는 삭제 불가
        }
    } else {
        prepareErrorResponse(404); // Not Found
    }
}

void Client::handleFileUpload(const Location& loc) {
    // 파일 업로드 처리
    std::string upload_dir = loc.getUploadStore();
    if (upload_dir.empty()) {
        upload_dir = "./www/uploads"; // 기본 업로드 디렉토리
    }
    
    // 업로드 디렉토리가 존재하는지 확인
    struct stat dir_stat;
    if (stat(upload_dir.c_str(), &dir_stat) != 0 || !S_ISDIR(dir_stat.st_mode)) {
        prepareErrorResponse(500); // Internal Server Error - 업로드 디렉토리 없음
        return;
    }
    
    std::string body = request.getBody();
    if (body.empty()) {
        prepareErrorResponse(400); // Bad Request - 빈 바디
        return;
    }
    
    // 업로드된 파일명 생성
    std::string filename = "upload_" + getCurrentTimestamp() + ".txt";
    std::string file_path = upload_dir + "/" + filename;
    
    // 파일 저장
    std::ofstream file(file_path.c_str(), std::ios::binary);
    if (file.is_open()) {
        file << body;
        file.close();
        
        response.setStatusCode(201); // Created
        response.setHeader("Content-Type", "application/json");
        
        std::string success_body = "{\n"
                                  "  \"status\": \"success\",\n"
                                  "  \"message\": \"File uploaded successfully\",\n"
                                  "  \"method\": \"POST\",\n"
                                  "  \"path\": \"" + request.getPath() + "\",\n"
                                  "  \"uploaded_file\": \"" + filename + "\",\n"
                                  "  \"file_size\": " + HttpUtils::toString(body.length()) + ",\n"
                                  "  \"upload_dir\": \"" + upload_dir + "\",\n"
                                  "  \"timestamp\": \"" + getCurrentTimestamp() + "\"\n"
                                  "}";
        
        response.setBody(success_body);
        response.setHeader("Content-Length", HttpUtils::toString(success_body.length()));
    } else {
        prepareErrorResponse(500); // Internal Server Error - 파일 쓰기 실패
    }
}

std::string Client::getCurrentTimestamp() const {
    time_t now = time(0);
    char buf[100];
    struct tm* timeinfo = localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buf);
}

bool Client::isDeleteAllowedForLocation(const Location& loc, const std::string& file_path) const {
    // 1. upload_store가 설정된 location인 경우: 해당 업로드 디렉토리 내에서만 삭제 허용
    if (loc.hasUploadStore()) {
        std::string upload_dir = loc.getUploadStore();
        // 파일 경로가 업로드 디렉토리 내에 있는지 확인
        return file_path.find(upload_dir) == 0;
    }
    
    // 2. root path가 설정된 location인 경우: 해당 root 디렉토리 내에서만 삭제 허용
    if (!loc.getRootPath().empty()) {
        std::string root_path = loc.getRootPath();
        // 절대 경로로 정규화
        if (!root_path.empty() && root_path[root_path.length() - 1] == '/') {
            root_path.erase(root_path.length() - 1);
        }
        return file_path.find(root_path) == 0;
    }
    
    // 3. 서버 전체 root 디렉토리 내에서만 삭제 허용 (기본 보안)
    std::string server_root = server.getRootPath();
    if (!server_root.empty()) {
        if (!server_root.empty() && server_root[server_root.length() - 1] == '/') {
            server_root.erase(server_root.length() - 1);
        }
        return file_path.find(server_root) == 0;
    }
    
    // 4. 기본적으로 ./www 디렉토리 내에서만 삭제 허용 (최소한의 보안)
    return file_path.find("./www") == 0;
}
