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

    if (!result.empty() && result.back().empty())
        result.pop_back();

    return result;
}

std::vector<std::string> HttpUtils::splitBySemicolon(const std::string& s) {
    std::vector<std::string> parts;
    size_t start = 0;

    while (true) {
        size_t pos = s.find(';', start);
        if (pos == std::string::npos) {
            parts.push_back(s.substr(start));
            break;
        }
        parts.push_back(s.substr(start, pos - start));
        start = pos + 1;
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

std::vector<std::string> HttpUtils::splitByCRLF(const std::string &raw) {
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

Method HttpUtils::stringToMethod(const std::string& method_str) {
    if (method_str == "GET") return GET;
    if (method_str == "POST") return POST;
    if (method_str == "DELETE") return DELETE;
    return UNKNOWN_METHOD;
}

std::string HttpUtils::methodToString(Method m) {
    switch (m) {
        case GET:    return "GET";
        case POST:   return "POST";
        case DELETE: return "DELETE";
        case UNKNOWN_METHOD: return "UNKNOWN";
        default:     return "UNKNOWN";
    }
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

std::string HttpUtils::getStatusPhrase(int code) {
    switch (code) {
        case 200: return "OK"; break;
        case 201: return "Created"; break;
        case 204: return "No Content"; break;
        case 301: return "Moved Permanently"; break;
        case 302: return "Found"; break;
        case 400: return "Bad Request"; break;
        case 401: return "Unauthorized"; break;
        case 403: return "Forbidden"; break;
        case 404: return "Not Found"; break;
        case 405: return "Method Not Allowed"; break;
        case 408: return "Request Timeout"; break;
        case 413: return "Payload Too Large"; break;
        case 414: return "URI Too Long"; break;
        case 500: return "Internal Server Error"; break;
        case 501: return "Not Implemented"; break;
        case 502: return "Bad Gateway"; break;
        case 503: return "Service Unavailable"; break;
        case 504: return "Gateway Timeout"; break;
        case 505: return "HTTP Version Not Supported"; break;
        default:  return "Unknown"; break;
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

// std::string HttpUtils::getErrorPage(int code) const {
//     std::map<int, std::string>::const_iterator it = _error_pages.find(code);
//     if (it != _error_pages.end()) {
//         return it->second;
//     }
//     return _default_error_page; 
// }

bool HttpUtils::fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool HttpUtils::isDirectory(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string HttpUtils::readFile(const std::string& path) {
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) return "";
    
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string content(static_cast<size_t>(size), '\0');
    file.read(&content[0], size);
    return content;
}

std::vector<std::string> HttpUtils::listDirectory(const std::string& path) {
    std::vector<std::string> entries;
    DIR* dir = opendir(path.c_str());
    if (!dir) return entries;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name != "." && name != "..") {
            entries.push_back(name);
        }
    }
    closedir(dir);
    return entries;
}

bool HttpUtils::isValidPort(int port) {
    return port > 0 && port <= 65535;
}


bool HttpUtils::dirExists(std::string& str, std::string dir) {
    std::vector<std::string> token = splitWords(str);
    std::string directory(dir);
    if (token.front() == directory)
        return true;
    return false;
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