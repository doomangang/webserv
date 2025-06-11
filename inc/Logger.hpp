#ifndef LOGGER_HPP
#define LOGGER_HPP

# include "Webserv.hpp"

enum LogPrio{
    DEBUG,
    INFO,
    ERROR
};

enum Mode{
    CONSOLE_OUTPUT,
    FILE_OUTPUT
};

class Logger{

    public:
        static std::string file_name;
        static LogPrio prio;
        static std::map<LogPrio, std::string> prio_str;

        static void         setFilenName(std::string);
        static void         logMsg(LogPrio p, Mode, const char*, ...);
        static void         setPrio(LogPrio);
        static void         enableFileLog();
        static std::string  getCurrTime();

};


#endif