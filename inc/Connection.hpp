#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include "Webserv.hpp"
#include "RequestParser.hpp"
#include "Response.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "Request.hpp"
#include "ResponseWriter.hpp"
#include "Server.hpp"

class Config;
class Location;
class Request;
// class RequestParser;
// class Response;
class ResponseWriter;

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

	void 						setServerData();
	void 						setLocationData();
	void 						setupServerAndLocation();

	/* request handling */

	void						updateProgress();
	void						processRequest();
	void						handleParsingError();
	void						cleanUp();

	bool						isMethodAllowed() const;
	bool						isCGIRequest() const;
	void						handleStaticFile();
	void						handleDirectoryListing();
	void						handleCGI();
	void						handleRedirect();

	std::string 				resolveFilePath() const;


	/* response handling */
	void						prepareResponse();
	void						prepareErrorResponse(int error_code);
	void						resetConnection();

	/* I/O accessors */
	int							getFd() const;
	const std::string& 			getClientIp() const;
	int         				getClientPort() const;

	void						setFd(int fd);
	void						setLastRequestAt(const time_t& tv);
	void						setIp(const std::string& ip);
	void						setPort(int port);
	void						setLastRequestAt(); // now()

	// Getter/Setter from Client
	int							getClientSocket() const;
	const struct sockaddr_in&   getClientAddress() const;
	void						setClientSocket(int sock);
	void						setClientAddress(const struct sockaddr_in& addr);
	
	// CGI buffers
    const std::string&          getCgiInputBuffer() const;
    void                        setCgiInputBuffer(const std::string& buf);
    const std::string&          getCgiOutputBuffer() const;
    void                        setCgiOutputBuffer(const std::string& buf);

    // Request buffer
    const std::string&          getRequestBuffer() const;
    void                        setRequestBuffer(const std::string& buf);

    // // Server info
    // const Server&               getServer() const;
    // void                        setServer(const Server& server);

private:
	/* connection state */
	int         				_fd;
	std::string 				_client_ip;
	int         				_client_port;
	time_t      				_last_request_at;
	ConnectionState 			_progress;

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
	std::string         		_cgi_Input_Buffer;    // CGI 입력 버퍼
	std::string         		_cgi_Output_Buffer;   // CGI 출력 버퍼
	std::string         		_request_Buffer;     // 클라이언트 요청 버퍼
	// Server              		_server;            // 서버 정보
};

#endif
