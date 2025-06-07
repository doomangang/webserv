#ifndef CGI_HANDLER_HPP
#define CGI_HANDLER_HPP

#include "Webserv.hpp"

class CgiHandler {
private:
    std::map<std::string, std::string> _env;
    std::string _scriptPath;
    std::vector<std::string> _args;
    std::string _inputData;

    // Helper to prepare environment for execve
    char** buildEnvArray() const;
    char** buildArgvArray() const;
    void freeArray(char** arr, size_t size) const;
public:
    CgiHandler();
    ~CgiHandler();

    // Set CGI environment variables
    void setEnv(const std::map<std::string, std::string>& env);

    // Set CGI script path and arguments
    void setScript(const std::string& scriptPath, const std::vector<std::string>& args);

    // Set input data for CGI (e.g., POST body)
    void setInput(const std::string& inputData);

    // Execute the CGI script and get output
    int execute(std::string& output, int timeoutSec = 5);

};

#endif // CGI_HANDLER_HPP