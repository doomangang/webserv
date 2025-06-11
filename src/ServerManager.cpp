# include "../inc/ServerManager.hpp"
# include "../inc/Client.hpp"
# include "../inc/Server.hpp"

ServerManager::ServerManager(){}
ServerManager::~ServerManager(){}

/**
 * Start all servers on ports specified in the config file
 */
void    ServerManager::setupServers(std::vector<Server> servers)
{
    _servers = servers;
    // char buf[INET_ADDRSTRLEN];
    bool    serverDub;
    for (std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        serverDub = false;
        for (std::vector<Server>::iterator it2 = _servers.begin(); it2 != it; ++it2)
        {
            if (it2->getHost() == it->getHost() && it2->getPort() == it->getPort())
            {
                it->setFd(it2->getFd());
                serverDub = true;
                break ;
            }
        }
        if (!serverDub)
            it->setupServer();
    }
}

/**
 * Runs main loop that goes through all file descriptors from 0 till the biggest fd in the set.
 * - check file descriptors returend from select():
 *      if server fd --> accept new client
 *      if client fd in read_set --> read message from client
 *      if client fd in write_set:
 *          1- If it's a CGI response and Body still not sent to CGI child process --> Send request body to CGI child process.
 *          2- If it's a CGI response and Body was sent to CGI child process --> Read outupt from CGI child process.
 *          3- If it's a normal response --> Send response to client.
 * - servers and clients sockets will be added to _recv_set_pool initially,
 *   after that, when a request is fully parsed, socket will be moved to _write_set_pool
 */
void    ServerManager::runServers()
{
    fd_set  recv_set_cpy;
    fd_set  write_set_cpy;
    int     select_ret;

    _biggest_fd = 0;
    initializeSets();
    struct timeval timer;
    while (true)
    {
        timer.tv_sec = 1;
        timer.tv_usec = 0;
        recv_set_cpy = _recv_fd_pool;
        write_set_cpy = _write_fd_pool;

        if ( (select_ret = select(_biggest_fd + 1, &recv_set_cpy, &write_set_cpy, NULL, &timer)) < 0 )
        {
            perror("webserv: select error");
            exit(1);
        }
        for (int i = 0; i <= _biggest_fd; ++i)
        {
            if (FD_ISSET(i, &recv_set_cpy) && _servers_map.count(i))
                acceptNewConnection(_servers_map.find(i)->second);
            else if (FD_ISSET(i, &recv_set_cpy) && _clients_map.count(i))
                readRequest(i, _clients_map[i]);
            else if (FD_ISSET(i, &write_set_cpy) && _clients_map.count(i))
            {
                Client& c = _clients_map[i];
                int cgi_state = c.response.getCgiState();

                if (cgi_state == 1 && FD_ISSET(c.response._cgi_obj.pipe_in[1], &write_set_cpy))
                    sendCgiBody(c, c.response._cgi_obj);
                else if (cgi_state == 1 && FD_ISSET(c.response._cgi_obj.pipe_out[0], &recv_set_cpy))
                    readCgiResponse(c, c.response._cgi_obj);
                else if ((cgi_state == 0 || cgi_state == 2) && FD_ISSET(i, &write_set_cpy))
                    sendResponse(i, c);
            }
        }
        checkTimeout();
    }
}


/* Checks time passed for clients since last message, If more than CONNECTION_TIMEOUT, close connection */
void    ServerManager::checkTimeout()
{
    std::vector<int> fds_to_close;
    for(std::map<int, Client>::iterator it = _clients_map.begin() ; it != _clients_map.end(); ++it)
    {
        if (time(NULL) - it->second.getLastRequestAt() > CONNECTION_TIMEOUT)
        {
            fds_to_close.push_back(it->first);
        }
    }
    for (size_t i = 0; i < fds_to_close.size(); ++i)
    {
        closeConnection(fds_to_close[i]);
    }
}

