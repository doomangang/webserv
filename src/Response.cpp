#include "../inc/Response.hpp"
#include "../inc/Server.hpp"

// OCF

Response::Response() : _code(200), _cgi_state(0) {}

Response::~Response() {}

Response::Response(const Response& other) {
	*this = other;
}

Response& Response::operator=(const Response& other) {
	if (this != &other) {
		_code = other._code;
		_status_description = other._status_description;
		_headers = other._headers;
		_body = other._body;
		_cgi_state = other._cgi_state;
		_cgi_obj = other._cgi_obj;
		_response_content = other._response_content;
	}
	return *this;
}

/*
** Core Methods
*/
void Response::clear() {
	_code = 200;
	_status_description.clear();
	_headers.clear();
	_body.clear();
	_cgi_state = 0;
	_response_content.clear();
	_cgi_obj.clear();
}

// Response.cpp - setErrorResponse 메서드 상세 디버깅

void Response::setErrorResponse(short code, const Server& server) {
    static int call_count = 0;
    call_count++;
    
    std::cout << "[DEBUG] setErrorResponse called #" << call_count << " for error code: " << code << std::endl;
    
    _code = code;
    _status_description = HttpUtils::getStatusPhrase(code);

    setDefaultHeaders();
    setHeader("Content-Type", "text/html");

    std::string error_page_path = server.getErrorPage(code);
    std::cout << "[DEBUG] Error page path from config: '" << error_page_path << "'" << std::endl;
    std::cout << "[DEBUG] Server root path: '" << server.getRootPath() << "'" << std::endl;
    
    // 상대경로를 절대경로로 변환
    std::string full_path = error_page_path;
    if (!error_page_path.empty() && error_page_path[0] != '/') {
        std::string root = server.getRootPath();
        if (!root.empty()) {
            if (root[root.length() - 1] == '/') {
                root.erase(root.length() - 1);
            }
            full_path = root + "/" + error_page_path;
        }
    }
    
    std::cout << "[DEBUG] Full error page path: '" << full_path << "'" << std::endl;
    
    // 현재 작업 디렉토리 확인
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "[DEBUG] Current working directory: " << cwd << std::endl;
    }
    
    // 파일 상태 상세 확인
    struct stat file_stat;
    int stat_result = stat(full_path.c_str(), &file_stat);
    std::cout << "[DEBUG] stat() result: " << stat_result << std::endl;
    
    if (stat_result == 0) {
        std::cout << "[DEBUG] File exists!" << std::endl;
        std::cout << "[DEBUG] File size: " << file_stat.st_size << " bytes" << std::endl;
        std::cout << "[DEBUG] Is regular file: " << (S_ISREG(file_stat.st_mode) ? "yes" : "no") << std::endl;
        std::cout << "[DEBUG] File permissions: " << std::oct << (file_stat.st_mode & 0777) << std::dec << std::endl;
    } else {
        std::cout << "[DEBUG] File does not exist or stat failed. errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
        
        // 부모 디렉토리들 확인
        std::string parent_dir = full_path;
        size_t last_slash = parent_dir.rfind('/');
        if (last_slash != std::string::npos) {
            parent_dir = parent_dir.substr(0, last_slash);
            std::cout << "[DEBUG] Checking parent directory: '" << parent_dir << "'" << std::endl;
            
            struct stat dir_stat;
            if (stat(parent_dir.c_str(), &dir_stat) == 0) {
                std::cout << "[DEBUG] Parent directory exists and is " << (S_ISDIR(dir_stat.st_mode) ? "directory" : "not directory") << std::endl;
            } else {
                std::cout << "[DEBUG] Parent directory does not exist" << std::endl;
            }
        }
    }
    
    // 파일 열기 시도 (더 상세한 에러 정보)
    std::ifstream file(full_path.c_str());
    if (file.is_open()) {
        std::cout << "[DEBUG] File opened successfully!" << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        _body = buffer.str();
        std::cout << "[DEBUG] Error page content length: " << _body.length() << std::endl;
        if (_body.length() > 0) {
            std::cout << "[DEBUG] Content preview: " << _body.substr(0, std::min((size_t)100, _body.length())) << "..." << std::endl;
        }
    } else {
        std::cout << "[DEBUG] Failed to open file!" << std::endl;
        std::cout << "[DEBUG] ifstream error flags - fail: " << file.fail() << ", bad: " << file.bad() << ", eof: " << file.eof() << std::endl;
        
        // 대안 경로들 시도
        std::vector<std::string> alternative_paths;
        alternative_paths.push_back("www/html/error_pages/405.html");
        alternative_paths.push_back("./error_pages/405.html");
        alternative_paths.push_back("error_pages/405.html");
        
        for (size_t i = 0; i < alternative_paths.size(); ++i) {
            std::cout << "[DEBUG] Trying alternative path: '" << alternative_paths[i] << "'" << std::endl;
            std::ifstream alt_file(alternative_paths[i].c_str());
            if (alt_file.is_open()) {
                std::cout << "[DEBUG] Alternative path works!" << std::endl;
                std::stringstream buffer;
                buffer << alt_file.rdbuf();
                _body = buffer.str();
                setHeader("Content-Length", HttpUtils::toString(_body.length()));
                return;
            }
        }
        
        // Fallback error page
        std::cout << "[DEBUG] Using fallback error page" << std::endl;
        _body = "<html><head><title>" + HttpUtils::toString(code) + " " + _status_description + "</title></head>"
              + "<body><center><h1>" + HttpUtils::toString(code) + " " + _status_description + "</h1></center></body></html>";
    }
    setHeader("Content-Length", HttpUtils::toString(_body.length()));
}

