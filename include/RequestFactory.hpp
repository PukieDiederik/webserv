#ifndef REQUEST_FACTORY_HPP
#define REQUEST_FACTORY_HPP

#include "HttpRequest.hpp"
#include <queue>
#include <string>

class RequestFactory {
public:
    
private:
    enum parse_status { REQ_LINE, HEADER, BODY };

    HttpRequest m_active_req;
    parse_status m_active_status;
    std::string m_buffer;
    std::queue<HttpRequest> m_req_buffer;

    void parse();
public:

    RequestFactory();
    RequestFactory(const RequestFactory& copy);

    ~RequestFactory();

    RequestFactory& operator=(const RequestFactory& copy);

    void in(const std::string& str); // Updates buffer and parses it
    bool isReqReady() const; // Checks if a request is ready
    HttpRequest getRequest(); // Returns a request from the request buffer
};

#endif
