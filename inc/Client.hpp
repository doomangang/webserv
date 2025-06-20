#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "RequestParser.hpp"
#include "ResponseWriter.hpp"

// Forward declarations to avoid circular includes
class Server;

class Client {
private:
	/* member attributes */
	int         		_fd;
	time_t     			_last_request_at;
	std::string 		_ip;
	int         		_port;

	ConnectionState		_connection_state;

	void				prepareAutoindexPage(const std::string& dir_path);
	
public:
	/* Orthodox Canonical Form (OCF) */
	Client();
	Client(int client_fd);
	Client(const Client& other);
	~Client();
	Client& operator=(const Client& other);

	Response            response;
	Request             request;
	Server              server;
	RequestParser       parser;
	ResponseWriter		writer;

	/* additional methods */
	void                readAndParse();
	void                findSetConfigs(const std::vector<Server>& servers);
	bool                isParseComplete() const;
	void				processRequest();
	void				prepareErrorResponse(int error_code);
	bool                isMethodAllowed() const;
	void                handleRedirect();
	bool                isCGIRequest(const Location&)const;
	void                handleCGI();
	std::string         resolveFilePath(const Location&) const;
	void                handleStaticFile(const std::string&);
	void                handleDirectoryListing(const Location&, const std::string& dir_path);
	
	// 메서드별 처리 함수들
	void                handleGetRequest(const Location& loc);
	void                handlePostRequest(const Location& loc);
	void                handleDeleteRequest(const Location& loc);
	void                handleFileUpload(const Location& loc);
	
	// 유틸리티 함수
	std::string         getCurrentTimestamp() const;
	bool                isDeleteAllowedForLocation(const Location& loc, const std::string& file_path) const;
	void                clearClient();
	void				updateTime();

	/* getter & setter */
	int                 getFd() const;
	time_t              getLastRequestAt() const;
	std::string         getIp() const;
	int                 getPort() const;
	Server              getServer() const;
	Request&            getRequest();
	Response&           getResponse();
	ResponseWriter&    	getWriter();
	RequestParser&     	getParser();
	ConnectionState     getConnectionState() const;
	
	void                setLastRequestAt();
	void                setFd(int fd);
	void                setIp(const std::string& ip);
	void                setPort(int port);
	void				setServer(const Server& server);
	void				setRequest(const Request& request);
	void				setResponse(const Response& response);
	void				setWriter(const ResponseWriter& writer);
	void				setParser(const RequestParser& parser);
	void				setConnectionState(ConnectionState state);

	/* exception classes */
};

/* operators */
#endif
