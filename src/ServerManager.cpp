#include "../inc/ServerManager.hpp"
#include <sstream>

//OCF

ServerManager::ServerManager() 
    : _max_fd(0) {
    FD_ZERO(&_read_set);
    FD_ZERO(&_read_copy_set);
    FD_ZERO(&_write_set);
    FD_ZERO(&_write_copy_set);
    FD_ZERO(&_error_set);
    FD_ZERO(&_error_copy_set);
}

ServerManager::ServerManager(const Config& config) 
    :
      _max_fd(0) {
    FD_ZERO(&_read_set);
    FD_ZERO(&_read_copy_set);
    FD_ZERO(&_write_set);
    FD_ZERO(&_write_copy_set);
    FD_ZERO(&_error_set);
    FD_ZERO(&_error_copy_set);
    
    _servers = config.getServers();
}

ServerManager::ServerManager(const ServerManager& other) : _config(other._config){}

ServerManager::~ServerManager() {}

ServerManager& ServerManager::operator=(const ServerManager& other) {
    if (this != &other) {
        _config = other._config;
        _servers = other._servers;
        _max_fd = other._max_fd;
        _read_set = other._read_set;
        _read_copy_set = other._read_copy_set;
        _write_set = other._write_set;
        _write_copy_set = other._write_copy_set;
        _error_set = other._error_set;
        _error_copy_set = other._error_copy_set;
    }
    return *this;
}

void ServerManager::setupServers(std::vector<Server> servers)
{
    std::cout << std::endl;
    Logger::logMsg(MAGENTA, CONSOLE_OUTPUT, "Initializing Servers...");

    _servers = std::move(servers);
    char buf[INET_ADDRSTRLEN];

    // key: "host:port" → fd
    std::map<std::string, int> serverFdMap;

    for (auto& server : _servers)
    {
        std::string hostStr = inet_ntop(AF_INET, &server.getHost(), buf, INET_ADDRSTRLEN);
        std::ostringstream key;
        key << hostStr << ":" << server.getPort();

        if (serverFdMap.count(key.str()))
        {
            server.setFd(serverFdMap[key.str()]);
        }
        else
        {
            server.setupServer();
            serverFdMap[key.str()] = server.getFd();
        }

        Logger::logMsg(MAGENTA, CONSOLE_OUTPUT,
                       "Server Created: ServerName[%s] Host[%s] Port[%d]",
                       server.getServerName().c_str(), hostStr.c_str(), server.getPort());
    }
}


void ServerManager::runServers()
{
    fd_set  recv_set_cpy;
    fd_set  write_set_cpy;
    int     select_ret;

    _max_fd = 0;
    initializeSets();
    struct timeval timer;
    while (true)
    {
        timer.tv_sec = 1;
        timer.tv_usec = 0;
        recv_set_cpy = _read_set;
        write_set_cpy = _write_set;

        if ( (select_ret = select(_max_fd + 1, &recv_set_cpy, &write_set_cpy, NULL, &timer)) < 0 )
        {
		    Logger::logMsg(RED, CONSOLE_OUTPUT, "webserv: select error %s   Closing ....", strerror(errno));
            exit(1);
            continue ;
        }
        for (int i = 0; i <= _max_fd; ++i)
        {
            if (FD_ISSET(i, &recv_set_cpy) && _servers_map.count(i))
                acceptNewConnection(_servers_map.find(i)->second);
            else if (FD_ISSET(i, &recv_set_cpy) && _clients_map.count(i))
                readRequest(i, _clients_map[i]);
            else if (FD_ISSET(i, &write_set_cpy) && _clients_map.count(i))
            {
                int cgi_state = _clients_map[i].response.getCgiState(); // 0->NoCGI 1->CGI write/read to/from script 2-CGI read/write done
                if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_in[1], &write_set_cpy))
                    sendCgiBody(_clients_map[i], _clients_map[i].response._cgi_obj);
                else if (cgi_state == 1 && FD_ISSET(_clients_map[i].response._cgi_obj.pipe_out[0], &recv_set_cpy))
                    readCgiResponse(_clients_map[i], _clients_map[i].response._cgi_obj);
                else if ((cgi_state == 0 || cgi_state == 2)  && FD_ISSET(i, &write_set_cpy))
                    sendResponse(i, _clients_map[i]);
            }
        }
        checkTimeout();
    }
}


