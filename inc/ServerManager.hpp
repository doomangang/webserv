#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Webserv.hpp"
#include "Config.hpp"
#include "HttpTypes.hpp"
#include "Server.hpp"
#include "Connection.hpp"
#include "CgiHandler.hpp"

class Server;
class Config;

class ServerManager {
private:
    /* member attributes */
    std::vector<Server> _servers;
    // Config              _config;
    int                 _max_fd;
    fd_set              _read_set,
                        _read_copy_set,
                        _write_set,
                        _write_copy_set,
                        _error_set,
                        _error_copy_set;

    // std::map<int, Connection> _clients_map;
    // std::map<int, Server> _servers_map;

        public:
    /* Orthodox Canonical Form (OCF) */
    ServerManager();
    ServerManager(const Config&);
    ServerManager(const ServerManager& other);
    ~ServerManager();
    ServerManager& operator=(const ServerManager& other);

    /* getter & setter */
    Config  getConfig() const;
    int     getMaxFd() const;
    void    setConfig(const Config&);
    void    setMaxFd(int);

    /* additional methods */
    bool    splitConfigString(const std::string& src,
                                std::string& config_block,
                                std::vector<std::string>& server_strings);
    bool    splitServerString(const std::string& src,
                                std::string& server_block, 
                                std::vector<std::string>& location_blocks);

    bool    isValidConfigBlock(const std::string&);
    bool    isValidServerBlock(const std::string&);
    bool    isValidLocationBlock(const std::string&);

    void    setupServers(std::vector<Server> servers);
    void    setupServer(Server& server);
    void    runServers();
    void    runServer(Server& server);
    void    exitServer(const std::string& msg);

    void    fdSet(int fd, SetType);
    void    fdZero(SetType);
    void    fdClear(int fd, SetType);
    bool    fdIsset(int fd, SetType) const;
    void    fdCopy(SetType origin, SetType copy);
    void    initializeSets();
    void    acceptNewConnection(Server& server);
    void    readRequest(int fd, Connection& client);
    void    sendCgiBody(Connection& client, CgiHandler& cgi_obj);
    void    readCgiResponse(Connection& client, CgiHandler& cgi_obj);
    void    sendResponse(int fd, Connection& client);
    void	removeFromSet(const int i, fd_set &set);
    void    closeConnection(int fd);
    void    checkTimeout();
    void    addToSet(int fd, fd_set& set);


    /* exception classes */
};

/* operators */
#endif
