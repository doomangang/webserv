// RequestParser.hpp
#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <string>
#include <vector>
#include "Request.hpp"
#include "Utils.hpp"
#include "Enum.hpp"

/* Color Sets */
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define GREY    "\033[38;5;250m"

class Request;

class RequestParser
{
public:
    RequestParser();
    ~RequestParser();
    RequestParser(const RequestParser &copy);
    RequestParser& operator=(const RequestParser &rhs);

    /* getter & setter */
    Incomplete  getParseState() const;
    std::string& getRawBuffer();

    void        setMaxBodySize(size_t size);
    void        setMaxHeaderSize(size_t size);

    /* state checker */
    bool isRequestLineComplete() const;
    bool isHeadersComplete() const;
    bool isParsingComplete() const;
    bool isBadRequest() const;
    bool isChunked() const;

    /* parse methods */
    void parseRequestLine(Request& request);
    void parseHeaders(Request& request);
    void parseBody(Request& request);
    void parseChunkedBody(Request& request);
    bool parseChunkSize(const std::string& line);

    void reset();

private:
    Incomplete _parse_state;
    size_t _max_body_size;
    size_t _max_header_size;
    size_t _expected_body_size;
    size_t _received_body_size;
    size_t _header_end_pos;
    std::string _raw_buffer;

    bool _is_chunked;
    size_t _current_chunk_size;
    size_t _current_chunk_received;
    enum ChunkState {
        CHUNK_SIZE,
        CHUNK_DATA,
        CHUNK_TRAILER
    } _chunk_state;

    void validateHeaderValues(Request& request);

};

#endif // REQUESTPARSER_HPP
