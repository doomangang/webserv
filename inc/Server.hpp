#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp"

class ServerManager;
class Config;
class Location;
class Connection;
class Response;
class Request;

class Server {
private:
	/* member attributes */
	ServerManager*              _manager;
	std::string                 _server_name;
	std::string                 _host;
	int                         _port;
	int                         _fd;
	int                         _request_uri_limit_size;
	int                         _request_header_limit_size;
	int                         _limit_client_body_size;
	std::string                 _default_error_page;
	Config*                     _config;
	std::vector<Location>       _locations;
	std::map<int, Connection>   _connections;
	std::queue<Response>        _responses;
	Server();

public:
	/* Orthodox Canonical Form (OCF) */
	Server(ServerManager*, std::string& server_block, std::string& location_blocks, Config*);
	Server(const Server& other);
	~Server();
	Server& operator=(const Server& other);

    // Getter
    const std::string& getServerName() const { return _server_name; }
    const std::string& getHost() const { return _host; }
    int getPort() const { return _port; }
    int getFd() const { return _fd; }
    int getRequestUriLimitSize() const { return _request_uri_limit_size; }
    int getRequestHeaderLimitSize() const { return _request_header_limit_size; }
    int getLimitClientBodySize() const { return _limit_client_body_size; }
    const std::string& getDefaultErrorPage() const { return _default_error_page; }
    Config* getConfig() const { return _config; }
    const std::vector<Location>& getLocations() const { return _locations; }

    // Setter
    void setServerName(const std::string& name) { _server_name = name; }
    void setHost(const std::string& host) { _host = host; }
    void setPort(int port) { _port = port; }
    void setFd(int fd) { _fd = fd; }
    void setRequestUriLimitSize(int size) { _request_uri_limit_size = size; }
    void setRequestHeaderLimitSize(int size) { _request_header_limit_size = size; }
    void setLimitClientBodySize(int size) { _limit_client_body_size = size; }
    void setDefaultErrorPage(const std::string& page) { _default_error_page = page; }
    void setConfig(Config* config) { _config = config; }
    void setLocations(const std::vector<Location>& locations) { _locations = locations; }

	/* additional methods */
	void   	setupServer();

	bool    hasNewConnection() const;
	void    acceptNewConnection();

	bool    hasRequest(int fd) const;
	Request recvRequest(int fd);
	void    solveRequest(const Request&);

	void    executeAutoindex(const Request&);
	void    executeGet(const Request&);
	void    executeHead(const Request&);
	void	executePut(const Request&);
	void	executePost(const Request&);
	void	executeDelete(const Request&);
	void	executeOptions(const Request&);
	void	executeTrace(const Request&);

	char**	createCGIEnv() const;
	void	executeCGI(const Request&);
	
	void	createResponse(int status_code);
	bool	isSendable(int fd) const;
	void	sendResponse(const Response&);
	bool	hasException(int fd) const;
	void	run();

	/* exception classes */
};

/* operators */
#endif