void ServerManager::initializeSets() {
    FD_ZERO(& _read_set);
    FD_ZERO(& _write_set);

    // adds servers sockets to _recv_fd_pool set
    for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        //Now it calles listen() twice on even if two servers have the same host:port
        if (listen(it->getFd(), 512) == -1)
        {
            Logger::logMsg(RED, CONSOLE_OUTPUT, "webserv: listen error: %s   Closing....", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
        {
            Logger::logMsg(RED, CONSOLE_OUTPUT, "webserv: fcntl error: %s   Closing....", strerror(errno));
            exit(EXIT_FAILURE);
        }
        addToSet(it->getFd(), _read_set);
        _servers_map.insert(std::make_pair(it->getFd(), *it));
    }
    // at this stage _biggest_fd will belong to the last server created.
    _max_fd = _servers.back().getFd();
}

void ServerManager::acceptNewConnection(Server& server) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(server.getFd(), (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Logger::logMsg(RED, CONSOLE_OUTPUT, "accept failed: %s", strerror(errno));
        }
        return;
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::logMsg(RED, CONSOLE_OUTPUT, "fcntl set O_NONBLOCK failed: %s", strerror(errno));
        close(client_fd);
        return;
    }

    Connection client;
    client.setFd(client_fd);
    client.setIp(inet_ntoa(client_addr.sin_addr));
    client.setPort(ntohs(client_addr.sin_port));
    client.setLastRequestAt();
    _clients_map[client_fd] = client;

    Logger::logMsg(GREEN, CONSOLE_OUTPUT, "New connection accepted: fd[%d]", client_fd);
    addToSet(client_fd, _read_set);
}

void ServerManager::addToSet(int fd, fd_set& set) {
    FD_SET(fd, &set);
    if (fd > _max_fd)
        _max_fd = fd;
}

void ServerManager::acceptNewConnection(Server& server) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(server.getFd(), (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Logger::logMsg(RED, CONSOLE_OUTPUT, "accept failed: %s", strerror(errno));
        }
        return ;
    }

    int flags = fcntl(client_fd, F_GETFL, 0);
    if (flags == -1 || fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        Logger::logMsg(RED, CONSOLE_OUTPUT, "fcntl set O_NONBLOCK failed: %s", strerror(errno));
        removeFromSet(client_fd, _read_set);
        close(client_fd); 
        return ;
    }

    Connection client;
    client.setFd(client_fd);
    client.setClientAddress(client_addr);
    client.setLastRequestAt(NULL);
    if (_clients_map.count(client_fd) != 0) {
        _clients_map.erase(client_fd); // Remove existing client if it exists
    }
    _clients_map[client_fd] = client;
    Logger::logMsg(GREEN, CONSOLE_OUTPUT, "New connection accepted: fd[%d]", client_fd);
}

void ServerManager::readRequest(int fd, Connection& client) {
    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            Logger::logMsg(RED, CONSOLE_OUTPUT, "read failed: %s", strerror(errno));
            closeConnection(fd);
        }
        return;
    } else if (bytes_read == 0) {
        Logger::logMsg(YELLOW, CONSOLE_OUTPUT, "Client fd[%d] disconnected", fd);
        closeConnection(fd);
        return;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the buffer
    // client.requestBuffer.append(buffer, bytes_read);
    client.setLastRequestAt(time(NULL));

    Logger::logMsg(BLUE, CONSOLE_OUTPUT, "Read request from fd[%d]: %ld bytes", fd, bytes_read);

    // Process the request (e.g., parse it)
    // This is where you would typically parse the request and handle it accordingly.
}

void ServerManager::sendCgiBody(Connection& client, CgiHandler& cgi_obj) {

}

void ServerManager::readCgiResponse(Connection& client, CgiHandler& cgi_obj) {

}

void ServerManager::sendResponse(int fd, Connection& client) {

}

/* Checks time passed for clients since last message, If more than CONNECTION_TIMEOUT, close connection */
void    ServerManager::checkTimeout()
{
    for(std::map<int, Connection>::iterator it = _clients_map.begin() ; it != _clients_map.end(); ++it)
    {
        if (time(NULL) - it->second.getLastRequestAt() > CONNECTION_TIMEOUT)
        {
            Logger::logMsg(YELLOW, CONSOLE_OUTPUT, "Client %d Timeout, Closing Connection..", it->first);
            closeConnection(it->first);
            return ;
        }
    }
}

void ServerManager::closeConnection(int fd) {
    if (_clients_map.count(fd)) {
        Logger::logMsg(YELLOW, CONSOLE_OUTPUT, "Closing connection fd[%d]", fd);
        close(fd);
        _clients_map.erase(fd);
    } else {
        Logger::logMsg(YELLOW, CONSOLE_OUTPUT, "Attempted to close unknown fd[%d]", fd);
    }
}

void	ServerManager::removeFromSet(const int i, fd_set &set)
{
    FD_CLR(i, &set);
    if (i == _max_fd)
        _max_fd--;
}
// Config ServerManager::getConfig() const { return _config; }
int     ServerManager::getMaxFd() const { return _max_fd; }
// void    ServerManager::setConfig(const Config& config) { _config = config; }
void    ServerManager::setMaxFd(int max_fd) { _max_fd = max_fd; }

/*예를 들어 splitConfigString 은:

전체 문자열에서 "server" 키워드로 블록 시작점 찾기

중괄호 {} 레벨 카운터로 블록 끝까지 자르기

그 사이 문자열을 servers.push_back(...)

남은 부분은 outConfig = ...*/