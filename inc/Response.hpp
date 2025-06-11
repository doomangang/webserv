#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "Webserv.hpp"
#include "Request.hpp"
#include "CgiHandler.hpp"
#include "HttpUtils.hpp"

// Forward declaration to avoid circular includes
class Server;

class Response {
public:
	/* Orthodox Canonical Form (OCF) */
	Response();
	Response(const Response& other);
	~Response();
	Response& operator=(const Response& other);

	/* Member variables */
	CgiHandler	_cgi_obj;
	std::string _response_content; // Used to buffer raw CGI output

	/* Core Methods */
	void buildResponseFromCgi();
	void clear();
	void setErrorResponse(short code, const Server& server);
	std::string toString() const;

	/* Getter & Setter */
	int getCgiState() const;
	void setCgiState(int state);

	void setStatusCode(int code);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	std::string getHeaderValue(const std::string& key) const;

private:
	int							_code;
	std::string					_status_description;
	std::map<std::string, std::string> _headers;
	std::string					_body;
	int							_cgi_state; // 0: NoCGI, 1: CGI Pending, 2: CGI Done

	void setDefaultHeaders();
};

#endif
