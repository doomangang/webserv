#include "../inc/Logger.hpp"


std::string Logger::file_name = "logfile.txt";
LogPrio Logger::prio = ERROR;

void    Logger::logMsg(LogPrio p, Mode m, const char* msg, ...)
{
    char        output[8192];
    va_list     args;
    int         n;

    va_start(args, msg);
    n = vsnprintf(output, 8192, msg, args);
    std::string date = getCurrTime();
    if (Logger::prio >= p)
    {
        if (m == FILE_OUTPUT)
        {
            if (mkdir("./logs", 0664) < 0 && errno != EEXIST)
            {
                std::cerr << "mkdir() Error: " << strerror(errno) << std::endl;
                return ;
            }
            int fd = open(("./logs/" + file_name).c_str(), O_CREAT | O_APPEND | O_RDWR, 0664);
            write(fd, date.c_str(), date.length());
            write(fd, "   ", 3);
            write(fd, output, n);
            write(fd, "\n", 1);
            close(fd);
        }
        else if (m == CONSOLE_OUTPUT)
        {
            if (p == DEBUG)
                std::cout << MAGENTA;
            else if (p == INFO)
                std::cout << CYAN;
            else if (p == ERROR)
                std::cout << RED;
            std::cout << getCurrTime()  << output << RESET << std::endl;
        }      
    }
    va_end(args);
}

std::string Logger::getCurrTime()
{
    char date[1000];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "[%Y-%m-%d  %H:%M:%S]   ", &tm);
    return (std::string(date));
}

void Logger::setPrio(LogPrio p)
{
    Logger::prio = p;
}

void Logger::setFilenName(std::string name)
{
    Logger::file_name = name;
}

