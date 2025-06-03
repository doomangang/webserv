#ifndef LOGGER_HPP
#define LOGGER_HPP

# include "Webserv.hpp"

/* Not used currently */
enum LogPrio{
    DEBUG,
    INFO,
    ERROR
};

enum L_State{
    ON,
    OFF
};


enum Mode{
    CONSOLE_OUTPUT,
    FILE_OUTPUT
};

enum ErrorType{
    SYSCALL_ERROR,
    LOG_ERROR,
    PARSE_ERROR,
    CGI_ERROR,
    SERVER_ERROR
};

class Logger{

    public:
        static std::string file_name;
        static LogPrio prio;
        static std::map<LogPrio, std::string> prio_str;
        static L_State state;

        static void         setFilenName(std::string);
        static void         setState(L_State);

        static void         logMsg(const char *, Mode, const char*, ...);
        static void         setPrio(LogPrio);
        static void         enableFileLog();
        static std::string  getCurrTime();
    private:
        static std::map<LogPrio, std::string> initMap();

};


#endif