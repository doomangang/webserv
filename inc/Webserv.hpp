#pragma once
#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include <iostream>
// # include <fstream>
# include <fcntl.h>
# include <cstring>
# include <string> 
# include <unistd.h>
# include <dirent.h>
# include <sstream>
// # include <bits/stdc++.h>
# include <cstdlib>
# include <fstream>
# include <sstream>
# include <cctype>
# include <ctime>
# include <cstdarg>

/* STL Containers */
# include <map>
# include <set>
# include <vector>
# include <algorithm>
# include <iterator>
# include <list>
// # include <unordered_map>
# include <queue>

/* System */
# include <sys/types.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <unistd.h>
// # include <machine/types.h>
# include <signal.h>

/* Network */
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
// # include <sys/epoll.h>
# include <arpa/inet.h>

// # include "Config.hpp"
// # include "Server.hpp"
// # include "Location.hpp"
// # include "Request.hpp"
// # include "Response.hpp"
// # include "RequestParser.hpp"
// # include "ResponseWriter.hpp"
// # include "Client.hpp"
// # include "CgiHandler.hpp"
// # include "Mime.hpp"
# include "Logger.hpp"
# include "HttpUtils.hpp"
# include "HttpTypes.hpp"

/* Color Sets */
#ifndef COLORS_DEFINED
#define COLORS_DEFINED
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
#endif

#define CONNECTION_TIMEOUT 60 // Time in seconds before client get kicked out if no data was sent.
#ifdef TESTER
    #define MESSAGE_BUFFER 40000 
#else
    #define MESSAGE_BUFFER 40000
#endif

#define MAX_URI_LENGTH 4096
#define MAX_CONTENT_LENGTH 30000000


/* 아래 함수들은 httpUtils.h 로 이동 */
// template <typename T>
// std::string toString(const T val)
// {
//     std::stringstream stream;
//     stream << val;
//     return stream.str();
// }

/* Utils.c */

// std::string statusCodeString(short);
// std::string getErrorPage(short);
// int buildHtmlIndex(std::string &, std::vector<uint8_t> &, size_t &);
// int ft_stoi(std::string str);
// unsigned int fromHexToDec(const std::string& nb);


#endif