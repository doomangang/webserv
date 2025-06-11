#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "Webserv.hpp"
#include "HttpTypes.hpp"

class Location {
private:
    /* member attributes */
    std::string              _uri; // _path
    std::string              _root_path;
    std::set<Method>         _allow_methods;
    std::set<std::string>    _index_files;
    std::set<std::string>    _cgi_extensions;
    std::set<std::string> _cgi_path;
    bool                     _autoindex;
    bool                     _has_redirect;
    int                      _redirect_code;
    std::string              _redirect_url;
    bool                     _has_upload_store;
    std::string              _upload_store;
    int                      _client_body_size;

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

    std::set<Method> getAllowMethods() const;
    void setAllowMethods(const std::set<Method>& methods);
    void addAllowMethod(Method m);
    void clearAllowMethods();

    std::set<std::string> getIndexFiles() const;
    void setIndexFiles(const std::vector<std::string>& files);

    std::set<std::string> getCgiExtensions() const;
    void setCgiExtensions(const std::set<std::string>& exts);

    std::set<std::string> getCgiPath() const;
    void setCgiPath(const std::set<std::string>& exts);

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

    int getClientBodySize() const;
    void setClientBodySize(int size);
    /* additional methods */

    /* exception classes */
};

/* operators */

#endif
