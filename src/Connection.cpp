#include "../inc/Connection.hpp"


// (초기 상태)  ──> [요청 줄 파싱 시도] ──> [헤더 파싱 시도] ──> [본문 파싱 시도] ──> [완료]
// |                      |                     |
// +-- 데이터 부족 ──> 대기/추가 recv() ──> 대기/추가 recv() ──> 응답 전송
// ↓                      ↓                     ↓
// 오류 발견 ──> 400(BAD_REQUEST) 응답 & 종료


void Connection::readClient() {
    char buf[8192];
    ssize_t n = recv(_fd, buf, sizeof(buf), 0);
    if (n <= 0) {
        _progress = END_CONNECTION; 
        return ;
    }
    std::string& _raw_buffer = this->_raw_buffer;
    _raw_buffer = _raw_buffer + std::string(buf, n); // _raw_buffer에 읽은 데이터를 추가

    
    _progress = FROM_CLIENT; // 현재 진행 상태를 FROM_CLIENT로 설정
    _last_request_at = {}; // 현재 시간을 설정 (초기화)
    _client_ip = ""; // 클라이언트 IP 주소를 설정 (예시로 빈 문자열 사용)
    _client_port = 0; // 클라이언트 포트를 설정 (예시로 0 사용)
    // _raw_buffer에 읽은 데이터를 추가
    _raw_buffer.append(buf, n);

}






//ocf
Connection::Connection() {}

Connection::Connection(int client_fd, const std::string& client_ip, int client_port) {
    (void)client_fd; (void)client_ip; (void)client_port;
}

Connection::Connection(const Connection& other) { *this = other; }

Connection::~Connection() {}

Connection& Connection::operator=(const Connection& other) {
    if (this != &other) {
        this->_fd = other.getFd();
    }
    return *this;
}