#include "HttpMessage.hpp"
#include <sstream>
#include <exception>

std::string& HttpMessage::body() { return _body; }
HttpMessage::headers_t& HttpMessage::headers() { return _headers; }
HttpMessage::headers_t::iterator HttpMessage::add_header(const std::string& name, const std::string& value)
{
    if (_headers[name].empty())
        _headers[name] = value;
    else
        _headers[name] += ", " + value;
    return _headers.find(name);
}

// Constructors
HttpMessage::HttpMessage() : _major_version(1), _minor_version(1) { }
HttpMessage::HttpMessage(const HttpMessage& copy) { *this = copy; }

HttpMessage::~HttpMessage() { }

HttpMessage& HttpMessage::operator=(const HttpMessage& copy)
{
    _major_version = copy._major_version;
    _minor_version = copy._minor_version;

    _headers = copy._headers;
    _body = copy._body;
    return *this;
}

// Getters/Setters
int HttpMessage::major_version() const { return _major_version; }
int HttpMessage::minor_version() const { return _minor_version; }
std::string HttpMessage::version_string() const
{
    std::stringstream result;
    result << "HTTP/" << _major_version << "." << _minor_version;
    return result.str();
}

const std::string& HttpMessage::header(const std::string& field_name) const
{
    return _headers.at(field_name);
}
const std::string& HttpMessage::body() const { return _body; }

// This will convert it back to a text based http message
std::string HttpMessage::toStringHeaders() const
{
    std::stringstream result;
    for (HttpMessage::headers_t::const_iterator i = _headers.begin(); i != _headers.end(); ++i)
        result << i->first << ": " << i->second << "\n";
    return result.str();
}
std::string HttpMessage::toStringBody() const { return _body;}

std::string HttpMessage::toString() const
{
    std::stringstream result;

    result << toStringStart() << toStringHeaders() << "\n" << toStringBody();

    return result.str();
}
