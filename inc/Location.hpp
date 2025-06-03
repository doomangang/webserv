// Location.hpp

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <string>
#include "Enum.hpp"

class Location {
private:
    /* member attributes */
    std::string              _uri;
    std::string              _root_path;
    std::set<enum Method>    _allow_methods;
    std::set<std::string>    _index_files;
    std::set<std::string>    _cgi_extensions;
    bool                     _autoindex;
    bool                     _has_redirect;
    int                      _redirect_code;
    std::string              _redirect_url;
    bool                     _has_upload_store;
    std::string              _upload_store;

public:
    /* Orthodox Canonical Form (OCF) */
    Location();
    Location(const std::string& location_block);
    Location(const Location& other);
    ~Location();
    Location& operator=(const Location& other);

    /* getter & setter */
    std::string getUri() const;
    void setUri(const std::string& uri);

    std::string getRootPath() const;
    void setRootPath(const std::string& path);

    std::set<enum Method> getAllowMethods() const;
    void setAllowMethods(std::set<enum Method>& methods);
    void addAllowMethod(Method m);
    void clearAllowMethods();

    std::set<std::string> getIndexFiles() const;
    void setIndexFiles(std::vector<std::string>& files);

    std::set<std::string> getCgiExtensions() const;
    void setCgiExtensions(std::set<std::string>& exts);

    bool getAutoindex() const;
    void setAutoindex(bool onoff);

    bool hasRedirect() const;
    int getRedirectCode() const;
    std::string getRedirectUrl() const;
    void setRedirect(int code, const std::string& url);

    bool hasUploadStore() const;
    std::string getUploadStore() const;
    void setHasUploadStore(bool has);
    void setUploadStore(const std::string& path);

    /* additional methods */

    /* exception classes */
};

/* operators */

#endif  // LOCATION_HPP