/**
 * @brief Parses the raw CGI output and builds a proper HTTP response.
 * CGI output can contain its own headers. This function separates them from the body.
 */
void Response::buildResponseFromCgi() {
	std::string raw_cgi_output = _response_content;
	_response_content.clear(); // Clear buffer after use

	// Find the end of CGI headers (first blank line)
	size_t header_end_pos = raw_cgi_output.find("\r\n\r\n");
	if (header_end_pos == std::string::npos) {
		header_end_pos = raw_cgi_output.find("\n\n");
		if (header_end_pos == std::string::npos) {
			// No headers, assume the whole output is the body
			_body = raw_cgi_output;
			setStatusCode(200);
			setHeader("Content-Length", HttpUtils::toString(_body.length()));
			return;
		}
	}
	
	std::string cgi_headers_str = raw_cgi_output.substr(0, header_end_pos);
	_body = raw_cgi_output.substr(header_end_pos + (raw_cgi_output[header_end_pos] == '\r' ? 4 : 2));

	// Set default headers first
	setDefaultHeaders();

	// Parse CGI headers
	std::vector<std::string> cgi_headers = HttpUtils::splitByCRLF(cgi_headers_str);
	bool status_found = false;
	for (size_t i = 0; i < cgi_headers.size(); ++i) {
		std::string& line = cgi_headers[i];
		size_t colon_pos = line.find(':');
		if (colon_pos != std::string::npos) {
			std::string key = line.substr(0, colon_pos);
			std::string value = line.substr(colon_pos + 1);
			HttpUtils::trim(value);

			if (HttpUtils::toLowerCase(key) == "status") {
				setStatusCode(std::atoi(value.c_str()));
				status_found = true;
			} else {
				// Overwrite default headers with CGI-provided ones
				setHeader(key, value);
			}
		}
	}

	if (!status_found) {
		setStatusCode(200);
	}
	
	setHeader("Content-Length", HttpUtils::toString(_body.length()));
}

std::string Response::toString() const {
	std::stringstream ss;
	
	// Status Line
	ss << "HTTP/1.1 " << _code << " " << _status_description << "\r\n";

	// Headers
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
		ss << it->first << ": " << it->second << "\r\n";
	}

	// End of headers
	ss << "\r\n";

	// Body
	ss << _body;

	return ss.str();
}

/*
** Getter & Setter
*/
int Response::getCgiState() const { return _cgi_state; }
void Response::setCgiState(int state) { _cgi_state = state; }

void Response::setStatusCode(int code) {
	_code = code;
	_status_description = HttpUtils::getStatusPhrase(code);
}

void Response::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void Response::setBody(const std::string& body) {
	_body = body;
}

std::string Response::getHeaderValue(const std::string& key) const {
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

void Response::setDefaultHeaders() {
	time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time (&rawtime);
    timeinfo = gmtime (&rawtime);
    strftime (buffer,80,"%a, %d %b %Y %H:%M:%S GMT",&*timeinfo);

	setHeader("Server", "webserv/1.0");
	setHeader("Date", std::string(buffer));
}