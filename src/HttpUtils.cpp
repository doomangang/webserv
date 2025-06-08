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


std::string HttpUtils::getMimeType(const std::string& extension) {
    static std::map<std::string, std::string> mime_types;
    
    // static 초기화를 한 번만 수행
    static bool initialized = false;
    if (!initialized) {
        mime_types["html"] = "text/html";
        mime_types["htm"] = "text/html";
        mime_types["css"] = "text/css";
        mime_types["js"] = "application/javascript";
        mime_types["json"] = "application/json";
        mime_types["jpg"] = "image/jpeg";
        mime_types["jpeg"] = "image/jpeg";
        mime_types["png"] = "image/png";
        mime_types["gif"] = "image/gif";
        mime_types["txt"] = "text/plain";
        mime_types["pdf"] = "application/pdf";
        initialized = true;
    }

    std::map<std::string, std::string>::const_iterator it = mime_types.find(extension);
    if (it != mime_types.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

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

std::string HttpUtils::getMimeType(size_t value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}