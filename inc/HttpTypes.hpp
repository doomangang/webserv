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
    READ_STARTLINE,
	READ_HEADER,
	READ_BODY,
	READ_TRAILER,
	READ_DONE,
	BAD_REQUEST
};

#endif
