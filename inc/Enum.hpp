#ifndef ENUM_HPP
#define ENUM_HPP

// HTTP 메서드
enum Method {
    GET,
    POST,
    DELETE,
    EMPTY,
    UNKNOWN
};

enum Progress {
    FROM_CLIENT,
    CGI,
    FROM_FILE,
    TO_CLIENT,
    END_CONNECTION,
    READ_CONTINUE,
    COMBINE
}
enum Incomplete {
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