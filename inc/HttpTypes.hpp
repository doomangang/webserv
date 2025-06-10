#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

enum Method {
    GET,
    POST,
    DELETE,
    EMPTY,
    UNKNOWN_METHOD
};

enum URIType { DIRECTORY, FILE, CGI_PROGRAM };
enum TransferType { GENERAL, CHUNKED };

enum ChunkState {
    CHUNK_SIZE,      // 청크 크기를 읽는 상태
    CHUNK_DATA,      // 청크 데이터를 읽는 상태
    CHUNK_TRAILER    // 마지막 청크 후 트레일러를 읽는 상태
};

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

enum SetType { WRITE_SET, WRITE_COPY_SET,
                    READ_SET, READ_COPY_SET,
                    ERROR_SET, ERROR_COPY_SET };

#endif
