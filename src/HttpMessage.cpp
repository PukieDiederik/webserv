#include "HttpMessage.hpp"
#include <sstream>
#include <exception>
#include "SessionManager.hpp"

# include <iostream>

void    debugss( const std::string& msg );

HttpMessage::headers_t::iterator HttpMessage::add_header(const std::string& name, const std::string& value)
{
    if (_headers[name].empty())
        _headers[name] = value;
    else
        _headers[name] += ", " + value;
    return _headers.find(name);
}

HttpMessage&    HttpMessage::remove_header(const std::string& name)
{
    _headers.erase(name);

    return *this;
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

const HttpMessage::headers_t& HttpMessage::headers() const { return _headers; }

const std::string& HttpMessage::headers(const std::string& field_name) const
{
    return _headers.at(field_name);
}

HttpMessage::headers_t::const_iterator HttpMessage::headers(const std::string& field, const std::string& value)
{
    if (_headers[field].empty())
        _headers[field] = value;
    else
        _headers[field] += ", " + value;
    return _headers.find(field);
}

void HttpMessage::removeHeader(const std::string& field) { _headers.erase(field); }

const std::string& HttpMessage::body() const { return _body; }
std::string& HttpMessage::body() { return _body; }

// This will convert it back to a text based http message
std::string HttpMessage::toStringHeaders() const
{
    std::stringstream result;
    for (HttpMessage::headers_t::const_iterator i = _headers.begin(); i != _headers.end(); ++i)
        result << i->first << ": " << i->second << "\r\n";

    // gde-alme    
    for ( HttpMessage::cookies_t::const_iterator i = _cookies.begin(); i != _cookies.end(); i++ ) {
        debugss( "Adding cookie: " + i->second );
        if ( i->second.find( '=' ) != std::string::npos)
            result << "Set-Cookie" << ": " << i->second  << "\r\n";
    }

    return result.str();
}
std::string HttpMessage::toStringBody() const { return _body;}

std::string HttpMessage::toString()
{
    std::stringstream result;

    result << toStringStart() << toStringHeaders() << "\r\n" << toStringBody();

    return result.str();
}

// gde-alme
void    HttpMessage::setCookies( const HttpMessage::cookies_t& cookies ) {
    _cookies = cookies;
}

HttpMessage::cookies_t  HttpMessage::getCookies() const {
    return _cookies;
}

