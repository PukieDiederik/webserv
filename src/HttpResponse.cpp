#include "HttpResponse.hpp"
#include <sstream>

std::string HttpResponse::toStringStart() const
{
    std::stringstream result;

    result << HttpMessage::version_string() << " " << _status_code
           << (_status_message.empty() ? "" : " "0) << _status_message << "\r\n";
}

HttpResponse::HttpResponse() :_status_code(0), _status_message(0) { }
HttpResponse::HttpResponse(const HttpResponse& copy) { *this = copy; }
HttpResponse::~HttpResponse();

HttpResponse& HttpResponse::operator= (const HttpResponse& copy)
{
    HttpMessage::operator=(copy);
    _status_code = copy._status_code;
    _status_message = copy._status_message;
    return *this;
}

HttpResponse& HttpResponse::set_version(int major, int minor)
{
    this->_major_version = major;
    this->_minor_version = minor;
    return *this;
}
HttpResponse& HttpResponse::set_body(const std::string& body) { this->_body = body; return *this; }

HttpResponse& HttpResponse::add_header(const std::string& name, const std::string& value)
{ this->add_header(name, value); return *this; }
const std::string* HttpResponse::get_header(const std::string& name) const { return this->header(name); }
HttpResponse& HttpResponse::remove_header(const std::string& name)
{ this->_headers.erase(name); return *this;}

HttpResponse& HttpResponse::set_status(int code ) { _status_code = code; return *this; }
HttpResponse& HttpResponse::set_status(int code, const std::string& message)
{_status_code = code; _status_message = message; return *this;}