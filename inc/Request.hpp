#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Webserv.hpp"

class Server;
class Location;

class Request {
public:

	/* Orthodox Canonical Form */
	Request();
	Request(const Request& other);
	~Request();
	Request& operator=(const Request& other);

	/* setters */
	void										setMethod(const std::string& method_str);
	void										setUrl(const std::string& url_str);
	void										setVersion(const std::string& version_str);
	void										setErrorCode(int code);
	void										setStatus(ParseState state);
	void										setBody(std::string&);
	void										setBytesToRead(ssize_t bytes);
	void										reserveBody(ssize_t size_hint);

	/* getters */
	Method										getMethod()   const;
	const std::string&				   			getUrl()	  const;
	const std::string&				   			getVersion()  const;
	const std::map<std::string,std::string>&	getHeaders() const;
	const std::string&				   			getBody()	 const;
	size_t										getBodyPos() const;
	ssize_t										getBytesToRead() const;
	ParseState									getStatus()   const;
	int								   			getErrorCode()const;

	/* request line */
	void										addHeader(const std::string& key, const std::string& value);
	void										appendBody(const std::string& chunk);
	void										cutBody(ssize_t size);
	void										addBodyPos(size_t);
	const std::string&							findValueInHeader(const std::string& key) const;
	
	/* utility */
	void cleaner();

private:
	Method										_method;
	std::string				  					_url;
	std::string				  					_version;
	std::map<std::string,std::string> 			_headers;
	std::string				  					_body;
	ParseState				   					_status;
	int						  					_error_code;
	size_t										_body_pos;
	ssize_t										_bytes_to_read;
};

#endif // REQUEST_HPP
