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