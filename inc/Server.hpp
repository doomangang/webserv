// Server.hpp

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <string>
#include "Location.hpp"

class ServerManager;
class Config;

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
    std::string                 _root_path;             // 새로 추가
    std::vector<std::string>    _index_files;           // 새로 추가
    bool                        _autoindex;             // 새로 추가
    std::string                 _upload_store;          // 새로 추가
    bool                        _has_upload_store;      // 새로 추가
    std::string                 _default_error_page;
    std::map<int, std::string>  _error_pages;
    Config*                     _config;
    std::vector<Location>       _locations;
    std::map<int, Connection>   _connections;
    std::queue<Response>        _responses;

public:
    /* Orthodox Canonical Form (OCF) */
    Server();
    Server(ServerManager*, const std::string& server_block, const std::string& location_blocks, Config*);
    Server(const Server& other);
    ~Server();
    Server& operator=(const Server& other);

    /* getter & setter */

    std::vector<std::string> getServerNames() const;
    void setServerNames(const std::vector<std::string>&);
    void addServerName(const std::string& name);

    std::string getHost() const;
    void setHost(const std::string& host);
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

    std::string getRootPath() const;
    void setRootPath(const std::string& path);

    std::vector<std::string> getIndexFiles() const;
    void setIndexFiles(const std::vector<std::string>& files);

    bool getAutoindex() const;
    void setAutoindex(bool onoff);

    bool hasUploadStore() const;
    std::string getUploadStore() const;
    void setHasUploadStore(bool has);
    void setUploadStore(const std::string& path);

    std::string getDefaultErrorPage() const;
    void setDefaultErrorPage(const std::string& page);

    void addErrorPage(int code, const std::string& path);
    std::string getErrorPage(int code) const;
    std::map<int, std::string> getErrorPages() const;
    
    Config* getConfig() const;
    void setConfig(Config* config);

    const std::vector<Location>& getLocations() const;
    void setLocations(const std::vector<Location>& locations);
    void addLocation(const Location& loc);
    const Location& getMatchingLocation(std::string&) const;
    const Location& getDefaultLocation() const;

    ServerManager* getManager() const;
    void setManager(ServerManager* manager);

	
	void	createResponse(int status_code);
	bool	isSendable(int fd) const;
	void	sendResponse(const Response&);
	bool	hasException(int fd) const;
	void	run();

	/* exception classes */
};

/* operators */
#endif