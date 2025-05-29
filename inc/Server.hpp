#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <queue>
#include <map>
// #include "ServerManager.hpp"
// #include "Config.hpp"
#include "Location.hpp"
// #include "Connection.hpp"
// #include "Response.hpp"
// #include "Request.hpp"

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

class ServerManager;
class Config;
class Location;
// class Connection;
// class Response;
// class Request;

class Server {
private:
	/* member attributes */
	ServerManager*              _manager;
	std::vector<std::string>    _server_names;
	std::string                 _host;
	int                         _port;
	int                         _fd;
	int                         _request_uri_limit_size;
	int                         _request_header_limit_size;
	int                         _limit_client_body_size;
	std::string                 _default_error_page;
	std::map<int, std::string>	_error_pages;
	Config*                     _config;
	std::vector<Location>       _locations;
	// std::map<int, Connection>   _connections;
	// std::queue<Response>        _responses;

public:
	/* Orthodox Canonical Form (OCF) */
	Server();
	Server(ServerManager*, std::string& server_block, std::string& location_blocks, Config*);
	Server(const Server& other);
	~Server();
	Server& operator=(const Server& other);

	/* getter & setter */
	/* getter & setter */

	std::vector<std::string> getServerNames() const;
	void setServerNames(std::vector<std::string>&);
	void addServerName(const std::string& name);

	std::string getHost() const;
	void setHost(const std::string& host);

	int getPort() const;
	void setPort(int port);

	int getFd() const;
	void setFd(int fd);

	int getRequestUriLimitSize() const;
	void setRequestUriLimitSize(int size);

	int getRequestHeaderLimitSize() const;
	void setRequestHeaderLimitSize(int size);

	int getLimitClientBodySize() const;
	void setLimitClientBodySize(int size);

	std::string getDefaultErrorPage() const;
	void setDefaultErrorPage(const std::string& page);

	void addErrorPage(int code, const std::string& path);
	std::string getErrorPage(int code) const;

	Config* getConfig() const;
	void setConfig(Config* config);

	const std::vector<Location>& getLocations() const;
	void setLocations(const std::vector<Location>& locations);
	void addLocation(Location loc);

	// std::map<int, Connection>& getConnections();
	// void setConnections(const std::map<int, Connection>& connections);

	// std::queue<Response>& getResponses();
	// void setResponses(const std::queue<Response>& responses);

	ServerManager* getManager() const;
	void setManager(ServerManager* manager);


	/* additional methods */
	// bool    hasNewConnection() const;
	// void    acceptNewConnection();

	// bool    hasRequest(int fd) const;
	// Request recvRequest(int fd);
	// void    solveRequest(const Request&);

	// void    executeAutoindex(const Request&);
	// void    executeGet(const Request&);
	// void    executeHead(const Request&);
	// void	executePut(const Request&);
	// void	executePost(const Request&);
	// void	executeDelete(const Request&);
	// void	executeOptions(const Request&);
	// void	executeTrace(const Request&);

	// char**	createCGIEnv() const;
	// void	executeCGI(const Request&);
	
	// void	createResponse(int status_code);
	// bool	isSendable(int fd) const;
	// void	sendResponse(const Response&);
	// void	closeConnection(int fd);
	// bool	hasException(int fd) const;
	// void	run();

	/* exception classes */
};

/* operators */
#endif