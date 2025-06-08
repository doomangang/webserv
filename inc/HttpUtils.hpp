#ifndef HTTP_UTILS_HPP
#define HTTP_UTILS_HPP

#include "Webserv.hpp"

class HttpUtils {
private:
    HttpUtils();
    HttpUtils(const HttpUtils& other);
    ~HttpUtils();
    HttpUtils& operator=(const HttpUtils& other);

public:
    // String utilities
    static void trim(std::string& str);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::vector<std::string> splitWords(const std::string& str);
    static std::vector<std::string> splitByCRLF(const std::string& str);

    
    static std::string toLowerCase(const std::string& str);
    static std::string urlDecode(const std::string& str);
    static std::string urlEncode(const std::string& str);
    
    // HTTP utilities
    static std::string getStatusPhrase(int code);
    static std::string getMimeType(const std::string& file_extension);
    static std::string getCurrentTimeString();
    static std::string getFileExtension(const std::string& path);
    
    // File utilities
    static bool fileExists(const std::string& path);
    static bool isDirectory(const std::string& path);
    static std::string readFile(const std::string& path);
    static std::vector<std::string> listDirectory(const std::string& path);
    
    // Network utilities
    static bool isValidPort(int port);

    // 실행부 함수
    std::string statusCodeString(short);
    std::string getErrorPage(short);
    int buildHtmlIndex(std::string &, std::vector<uint8_t> &, size_t &);
    int ft_stoi(std::string str);
    unsigned int fromHexToDec(const std::string& nb);

    
    // Template utility
    template<typename T>
    static std::string toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
};

#endif