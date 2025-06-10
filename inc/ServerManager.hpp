#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

#include "Webserv.hpp"
#include "Client.hpp"
#include "Response.hpp"

/**
 * ServerManager
 * * operates the webserver and is responsible for
 * - runing servers with configration extracted from config file
 * - establishing new connections with clients and receive/send requests/responses.
 */
class ServerManager
{
    public:                 
        ServerManager();
        ~ServerManager();
        void    setupServers(std::vector<Server>);
        void    runServers();
        
    private:
        std::vector<Server> _servers;
        std::map<int, Server> _servers_map;
        std::map<int, Client> _clients_map;
        fd_set     _recv_fd_pool;
        fd_set     _write_fd_pool;
        int        _biggest_fd;

        void acceptNewConnection(Server &);
        void checkTimeout();
        void initializeSets();
        void readRequest(const int &, Client &);
        void handleReqBody(Client &);
        void sendResponse(const int &, Client &);
        void sendCgiBody(Client &, CgiHandler &);
        void readCgiResponse(Client &, CgiHandler &);
        void closeConnection(const int);
        // void assignServer(Client &);
        void addToSet(const int , fd_set &);
        void removeFromSet(const int , fd_set &);
};


#endif // SERVERMANAGER_HPP