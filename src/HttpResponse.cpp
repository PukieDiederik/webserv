#include "HttpResponse.hpp"
#include <sstream>


// BEGIN: Helper Functions
std::map<short, std::string>    createStatusCodeMap() {
    std::map<short, std::string>    map;

    map.insert(std::make_pair(200, "OK"));
    map.insert(std::make_pair(400, "Bad request"));
    map.insert(std::make_pair(403, "Forbidden"));
    map.insert(std::make_pair(404, "File Not Found"));
    map.insert(std::make_pair(405, "Method Not Allowed"));
    map.insert(std::make_pair(500, "Internal Server Error"));

    return map;
}
// END: Helper functions

// BEGIN: Variables
const std::map<short, std::string>  HttpResponse::_status_code_map = createStatusCodeMap();
// END: Variables


// BEGIN: Canonical Form Functions
HttpResponse::HttpResponse() :HttpMessage(), _status_code(0), _status_message("") {}

HttpResponse::HttpResponse(const HttpResponse& copy) :HttpMessage()
{
    *this = copy;
}

HttpResponse::~HttpResponse() {}

HttpResponse&       HttpResponse::operator=(const HttpResponse& copy)
{
    HttpMessage::operator=(copy);
    _status_code = copy._status_code;
    _status_message = copy._status_message;

    return *this;
}
// END: Canonical Form Functions

HttpResponse& HttpResponse::status(int code ) { _status_code = code; return *this; }
HttpResponse& HttpResponse::status(int code, const std::string& message)
{
    _status_code = code;
    _status_message = message;
    return *this;
}

int HttpResponse::status() const { return _status_code; }
const std::string& HttpResponse::statusMessage() const { return _status_message; }

// BEGIN: Class Functions
std::string         HttpResponse::toStringStart() const
{
    std::stringstream   result;

    result
        << HttpMessage::version_string()
        << " "
        << _status_code
        << (_status_message.empty() ? "" : " ")
        << _status_message
        << "\r\n";

    return result.str();
}

std::string         HttpResponse::toString()
{
    std::stringstream   cl;
    cl << _body.length();
    _headers["Content-Length"] = cl.str();

    std::stringstream   result;
    result << toStringStart() << toStringHeaders() << "\r\n" << toStringBody();

    return result.str();
}
// END: Class Functions