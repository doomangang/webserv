#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "Webserv.hpp"

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

class Server;

class ServerManager {
public:
    enum SetType { WRITE_SET, WRITE_COPY_SET,
                    READ_SET, READ_COPY_SET,
                    ERROR_SET, ERROR_COPY_SET };

private:
    /* member attributes */
    std::vector<Server> _servers;
    Config              _config;
    int                 _max_fd;
    fd_set              _read_set,
                        _read_copy_set,
                        _write_set,
                        _write_copy_set,
                        _error_set,
                        _error_copy_set;

    std::unordered_map<int, Client> _clients_map;
    std::unordered_map<int, Server> _servers_map;

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
    void    readRequest(int fd, Client& client);
    void    sendCgiBody(Client& client, CgiHandler& cgi_obj);
    void    readCgiResponse(Client& client, CgiHandler& cgi_obj);
    void    sendResponse(int fd, Client& client);
    void	removeFromSet(const int i, fd_set &set);
    void    closeConnection(int fd);
    void    checkTimeout();
    void    addToSet(int fd, fd_set& set);


    /* exception classes */
};

/* operators */
#endif
