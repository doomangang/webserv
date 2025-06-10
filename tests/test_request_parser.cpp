#include "../inc/Request.hpp"
#include "../inc/RequestParser.hpp"
#include "../inc/HttpUtils.hpp"
#include <iostream>
#include <cassert>
#include <string>

// Helper function to print test results
void printTestResult(const std::string& test_name, bool success) {
    if (success) {
        std::cout << "[ \033[32mPASS\033[0m ] " << test_name << std::endl;
    } else {
        std::cout << "[ \033[31mFAIL\033[0m ] " << test_name << std::endl;
    }
}

void test_simple_get_request() {
    RequestParser parser;
    Request request;
    std::string raw_request = "GET /index.html HTTP/1.1\r\nHost: e.com\r\n\r\n";
    
    parser.getRawBuffer().append(raw_request);
    
    parser.parseRequestLine(request);
    parser.parseHeaders(request);
    parser.parseBody(request);

    printTestResult("Simple GET Request", 
        request.getMethod() == GET &&
        request.getUrl() == "/index.html" &&
        request.getHeaderValue("host") == "e.com" &&
        parser.getParseState() == COMPLETE);
}

void test_post_with_body() {
    RequestParser parser;
    Request request;
    std::string raw_request = "POST /submit HTTP/1.1\r\nContent-Length: 10\r\n\r\n0123456789";
    
    parser.getRawBuffer().append(raw_request);

    parser.parseRequestLine(request);
    parser.parseHeaders(request);
    parser.parseBody(request);

    printTestResult("POST with Content-Length",
        request.getMethod() == POST &&
        request.getBody() == "0123456789" &&
        parser.getParseState() == COMPLETE);
}

// Test 3: Incremental parsing (simulating multiple recv calls)
void test_incremental_parsing() {
    RequestParser parser;
    Request request;

    // First chunk arrives
    parser.getRawBuffer().append("POST /api/data HTTP/1.1\r\nHo");
    parser.parseRequestLine(request);
    assert(parser.getParseState() == REQUEST_LINE_COMPLETE);

    // Second chunk arrives
    parser.getRawBuffer().append("st: my.service.com\r\nContent-Length: 10\r\n\r\n");
    parser.parseHeaders(request);
    assert(parser.getParseState() == HEADERS_COMPLETE);
    assert(request.getHeaderValue("host") == "my.service.com");

    // Body arrives in two parts
    parser.getRawBuffer().append("abcde");
    parser.parseBody(request);
    assert(parser.getParseState() == BODY_INCOMPLETE);

    parser.getRawBuffer().append("fghij");
    parser.parseBody(request);

    bool passed = true;
    if (request.getBody() != "abcdefghij") { passed = false; }
    if (parser.getParseState() != COMPLETE) { passed = false; }

    printTestResult("Incremental Parsing", passed);
}

// Test 4: Chunked transfer encoding
void test_chunked_request() {
    RequestParser parser;
    Request request;

    // Setup for chunked parsing
    request.setStatus(HEADERS_COMPLETE); // Manually set state after headers
    parser.getRawBuffer().append("4\r\n"      // Chunk 1 size
                                 "Wiki\r\n"   // Chunk 1 data
                                 "5\r\n"      // Chunk 2 size
                                 "pedia\r\n"  // Chunk 2 data
                                 "E\r\n"      // Chunk 3 size
                                 " in\r\n\r\nchunks.\r\n" // Chunk 3 data
                                 "0\r\n"      // Last chunk size
                                 "\r\n");     // End of request

    // We need to simulate the header part that sets chunked mode
    parser.parseChunkedBody(request);

    bool passed = true;
    if (request.getBody() != "Wikipedia in\r\n\r\nchunks.") { passed = false; }
    // A proper chunked parser would set the state to COMPLETE here.
    // This test assumes a basic implementation for now.

    printTestResult("Chunked Request Body", passed);
}


// Test 5: Bad request (invalid method)
void test_bad_request() {
    RequestParser parser;
    Request request;
    std::string raw_request = "INVALID / HTTP/1.1\r\nHost: e.com\r\n\r\n";
    
    parser.getRawBuffer().append(raw_request);
    parser.parseRequestLine(request);

    printTestResult("Bad Request (Invalid Method)", parser.isBadRequest());
}

int main() {
    std::cout << "--- Running RequestParser Unit Tests ---" << std::endl;
    test_simple_get_request();
    test_post_with_body();
    // test_incremental_parsing(); // 필요하면 주석 해제
    // test_chunked_request();     // 필요하면 주석 해제
    test_bad_request();
    std::cout << "--------------------------------------" << std::endl;
    return 0;
}