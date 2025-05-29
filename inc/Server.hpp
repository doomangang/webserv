#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <queue>
#include "ServerManager.hpp"
#include "Config.hpp"
#include "Location.hpp"
#include "Connection.hpp"
#include "Response.hpp"
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

public:
	/* Orthodox Canonical Form (OCF) */
	Server();
	Server(ServerManager* manager, Config* config);
	Server(const Server& other);
	~Server();
	Server& operator=(const Server& other);

	/* getter & setter */


	/* additional methods */
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
	void	closeConnection(int fd);
	bool	hasException(int fd) const;
	void	run();

	/* exception classes */
};

/* operators */
#endif