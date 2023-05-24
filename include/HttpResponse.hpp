#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpMessage.hpp"
#include <string>

class HttpResponse : public HttpMessage
{
private:
    int _status_code;
    int _status_message;

protected:
    virtual std::string toStringStart() const;
public:
    HttpResponse();
    HttpResponse(const HttpResponse& copy);
    ~HttpResponse();

    HttpResponse& operator= (const HttpResponse& copy);

    HttpResponse& set_version(int major, int minor);
    HttpResponse& set_body(const std::string& body);

    HttpResponse& add_header(const std::string&, const std::string& value);
    const std::string& get_header(const std::string& name) const;
    HttpResponse& remove_header(const std::string& name);

    HttpResponse& set_status(int code);
    HttpResponse& set_status(int code, const std::string& message);

}

#endif