/* initialize recv+write fd_sets and add all server listening sockets to _recv_fd_pool. */
void    ServerManager::initializeSets()
{
    FD_ZERO(&_recv_fd_pool);
    FD_ZERO(&_write_fd_pool);

    // adds servers sockets to _recv_fd_pool set
    for(std::vector<Server>::iterator it = _servers.begin(); it != _servers.end(); ++it)
    {
        //Now it calles listen() twice on even if two servers have the same host:port
        if (listen(it->getFd(), 512) == -1)
        {
            //LoggerlogMsg(ERROR, CONSOLE_OUTPUT, "webserv: listen error: %s   Closing....", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (fcntl(it->getFd(), F_SETFL, O_NONBLOCK) < 0)
        {
            //LoggerlogMsg(ERROR, CONSOLE_OUTPUT, "webserv: fcntl error: %s   Closing....", strerror(errno));
            exit(EXIT_FAILURE);
        }
        addToSet(it->getFd(), _recv_fd_pool);
        _servers_map.insert(std::make_pair(it->getFd(), *it));
    }
    // at this stage _biggest_fd will belong to the last server created.
    _biggest_fd = _servers.back().getFd();
}

/**
 * Accept new incomming connection.
 * Create new Client object and add it to _client_map
 * Add client socket to _recv_fd_pool
*/
void    ServerManager::acceptNewConnection(Server &serv)
{
    struct sockaddr_in client_address;
    long  client_address_size = sizeof(client_address);
    int client_sock;

    if ( (client_sock = accept(serv.getFd(), (struct sockaddr *)&client_address,
     (socklen_t*)&client_address_size)) == -1)
    {
        //LoggerlogMsg(ERROR, CONSOLE_OUTPUT, "webserv: accept error %s", strerror(errno));
        return ;
    }
    //LoggerlogMsg(INFO, CONSOLE_OUTPUT, "New Connection From %s, Assigned Socket %d",inet_ntop(AF_INET, &client_address, buf, INET_ADDRSTRLEN), client_sock);

    addToSet(client_sock, _recv_fd_pool);

    if (fcntl(client_sock, F_SETFL, O_NONBLOCK) < 0)
    {
        //LoggerlogMsg(ERROR, CONSOLE_OUTPUT, "webserv: fcntl error %s", strerror(errno));
        removeFromSet(client_sock, _recv_fd_pool);
        close(client_sock);
        return ;
    }

    Client  new_client(serv.getFd());
    new_client.setServer(serv);

    if (_clients_map.count(client_sock) != 0)
        _clients_map.erase(client_sock);
    _clients_map.insert(std::make_pair(client_sock, new_client));
}


/* Closes connection from fd i and remove associated client object from _clients_map */
void    ServerManager::closeConnection(const int i)
{
    if (_clients_map.count(i)) {
        Client& c = _clients_map[i];
        if (c.response.getCgiState() == 1) {
            if (FD_ISSET(c.response._cgi_obj.pipe_in[1], &_write_fd_pool))
                removeFromSet(c.response._cgi_obj.pipe_in[1], _write_fd_pool);
            if (FD_ISSET(c.response._cgi_obj.pipe_out[0], &_recv_fd_pool))
                removeFromSet(c.response._cgi_obj.pipe_out[0], _recv_fd_pool);
            close(c.response._cgi_obj.pipe_in[1]);
            close(c.response._cgi_obj.pipe_out[0]);
        }
    }

    if (FD_ISSET(i, &_write_fd_pool))
        removeFromSet(i, _write_fd_pool);
    if (FD_ISSET(i, &_recv_fd_pool))
        removeFromSet(i, _recv_fd_pool);
    close(i);
    _clients_map.erase(i);
}

/**
 * Build the response and send it to client.
 * If no error was found in request and Connection header value is keep-alive,
 * connection is kept, otherwise connection will be closed.
 */
void    ServerManager::sendResponse(const int &i, Client &c)
{
    if (!c.writer.hasDataToSend() && c.response.getCgiState() == 0) 
    {
        c.processRequest();

        if (c.response.getCgiState() == 1) 
        {
            handleReqBody(c); // Prepare the body to be sent to CGI
            addToSet(c.response._cgi_obj.pipe_in[1], _write_fd_pool);
            addToSet(c.response._cgi_obj.pipe_out[0], _recv_fd_pool);
            
            removeFromSet(i, _write_fd_pool);
            return;
        }
        
        c.writer.queueResponse(c.response);
    }
    
    if (c.writer.hasDataToSend())
    {
        ssize_t bytes_sent = c.writer.sendData();

        if (bytes_sent < 0) {
            closeConnection(i);
            return;
        }

        if (c.writer.isComplete()) {
            if (c.request.keepAlive()) {
                c.clearClient();
                removeFromSet(i, _write_fd_pool);
                addToSet(i, _recv_fd_pool);
            } else {
                closeConnection(i);
            }
        }
    }
}

// /* Assigen server_block configuration to a client based on Host Header in request and server_name*/
// void    ServerManager::assignServer(Client &c)
// {
//     for (std::vector<Server>::iterator it = _servers.begin();
//         it != _servers.end(); ++it)
//     {
//         if (c.server.getHost() == it->getHost() &&
//             c.server.getPort() == it->getPort() &&
//             c.request.getHeaderValue("host") == it->getServerName()[0])
//         {
//             c.setServer(*it);
//             return ;
//         }
//     }
// }

/**
 * - Reads data from client and feed it to the parser.
 * Once parser is done or an error was found in the request,
 * socket will be moved from _recv_fd_pool to _write_fd_pool
 * and response will be sent on the next iteration of select().
 */
void    ServerManager::readRequest(const int &i, Client &c)
{
    c.readAndParse();

    if (c.isParseComplete())
    {
        if (!c.request.hasError())
            c.findSetConfigs(_servers);
        
        removeFromSet(i, _recv_fd_pool);
        addToSet(i, _write_fd_pool);
    }
}

void    ServerManager::handleReqBody(Client &c)
{
    	if (c.request.getBody().length() == 0)
		{
			std::string tmp;
			std::fstream file;(c.response._cgi_obj.getCgiPath().c_str());
			std::stringstream ss;
			ss << file.rdbuf();
			tmp = ss.str();
			c.request.setBody(tmp);
		}
}

/* Send request body to CGI script */
void    ServerManager::sendCgiBody(Client &c, CgiHandler &cgi)
{
    int bytes_sent;
    std::string req_body = c.request.getBody();

    if (req_body.empty())
        bytes_sent = 0;
    else if (req_body.length() >= MESSAGE_BUFFER)
        bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), MESSAGE_BUFFER);
    else
        bytes_sent = write(cgi.pipe_in[1], req_body.c_str(), req_body.length());

    if (bytes_sent < 0)
    {
        removeFromSet(cgi.pipe_in[1], _write_fd_pool);
        close(cgi.pipe_in[1]);
        close(cgi.pipe_out[0]);
        c.response.setErrorResponse(500, c.getServer());
        c.response.setCgiState(2); // done (with an error)
        addToSet(c.getFd(), _write_fd_pool); // Add client back to write set to send error page
    }
    else if ((size_t)bytes_sent >= req_body.length())
    {
        removeFromSet(cgi.pipe_in[1], _write_fd_pool);
        close(cgi.pipe_in[1]);
    }
    else
    {
        c.updateTime();
        req_body = req_body.substr(bytes_sent);
        c.request.setBody(req_body); // Update the request body
    }
}

