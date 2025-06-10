#include "../inc/Response.hpp"

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

void Response::setErrorResponse(short code, const Server& server) {
	_code = code;
	_status_description = HttpUtils::getStatusPhrase(code);

	setDefaultHeaders();
	setHeader("Content-Type", "text/html");

	std::string error_page_path = server.getErrorPage(code);
	std::ifstream file(error_page_path.c_str());
	if (file.is_open()) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		_body = buffer.str();
	} else {
		// Fallback error page
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