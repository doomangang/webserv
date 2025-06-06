#include "../inc/Utils.hpp"

void Utils::trim(std::string& s) {
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

std::vector<std::string> Utils::splitBySemicolon(const std::string& s) {
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

std::vector<std::string> Utils::splitWords(const std::string& s) {
    std::vector<std::string> words;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        words.push_back(token);
    }
    return words;
}

std::string Utils::getMimeType(const std::string& extension) {
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

Utils::Utils() {}
Utils::Utils(std::vector<Server> servers, char* envp[]) { (void)servers, (void)envp; }
Utils::Utils(const Utils& other) {
    *this = other;
}
Utils::~Utils() {}
Utils& Utils::operator=(const Utils& other) {
    if (this != &other) {
        (void)other;
    }
    return *this;
}