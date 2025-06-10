#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "Webserv.hpp"
#include "Server.hpp"
#include "Location.hpp"
#include "HttpTypes.hpp"

class Server;
class Location;

class Request {
private:
    /* member attributes */
    Connection*                         _connection;
    Server*                             _server;
    Location*                           _location;
    time_t                             _start_at;
    Method                              _method;
    std::string                         _uri;
    URIType                             _uri_type;
    std::map<std::string, std::string>  _headers;
    TransferType                        _transfer_type;
    std::string                         _content;
    std::string                         _origin;

public:
    /* Orthodox Canonical Form (OCF) */
    Request();
    Request(Connection*, Server*, std::string& start_line);
    Request(const Request& other);
    ~Request();

    void                 setMethod(const std::string &method_str);
    void                 setUrl(const std::string &url_str);
    void                 setVersion(const std::string &ver_str);
    /* getter & setter */
    Connection*                         getConnection()const;
    Server*                             getServer()const;
    Location*                           getLocation()const;
    Method*                             getMethod()const;
    std::string                         getUri()const;
    URIType                             getUriType()const;
    std::map<std::string,std::string>   getHeaders()const;
    TransferType                        getTransferType()const;
    std::string                         getContent()const;
    std::string                         getOrigin()const;
    
    /* additional methods */
    bool        isOverTime(const time_t& now)const;
    void        addContent(const std::string&);
    void        addOrigin(const std::string&);
    void        addHeader(const std::string& header_line);
    bool        isValidHeader(const std::string& header_line) const;

    //header
    bool                 parseHeaderLine(const std::string &header_line);
    bool                 hasHeader(const std::string &key) const;
    std::string          getHeaderValue(const std::string &key) const;
    const std::map<std::string, std::string>& GetAllHeaders() const;
    void                 addHeader(const std::string &key, const std::string &value);

    //body
    void                 setBody(const std::string &body_str);
    void                 reserveBody(ssize_t size_hint);
    void                 setBytesToRead(ssize_t bytes);
    void                 addBodyChunk(const std::string &chunk);
    void                 cutBody(ssize_t size);

    //getter & setter
    Method               getMethod() const;
    const std::string&   getUrl() const;
    const std::string&   getVersion() const;
    size_t               getBodyPos() const;
    const std::string&   getBody() const;
    Incomplete           getStatus() const;
    ssize_t              getBytesToRead() const;
    int                  getErrorCode() const;

    void                 setStatus(Incomplete type);
    void                 setErrorCode(int code);
    bool                 hasError() const;
    void                 addBodyPos(size_t n);


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
	std::string 								getQueryParam(const std::string&) const;

	/* request line */
	void										addHeader(const std::string& key, const std::string& value);
	bool										hasHeader(const std::string& key) const;
	std::string 								getHeaderValue(const std::string& key) const;
	const std::string& 							getPath() const;
	const std::string& 							getQueryString() const;
	const std::string& 							getFragment() const;

	void										cutBody(ssize_t size);
	void										addBodyPos(size_t);
	void										addBodyChunk(const std::string& chunk);
	
	void										parseUri();
	void										parseQueryString();
	bool 										parseHeaderFields(const std::string& line);

	/* utility */
	bool										hasError() const;
	void										cleaner();

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

	//Url
	std::string									_path;
	std::string									_fragment;
	std::string									_query_string;
	std::map<std::string,std::string>			_query_params;
	
};

#endif // REQUEST_HPP
