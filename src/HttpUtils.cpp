#include "../inc/HttpUtils.hpp"

void HttpUtils::trim(std::string& s) {
    const char* ws = " \t\n\r";

    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) {
        s.clear();
        return;
    }
    s.erase(0, start);

    size_t end = s.find_last_not_of(ws);
    s.erase(end + 1);
}

std::vector<std::string> HttpUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }

    if (!parts.empty() && parts.back().empty())
        parts.pop_back();

    return result;
}

std::vector<std::string> HttpUtils::splitBySemicolon(const std::string& s) {
    std::vector<std::string> parts;
    size_t start = 0, pos = 0;

    while (pos < s.size()) {
        if (s[pos] == ';') {
            std::string token = s.substr(start, pos - start);
            parts.push_back(token);
            start = pos + 1;
        }
        pos++;
    }
    return parts;
}

std::vector<std::string> HttpUtils::splitWords(const std::string& s) {
    std::vector<std::string> words;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        words.push_back(token);
    }
    return words;
}

static std::vector<std::string> HttpUtils::splitByCRLF(const std::string &raw) {
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

std::string HttpUtils::toLowerCase(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

std::string HttpUtils::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            // 16진수 문자 2개를 숫자로 변환
            std::string hex = str.substr(i + 1, 2);
            char* end;
            long value = std::strtol(hex.c_str(), &end, 16);
            
            if (end == hex.c_str() + 2) {  // 성공적으로 변환됨
                result += static_cast<char>(value);
                i += 2;
            } else {
                result += str[i];
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string HttpUtils::urlEncode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = str[i];
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else {
            std::ostringstream oss;
            oss << '%' << std::hex << std::uppercase << static_cast<int>(c);
            result += oss.str();
        }
    }
    return result;
}

void HttpUtils::getStatusPhrase(int code) {
    _status_code = code;
    switch (code) {
        case 200: _status_description = "OK"; break;
        case 201: _status_description = "Created"; break;
        case 204: _status_description = "No Content"; break;
        case 301: _status_description = "Moved Permanently"; break;
        case 302: _status_description = "Found"; break;
        case 400: _status_description = "Bad Request"; break;
        case 401: _status_description = "Unauthorized"; break;
        case 403: _status_description = "Forbidden"; break;
        case 404: _status_description = "Not Found"; break;
        case 405: _status_description = "Method Not Allowed"; break;
        case 408: _status_description = "Request Timeout"; break;
        case 413: _status_description = "Payload Too Large"; break;
        case 414: _status_description = "URI Too Long"; break;
        case 500: _status_description = "Internal Server Error"; break;
        case 501: _status_description = "Not Implemented"; break;
        case 502: _status_description = "Bad Gateway"; break;
        case 503: _status_description = "Service Unavailable"; break;
        case 504: _status_description = "Gateway Timeout"; break;
        case 505: _status_description = "HTTP Version Not Supported"; break;
        default:  _status_description = "Unknown"; break;
    }
}

std::string HttpUtils::getMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = path.substr(dot + 1);

    for (size_t i = 0; i < ext.length(); ++i) {
        ext[i] = std::tolower(ext[i]);
    }
    
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

//실행부 함수

std::string Server::getErrorPage(int code) const {
    std::map<int, std::string>::const_iterator it = _error_pages.find(code);
    if (it != _error_pages.end()) {
        return it->second;
    }
    return _default_error_page; 
}

// OCF (Orthodox Canonical Form) 생성자
HttpUtils::HttpUtils() {}
HttpUtils::HttpUtils(const HttpUtils& other) {
    *this = other;
}
HttpUtils::~HttpUtils() {}
HttpUtils& HttpUtils::operator=(const HttpUtils& other) {
    if (this != &other) {
        (void)other;
    }
    return *this;
}