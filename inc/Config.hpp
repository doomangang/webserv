#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Webserv.hpp"

class Config {
private:
    /* member attributes */
    std::string _software_name;
    std::string _software_version;
    std::string _http_version;
    std::string _cgi_version;
    char**      _base_env;
    Config();
    
public:
    /* Orthodox Canonical Form (OCF) */
    Config(std::string& config_block, char* envp[]);
    Config(const Config& other);
    ~Config();
    Config& operator=(const Config& other);

    /* getter & setter */
    std::string getSoftwareName()   const;
    std::string getSoftwareVersion()const;
    std::string getHttpVersion()    const;
    std::string getCgiVersion()     const;
    char**      getBaseEnv()        const;
    
    /* additional methods */

    /* exception classes */
    class ErrorException : public std::exception
    {
        private:
            std::string _message;
        public:
            ErrorException(std::string message) throw()
            {
                _message = "CONFIG PARSER ERROR: " + message;
            }
            virtual const char* what() const throw()
            {
                return (_message.c_str());
            }
            virtual ~ErrorException() throw() {}
    };
};

/* operators */
#endif