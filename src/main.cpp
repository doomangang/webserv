#include "../inc/ServerManager.hpp"

int main(int argc, char **argv) {
    if (argc == 1 || argc == 2) {
        try {
            std::string configPath;
            ServerManager   manager;

            configPath = argc == 1 ? "../configs/default.conf" : argv[1];
        }
    }
}