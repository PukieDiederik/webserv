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
    HttpRequest(const HttpRequest& copy);
    ~HttpRequest();

    HttpRequest& operator=(const HttpRequest& copy);

    // Getters & Setters
    const std::string& target() const;
    std::string& target();
    	
    const std::string& method() const;
    std::string& method();

    const std::string& host() const;
};

#endif