/* Reads outpud produced by the CGI script */
void    ServerManager::readCgiResponse(Client &c, CgiHandler &cgi)
{
    char    buffer[MESSAGE_BUFFER * 2];
    memset(buffer, 0, sizeof(buffer));
    int     bytes_read = 0;
    bytes_read = read(cgi.pipe_out[0], buffer, MESSAGE_BUFFER * 2);

    if (bytes_read == 0)
    {
        // CGI EOF
        removeFromSet(cgi.pipe_out[0], _recv_fd_pool);
        close(cgi.pipe_out[0]);
		int status;
		waitpid(cgi.getCgiPid(), &status, 0);
		if(WIFEXITED(status) && WEXITSTATUS(status) != 0)
		{
			c.response.setErrorResponse(502, c.getServer()); // Bad Gateway
		}
        c.response.setCgiState(2); // Mark CGI as complete
        
        c.response.buildResponseFromCgi();
        
        addToSet(c.getFd(), _write_fd_pool);
        return;
    }
    else if (bytes_read < 0)
    {
        removeFromSet(cgi.pipe_out[0], _recv_fd_pool);
        close(cgi.pipe_out[0]);
        c.response.setCgiState(2);
        c.response.setErrorResponse(500, c.getServer());
        addToSet(c.getFd(), _write_fd_pool);
        return;
    }
    else
    {
        c.updateTime();
		c.response._response_content.append(buffer, bytes_read);
    }
}

void	ServerManager::addToSet(const int i, fd_set &set)
{
    FD_SET(i, &set);
    if (i > _biggest_fd)
        _biggest_fd = i;
}

void	ServerManager::removeFromSet(const int i, fd_set &set)
{
    FD_CLR(i, &set);
    if (i == _biggest_fd)
    {
        _biggest_fd = 0;
        for(int j=0; j < i; ++j) {
            if(FD_ISSET(j, &_recv_fd_pool) || FD_ISSET(j, &_write_fd_pool))
                _biggest_fd = j;
        }
    }
}