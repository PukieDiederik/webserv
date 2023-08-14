#include "HttpResponse.hpp"
#include <sstream>

std::string HttpResponse::toStringStart() const
{
    std::stringstream result;

    result << HttpMessage::version_string() << " " << _status_code
           << (_status_message.empty() ? "" : " ") << _status_message << "\r\n";
    return result.str();
}

HttpResponse::HttpResponse() :HttpMessage(), _status_code(0), _status_message("") { }
HttpResponse::HttpResponse(const HttpResponse& copy) :HttpMessage() { *this = copy; }
HttpResponse::~HttpResponse() { }

HttpResponse& HttpResponse::operator= (const HttpResponse& copy)
{
    HttpMessage::operator=(copy);
    _status_code = copy._status_code;
    _status_message = copy._status_message;
    return *this;
}

HttpResponse& HttpResponse::status(int code ) { _status_code = code; return *this; }
HttpResponse& HttpResponse::status(int code, const std::string& message)
{
    _status_code = code;
    _status_message = message;
    return *this;
}

int HttpResponse::status() const { return _status_code; }
const std::string& HttpResponse::statusMessage() const { return _status_message; }

std::string HttpResponse::toString()
{
    std::stringstream cl;
    cl << _body.length();
    _headers["Content-Length"] = cl.str();

    std::stringstream result;
    result << toStringStart() << toStringHeaders() << "\r\n" << toStringBody();

    return result.str();
}
