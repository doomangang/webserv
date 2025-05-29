#include "../inc/Location.hpp"

Location::Location() {
}

Location::Location(std::string& location_block) {
    (void) location_block;
}

Location::Location(const Location& other) {
    *this = other;
}

Location::~Location() {
}

Location& Location::operator=(const Location& other) {
    if (this != &other) {
        (void) other;
    }
    return *this;
}

	std::string                         Location::getUri()            const { return _uri;}
	std::string                         Location::getRootPath()       const { return _root_path;}
	std::set<std::string>               Location::getAllowMethod()    const { return _allow_method;}
	std::string                         Location::getAuthBasicRealm() const { return _auth_basic_realm;}
	std::map<std::string, std::string>  Location::getAuthBasicFile()  const { return _auth_basic_file;}
	std::set<std::string>               Location::getIndex()          const { return _index;}
	std::set<std::string>               Location::getCgi()            const { return _cgi;}
	bool                                Location::getAutoIndex()      const { return _auto_index;}
	void                         		Location::setUri(std::string uri) { _uri = uri; }
	void                         		Location::setRootPath(std::string, bool) {}
	void               					Location::setAllowMethod(std::Location::set<std::string>)    {}
	void                         		Location::setAuthBasicRealm() {}
	void  								Location::setAuthBasicFile()  {}
	void               					Location::setIndex()          {}
	void               					Location::setCgi()            {}
	void                                Location::setAutoIndex()      {}