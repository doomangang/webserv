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

enum Incomplete {
    NONE,                    
    REQUEST_LINE_INCOMPLETE, 
    HEADERS_INCOMPLETE,      
    BODY_INCOMPLETE,
    TRAILER_INCOMPLETE,        
    COMPLETE,           
    BAD_REQUEST      
};

#endif