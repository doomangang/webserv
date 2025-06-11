// Server.hpp

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp"
#include "Location.hpp"
#include "Server.hpp"
#include "ServerManager.hpp"
#include "Config.hpp"
// #include "HttpUtils.hpp"

// class ServerManager;
// class Config;
// class Response;
// class Request;

class Server {
private:
    /* member attributes */
    ServerManager* _manager;
    std::vector<std::string>    _server_names;
    std::string                 _host;
    int                         _port;
    int                         _fd;
    int                         _request_uri_limit_size;
    int                         _request_header_limit_size;
    int                         _limit_client_body_size;
    std::string                 _root_path;
    std::vector<std::string>    _index_files;
    bool                        _autoindex;
    std::string                 _upload_store;
    bool                        _has_upload_store;
    std::string                 _default_error_page;
    std::map<int, std::string>  _error_pages;
    Config*                     _config;
    std::vector<Location>       _locations;

public:
    /* Orthodox Canonical Form (OCF) */
    Server();
    Server(ServerManager*, const std::string&, const std::string&, Config*);
    Server(const Server& other);
    ~Server();
    Server& operator=(const Server& other);

    /* Core Methods */
    void   	setupServer();

    /* Getter & Setter */
    // Server Names
    std::vector<std::string> getServerNames() const;
    void setServerNames(const std::vector<std::string>&);
    void addServerName(const std::string& name);

    // Host & Port
    const std::string& getHost() const;
    void setHost(const std::string& host);
    int getPort() const;
    void setPort(int port);

    // File Descriptor
    int getFd() const;
    void setFd(int fd);

    // Limits
    int getRequestUriLimitSize() const;
    void setRequestUriLimitSize(int size);
    int getRequestHeaderLimitSize() const;
    void setRequestHeaderLimitSize(int size);
    int getLimitClientBodySize() const;
    void setLimitClientBodySize(int size);

    // Path & Index
    std::string getRootPath() const;
    void setRootPath(const std::string& path);
    std::vector<std::string> getIndexFiles() const;
    void setIndexFiles(const std::vector<std::string>& files);

    // Autoindex
    bool getAutoindex() const;
    void setAutoindex(bool onoff);

    // Upload
    bool hasUploadStore() const;
    std::string getUploadStore() const;
    void setHasUploadStore(bool has);
    void setUploadStore(const std::string& path);

    // Error Pages
    const std::string& getDefaultErrorPage() const;
    void setDefaultErrorPage(const std::string& page);
    void addErrorPage(int code, const std::string& path);
    std::string getErrorPage(int code) const;
    std::map<int, std::string> getErrorPages() const;
    
    // Config
    Config* getConfig() const;
    void setConfig(Config* config);

    // Locations
    const std::vector<Location>& getLocations() const;
    void setLocations(const std::vector<Location>& locations);
    void addLocation(const Location& loc);
    const Location& getMatchingLocation(std::string& uri) const;
    const Location& getDefaultLocation() const;

    // Manager
    ServerManager* getManager() const;
    void setManager(ServerManager* manager);

    // /* getter & setter */

    // std::vector<std::string> getServerNames() const;
    // void setServerNames(const std::vector<std::string>&);
    // void addServerName(const std::string& name);

    // std::string getHost() const;
    // void setHost(const std::string& host);

	// /* additional methods */
	// void   	setupServer();

    // int getPort() const;
    // void setPort(int port);

    // int getFd() const;
    // void setFd(int fd);

    // int getRequestUriLimitSize() const;
    // void setRequestUriLimitSize(int size);

    // int getRequestHeaderLimitSize() const;
    // void setRequestHeaderLimitSize(int size);

    // int getLimitClientBodySize() const;
    // void setLimitClientBodySize(int size);

    // std::string getRootPath() const;
    // void setRootPath(const std::string& path);

    // std::vector<std::string> getIndexFiles() const;
    // void setIndexFiles(const std::vector<std::string>& files);

    // bool getAutoindex() const;
    // void setAutoindex(bool onoff);

    // bool hasUploadStore() const;
    // std::string getUploadStore() const;
    // void setHasUploadStore(bool has);
    // void setUploadStore(const std::string& path);

    // // std::string getDefaultErrorPage() const;
    // void setDefaultErrorPage(const std::string& page);

    // void addErrorPage(int code, const std::string& path);
    // std::string getErrorPage(int code) const;
    // std::map<int, std::string> getErrorPages() const;
    
    // Config* getConfig() const;
    // void setConfig(Config* config);

    // const std::vector<Location>& getLocations() const;
    // void setLocations(const std::vector<Location>& locations);
    // void addLocation(const Location& loc);
    // const Location& getMatchingLocation(std::string&) const;
    // const Location& getDefaultLocation() const;

    // ServerManager* getManager() const;
    // void setManager(ServerManager* manager);

	/* exception classes */
};

/* operators */
#endif