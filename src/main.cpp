#include "../inc/Webserv.hpp"
#include "../inc/ServerManager.hpp"
#include "../inc/ConfigParser.hpp"
#include "../inc/Config.hpp"

void sigpipeHandle(int sig) { if(sig) {}}

int main(int argc, char **argv, char** envp) {
    if (argc == 1 || argc == 2) {
        try {
            std::string configPath;
            ConfigParser    parser;
            ServerManager   manager;

            signal(SIGPIPE, sigpipeHandle);
            configPath = argc == 1 ? "./configs/default.conf" : argv[1];
            parser.loadConfigFile(configPath);
            Logger::logMsg(INFO, CONSOLE_OUTPUT, "Config file loaded successfully");
            
            Config config = parser.parseConfigFile(configPath, envp);
            Logger::logMsg(INFO, CONSOLE_OUTPUT, "Config parsed successfully");
            
            std::vector<Server> servers = config.getServers();            
            manager.setupServers(servers);
            Logger::logMsg(INFO, CONSOLE_OUTPUT, "Servers setup completed, starting main loop...");
            
            manager.runServers();
        }
        catch (const std::exception& e) {
            std::cerr << RED << "Error: " << e.what() << RESET << std::endl;
            return 1;
        }
        catch (...) {
            std::cerr << RED << "An unknown error occurred." << RESET << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << RED << "Usage: " << argv[0] << " [config_file_path]" << RESET << std::endl;
        return 1;
    }
    
}