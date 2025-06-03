// Request.hpp
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>
#include "Enum.hpp"
#include "RequestParser.hpp"

/* Color Sets */
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define GREY    "\033[38;5;250m"

class Request
{
public:
    // OCF
    Request();
    Request(const Request &copy);
    Request& operator=(const Request &rhs);
    ~Request();

    void                 SetMethod(const std::string &method_str);
    void                 SetUrl(const std::string &url_str);
    void                 SetVersion(const std::string &ver_str);

    //header
    /**
     * @brief 단일 헤더 라인("Key: value")을 파싱하여 내부 맵에 저장
     * @param header_line  헤더 한 줄 (끝에 CRLF는 이미 제거된 상태)
     * @return             형식이 올바르면 true, 그렇지 않으면 false
     *
     * 동작:
     *  - ':' 위치를 찾아 key/value 분리
     *  - key는 모두 소문자로 변환 후 저장
     *  - value는 앞뒤 공백을 trim해서 저장
     *  - 중복된 key가 들어오면 덮어쓰기
     */
    bool                 ParseHeaderLine(const std::string &header_line);

    /**
     * @brief 헤더 맵에 특정 키가 존재하는지 확인 (case-insensitive)
     * @param key   조회할 헤더 이름 (예: "Host", "content-length")
     * @return      존재하면 true, 없으면 false
     */
    bool                 HasHeader(const std::string &key) const;

    /**
     * @brief 헤더 맵에서 key에 해당하는 value를 조회 (case-insensitive)
     * @param key   예: "Content-Length"
     * @return      해당 값이 있으면 trim된 value, 없으면 빈 문자열
     */
    std::string          GetHeaderValue(const std::string &key) const;

    /**
     * @brief 내부에 저장된 모든 헤더를 const 참조로 반환
     * @return  key는 소문자로 저장, value는 앞뒤 공백이 제거된 상태
     */
    const std::map<std::string, std::string>& GetAllHeaders() const;

    //body
    void                 SetBody(const std::string &body_str);
    void                 ReserveBody(ssize_t size_hint);
    void                 SetBytesToRead(ssize_t bytes);
    void                 AddBodyChunk(const std::string &chunk);
    void                 CutBody(ssize_t size);

    //getter & setter
    Method               GetMethod() const;
    const std::string&   GetUrl() const;
    const std::string&   GetVersion() const;
    size_t               GetBodyPos() const;
    Incomplete           GetStatus() const;
    ssize_t              GetBytesToRead() const;

    void                 SetStatus(Incomplete type);
    void                 AddBodyPos(size_t n);

    void                 Cleaner();

private:
    RequestParser       parser;
    // member attributes
    Method               method;
    std::string          url;
    std::string          version;

    /**
     *  - key: 모두 소문자로 변환된 헤더 이름 ("host", "content-length", ...)
     *  - value
     */
    std::map<std::string, std::string>  header;

    std::string          body;
    size_t               body_pos;       // 본문에서 이미 처리된 위치
    Incomplete           status;
    ssize_t              bytes_to_read;  // 읽어야 할 남은 본문 바이트 수
};

#endif
