#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include "HttpMessage.hpp"
#include <string>

class HttpResponse : public HttpMessage
{
private:
    int _status_code;
    std::string _status_message;

protected:
    virtual std::string toStringStart() const;
public:
    HttpResponse();
    HttpResponse(const HttpResponse& copy);
    ~HttpResponse();

    HttpResponse& operator= (const HttpResponse& copy);

    int status() const;
    const std::string& statusMessage() const;

    HttpResponse& status(int code);
    HttpResponse& status(int code, const std::string& message);

    virtual std::string toString();
};
#endif
