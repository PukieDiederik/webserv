#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "HttpMessage.hpp"
#include <string>

class HttpRequest : public HttpMessage
{
protected:
    virtual std::string toStringStart() const;

    std::string _target;
    std::string _method;

public:
    HttpRequest();
    HttpRequest(const std::string& in); // parses the entire request
    HttpRequest(const HttpRequest& copy);
    ~HttpRequest();

    HttpRequest& operator=(const HttpRequest& copy);

    std::string& target();
    const std::string& target() const;

    const std::string& method() const;
    const std::string& host() const;
};

#endif
