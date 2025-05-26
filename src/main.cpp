#include "../inc/ServerManager.hpp"

int main(int argc, char **argv) {
    if (argc == 1 || argc == 2) {
        try {
            std::string configPath;
            ServerManager   manager;

            configPath = argc == 1 ? "../configs/default.conf" : argv[1];
            if (!manager.loadConfig(configPath)) {
                std::cerr << RED << "Failed to load configuration from " << configPath << RESET << std::endl;
                return 1;
            }
            
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