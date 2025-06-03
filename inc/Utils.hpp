#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <vector>
#include <sstream>
#include "Server.hpp"

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

class Utils {
private:
    /* member attributes */
    /* Orthodox Canonical Form (OCF) */
    Utils();
    Utils(std::vector<Server>, char* envp[]);
    Utils(const Utils& other);
    ~Utils();
    Utils& operator=(const Utils& other);
public:
    /* getter & setter */
    
    /* additional methods */
    static void trim(std::string& s);
    static std::vector<std::string>    splitBySemicolon(const std::string& s);
    static std::vector<std::string>    splitWords(const std::string& s);
    /* exception classes */
};

/* operators */
#endif