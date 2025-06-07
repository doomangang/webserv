#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Webserv.hpp"

class Location {
private:
	/* member attributes */
	std::string                         _uri;
	std::string                         _root_path;
	std::set<std::string>               _allow_method;
	std::string                         _auth_basic_realm;
	std::map<std::string, std::string>  _auth_basic_file;
	std::set<std::string>               _index;
	std::set<std::string>               _cgi;
	bool                                _autoindex;
	Location();

public:
	/* Orthodox Canonical Form (OCF) */
	Location(std::string& location_block);
	Location(const Location& other);
	~Location();
	Location& operator=(const Location& other);

	/* getter & setter */
	std::string                         getUri()            const;
	std::string                         getRootPath()       const;
	std::set<std::string>               getAllowMethod()    const;
	std::string                         getAuthBasicRealm() const;
	std::map<std::string, std::string>  getAuthBasicFile()  const;
	std::set<std::string>               getIndex()          const;
	std::set<std::string>               getCgi()            const;
	bool                                getAutoIndex()      const;
	/* additional methods */

	/* exception classes */
};

/* operators */
#endif