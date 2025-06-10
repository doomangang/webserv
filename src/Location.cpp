// Location.cpp

#include "../inc/Location.hpp"

/* OCF: 기본 생성자 */
Location::Location()
	: _uri(""),
	  _root_path(""),
	  _allow_methods(),
	  _index_files(),
	  _cgi_extensions(),
	  _autoindex(false),
	  _has_redirect(false),
	  _redirect_code(0),
	  _redirect_url(""),
	  _has_upload_store(false),
	  _upload_store()
{
}

/* location_block 기반 생성자(필요하다면 파싱 로직을 추가) */
Location::Location(const std::string& location_block)
	: _uri(""),
	  _root_path(""),
	  _allow_methods(),
	  _index_files(),
	  _cgi_extensions(),
	  _autoindex(false),
	  _has_redirect(false),
	  _redirect_code(0),
	  _redirect_url(""),
	  _has_upload_store(false),
	  _upload_store()
{ // TODO: location_block 파싱 로직이 필요한 경우 여기에 추가
	(void)location_block; }

/* 복사 생성자 */
Location::Location(const Location& other)
	: _uri(other._uri),
	  _root_path(other._root_path),
	  _allow_methods(other._allow_methods),
	  _index_files(other._index_files),
	  _cgi_extensions(other._cgi_extensions),
	  _autoindex(other._autoindex),
	  _has_redirect(other._has_redirect),
	  _redirect_code(other._redirect_code),
	  _redirect_url(other._redirect_url),
	  _has_upload_store(other._has_upload_store),
	  _upload_store(other._upload_store)
{
}

/* 소멸자 */
Location::~Location() {
}

/* 대입 연산자 */
Location& Location::operator=(const Location& other) { 
	if (this != &other) {     
		_uri              = other._uri;
		_root_path        = other._root_path;
		_allow_methods    = other._allow_methods;
		_index_files      = other._index_files;
		_cgi_extensions   = other._cgi_extensions;
		_autoindex        = other._autoindex;
		_has_redirect     = other._has_redirect;
		_redirect_code    = other._redirect_code;
		_redirect_url     = other._redirect_url;
		_has_upload_store = other._has_upload_store;
		_upload_store     = other._upload_store;
	}
	return *this; 
}

/* getter 구현 */
std::string Location::getUri() const { return _uri; }

std::string Location::getRootPath() const { return _root_path; }

std::set<Method> Location::getAllowMethods() const { return _allow_methods; }

std::set<std::string> Location::getIndexFiles() const { return _index_files; }

std::set<std::string> Location::getCgiExtensions() const { return _cgi_extensions; }

bool Location::getAutoindex() const { return _autoindex; }

bool Location::hasRedirect() const { return _has_redirect; }

int Location::getRedirectCode() const { return _redirect_code; }

std::string Location::getRedirectUrl() const { return _redirect_url; }

bool Location::hasUploadStore() const { return _has_upload_store; }

std::string Location::getUploadStore() const { return _upload_store; }

/* setter 구현 */
void Location::setUri(const std::string& uri) { _uri = uri; }

void Location::setRootPath(const std::string& path) { _root_path = path; }

void Location::setAllowMethods(std::set<Method>& methods) { _allow_methods = methods; }

void Location::addAllowMethod(Method m) { _allow_methods.insert(m); }

void Location::clearAllowMethods() { _allow_methods.clear(); }

void Location::setIndexFiles(std::vector<std::string>& files) {
	_index_files.clear();
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
		_index_files.insert(*it);
	}
}

void Location::setCgiExtensions(std::set<std::string>& exts) { _cgi_extensions = exts; }

void Location::setAutoindex(bool onoff) { _autoindex = onoff; }

void Location::setRedirect(int code, const std::string& url) { 
	_has_redirect  = true;
	_redirect_code = code;
	_redirect_url  = url; 
}

void Location::setHasUploadStore(bool has) { _has_upload_store = has; }

void Location::setUploadStore(const std::string& path) { 
	_has_upload_store = true;
	_upload_store     = path; 
}
