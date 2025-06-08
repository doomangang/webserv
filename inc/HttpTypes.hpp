#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

enum Method {
    GET,
    POST,
    DELETE,
    HEAD,
    OPTIONS,
    UNKNOWN_METHOD
};

enum TransferType { GENERAL, CHUNKED };

enum HttpVersion {
    HTTP_1_0,
    HTTP_1_1,
    UNKNOWN_VERSION
};

enum ConnectionState {
    FROM_CLIENT,
	CGI,
	FROM_FILE,
	TO_CLIENT,
	END_CONNECTION,
	READ_CONTINUE,
	COMBINE
};

enum ParseState {
    NONE,                    
    REQUEST_LINE_INCOMPLETE, 
    REQUEST_LINE_COMPLETE,
    HEADERS_INCOMPLETE,      
    HEADERS_COMPLETE,      
    BODY_INCOMPLETE,
    TRAILER_INCOMPLETE,        
    COMPLETE,           
    BAD_REQUEST  
};

#endif
