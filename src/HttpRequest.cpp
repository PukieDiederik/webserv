#include "HttpRequest.hpp"
#include "ParsingUtils.hpp"
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

HttpRequest::HttpRequest(const std::string& in) : HttpMessage()
{
    std::istringstream is(in);
    std::string line;
    std::string rest = in;

    { // Parse header
        std::getline(is, line);
        rest = rest.substr(line.length() + 1);
        line = trimSpace(line);

        std::size_t method_end = line.find_first_of(' ');
        if (method_end == std::string::npos)
            throw ParsingException("Not enough fields in header line");
        _method = line.substr(0, method_end);

        std::size_t target_end = line.find_first_of(' ', method_end + 1);
        if (target_end == std::string::npos)
            throw ParsingException("Not enough fields in header line");
        _target = line.substr(method_end + 1, target_end - method_end - 1);

        // Checks if there are more fields than expected
        if (line.find_first_of(' ', target_end + 1) != std::string::npos)
            throw ParsingException("Too many fields in header line");

        std::string tmp = line.substr(target_end + 1);
        if (tmp.find("HTTP/") != 0)
            throw ParsingException("Invalid http version specifier");
        std::stringstream version_ss(tmp.substr(5));
        if (!std::isdigit(version_ss.peek()))
            throw ParsingException("Invalid http version specifier");
        version_ss >> this->_major_version;
        if (version_ss.get() != '.' || !std::isdigit(version_ss.peek()))
            throw ParsingException("Invalid http version specifier");
        version_ss >> this->_minor_version;
    }

    { // Parse header fields
        std::getline(is, line);
        rest = rest.substr(line.length());
        line = trimSpace(line);
        while (line != "")
        {
            std::size_t separator = line.find_first_of(": ");
            if (separator == std::string::npos)
                throw ParsingException("Header field does not have seperator");
            std::string field_name = line.substr(0, separator);
            if (field_name.empty() ||
                field_name.find_first_not_of(std::string("!#$%&'*+-.^_`|~") + ALPHA + DIGIT) != std::string::npos)
                throw ParsingException("Invalid characters in field name");

            std::string field_value = trimSpace(line.substr(separator + 1));
            add_header(field_name, field_value);

            std::getline(is, line);
            rest = rest.substr(line.length() + 1);
            line = trimSpace(line);
        }

        if (!_headers.count("Host"))
            throw ParsingException("'Host' header was not provided");
    }

    // Remove newline which separates headers and body
    rest = rest.substr(1);

    _body = rest;
}

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

