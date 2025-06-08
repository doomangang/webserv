#include "../inc/Webserv.hpp"
#include "../inc/ServerManager.hpp"
#include "../inc/ConfigParser.hpp"

void sigpipeHandle(int sig) { if(sig) {}}

int main(int argc, char **argv, char** envp) {
    if (argc == 1 || argc == 2) {
        try {
            std::string configPath;
            ConfigParser    parser;

            signal(SIGPIPE, sigpipeHandle);

            configPath = argc == 1 ? "./configs/default.conf" : argv[1];
            parser.loadConfigFile(configPath);
            ServerManager   manager(parser.parseConfigFile(configPath, envp));
            //create server
            manager.setupServers(manager.getConfig().getServers());
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