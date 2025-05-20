#include "../inc/Location.hpp"

Location::Location() {
    std::cout << GREEN << "Location default constructor called\n" << RESET << std::endl;
}

Location::Location(std::string& location_block) {
    
}

Location::Location(const Location& other) {
    *this = other;
    std::cout << GREEN << "Location copy constructor called\n" << RESET << std::endl;
}

Location::~Location() {
    std::cout << RED << "Location destructor called\n" << RESET << std::endl;
}

Location& Location::operator=(const Location& other) {
    std::cout << YELLOW << "Location assignment operator called\n" << RESET << std::endl;
    if (this != &other) {
        // assignment code here
    }
    return *this;
}