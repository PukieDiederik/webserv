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

HttpResponse& HttpResponse::set_version(int major, int minor)
{
    this->_major_version = major;
    this->_minor_version = minor;
    return *this;
}

HttpResponse& HttpResponse::set_header(const std::string& name, const std::string& value)
{ this->add_header(name, value); return *this; }
const std::string& HttpResponse::get_header(const std::string& name) const { return this->header(name); }
HttpResponse& HttpResponse::remove_header(const std::string& name)
{ this->headers().erase(name); return *this;}

HttpResponse& HttpResponse::set_status(int code ) { _status_code = code; return *this; }
HttpResponse& HttpResponse::set_status(int code, const std::string& message)
{_status_code = code; _status_message = message; return *this; }

std::string& HttpResponse::body() { return this->_body; }
const std::string& HttpResponse::body() const { return this->_body; }

int HttpResponse::get_status() const { return _status_code; }
const std::string& HttpResponse::get_status_message() const { return _status_message; }

std::string HttpResponse::toString()
{
    std::stringstream cl;
    cl << _body.length();
    _headers["Content-Length"] = cl.str();

    std::stringstream result;
    result << toStringStart() << toStringHeaders() << "\r\n" << toStringBody();

    return result.str();
}

std::map<short, std::string>    createStatusCodeMap() {
    std::map<short, std::string> map;

    map.insert(std::make_pair(200, "OK"));
    map.insert(std::make_pair(403, "Forbidden"));
    map.insert(std::make_pair(404, "File Not Found"));
    map.insert(std::make_pair(405, "Method Not Allowed"));
    map.insert(std::make_pair(500, "Internal Server Error"));

    return map;
}

const std::map<short, std::string>  HttpResponse::_status_code_map = createStatusCodeMap();

std::string HttpResponse::get_status_code_description(const int statusCode)
{
    std::map<short, std::string>::const_iterator    it = _status_code_map.find(statusCode);

    return (it != HttpResponse::_status_code_map.end()) ? it->second : "Undefined";
}
