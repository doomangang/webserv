// RequestParser.hpp
#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include "Webserv.hpp"

class Request;

class RequestParser {
public:
    /* Orthodox Canonical Form */
    RequestParser();
    RequestParser(const RequestParser& other);
    ~RequestParser();
    RequestParser& operator=(const RequestParser& other);

    /* parse steps */
    void        parseRequestLine(Request& request);
    void        parseHeaders(Request& request);
    void        parseBody(Request& request);

    /* configure limits */
    void        setMaxBodySize(size_t size);
    void        setMaxHeaderSize(size_t size);

    /* parsing status */
    bool        isRequestLineComplete() const;
    bool        isHeadersComplete()     const;
    bool        isParsingComplete()     const;
    bool        isBadRequest()          const;
    ParseState  getParseState()         const;

    /* raw buffer access */
    std::string& getRawBuffer();

    /* reset for next request */
    void        reset();

private:
    ParseState  _parse_state;
    size_t      _max_body_size;
    size_t      _max_header_size;
    std::string _raw_buffer;

    TransferType _is_chunked;
    size_t      _expected_body_size;
    size_t      _received_body_size;
    size_t      _header_end_pos;
};

#endif // REQUESTPARSER_HPP
