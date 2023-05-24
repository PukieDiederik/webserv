#include "HttpRequest.hpp"
#include <sstream>

std::string HttpRequest::toStringStart() const
{
    std::stringstream result;
    result << _method << " " << _target << " " << HttpMessage::version_string() << "\n";
    return result.str();
}

HttpRequest::HttpRequest(std::string in)
{
    // TODO: parse a request
    (void)in;
}

HttpRequest::HttpRequest(const HttpRequest& copy) { *this = copy; }

HttpRequest::~HttpRequest() { }

HttpRequest& HttpRequest::operator=(const HttpRequest& copy)
{
    HttpMessage::operator=(copy);
    _target = copy._target;
    return *this;
}

std::string& HttpRequest::target() { return _target; }
const std::string& HttpRequest::target() const { return _target; }
