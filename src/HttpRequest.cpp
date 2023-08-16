#include "HttpRequest.hpp"
#include "ParserUtils.hpp"
#include "ParsingException.hpp"
#include <sstream>

#include <iostream>

std::string HttpRequest::toStringStart() const
{
    std::stringstream result;
    result << _method << " " << _target << " " << HttpMessage::version_string() << "\r\n";
    return result.str();
}

HttpRequest::HttpRequest() { }

HttpRequest::HttpRequest(const HttpRequest& copy) :HttpMessage() { *this = copy; }

HttpRequest::~HttpRequest() { }

HttpRequest& HttpRequest::operator=(const HttpRequest& copy)
{
    HttpMessage::operator=(copy);
    _target = copy._target;
    _method = copy._method;
    return *this;
}

std::string& HttpRequest::target() { return _target; }
const std::string& HttpRequest::target() const { return _target; }

const std::string& HttpRequest::method() const { return _method; }
std::string& HttpRequest::method() { return _method; }

const std::string& HttpRequest::host() const { return headers("Host"); }

