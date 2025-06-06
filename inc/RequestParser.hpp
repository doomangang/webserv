// RequestParser.hpp
#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <string>
#include <vector>
#include "Request.hpp"

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

class RequestParser
{
public:
    RequestParser();
    ~RequestParser();

    /**
     * @brief raw_request 전체 문자열을 파싱해서 Request 객체에 저장
     * @param raw_request  소켓에서 읽어들인 HTTP 요청(바이트 그대로 모두 포함)
     * @param req          파싱 결과를 저장할 Request 객체 (호출 전에 req.Cleaner() 권장)
     * @return             헤더나 요청 줄 형식 오류가 있으면 false, 정상 파싱 시 true
     */
    static bool parseRawRequest(const std::string &raw_request, Request &req);

private:
    std::string _raw_request;

    RequestParser(const RequestParser &copy);
    RequestParser& operator=(const RequestParser &rhs);

    /**
     * @brief "\r\n" (CRLF)를 기준으로 문자열을 잘라서
     *        각 줄을 벡터에 담아 반환
     * @param raw   원시 문자열
     * @return      CRLF 단위로 나뉜 각 줄
     */
    static std::vector<std::string> splitByCRLF(const std::string &raw);
};

#endif // REQUESTPARSER_HPP
