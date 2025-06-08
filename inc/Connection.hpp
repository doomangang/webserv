#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Webserv.hpp"

class Connection {
public:
	/* Orthodox Canonical Form */
	Connection();
	Connection(int client_fd,
			   const std::string& client_ip,
			   int client_port,
			   const Config* config_ptr);
	Connection(const Connection& other);
	~Connection();
	Connection& operator=(const Connection& other);

	/* network operations */
	void						readClient();
	void						writeClient();

	/* HTTP lifecycle */
	void						process();

	/* readiness checks */
	bool						needsRead()  const;
	bool						needsWrite() const;
	bool						isComplete() const;

	// 실행부
	Response					response;          // 클라이언트에 대한 응답 객체
	std::string         		cgiInputBuffer;    // CGI 입력 버퍼
	std::string         		cgiOutputBuffer;   // CGI 출력 버퍼
	std::string         		requestBuffer;     // 클라이언트 요청 버퍼
	Request             		request;           // 요청 객체
	Server              		server;            // 서버 정보

	/* I/O accessors */
	int							getFd() const;
	const std::string& 			getClientIp() const;
	int         				getClientPort() const;

	// Getter/Setter from Client
	int							getClientSocket() const;
	const struct sockaddr_in&   getClientAddress() const;
	void						setClientSocket(int sock);
	void						setClientAddress(const struct sockaddr_in& addr);
	void						setFd(int fd);
	void						setLastRequestAt(const time_t& tv);
	void						setIp(const std::string& ip);
	void						setPort(int port);
	void						setLastRequestAt(); // now()

private:
	/* connection state */
	int         				_fd;
	std::string 				_client_ip;
	int         				_client_port;
	time_t      				_last_request_at;

	/* parsing & request */
	RequestParser 				_parser;
	Request       				_request;

	/* response */
	Response      				_response;
	std::string   				_response_buffer;
	size_t        				_bytes_sent;

	/* configuration */
	const Config*   			_config_ptr;
	const Server*   			_server_ptr;
	const Location* 			_location_ptr;

	// 실행부 멤버 from Client
	int                 		_client_socket;
	struct sockaddr_in  		_client_address;
};

#endif
