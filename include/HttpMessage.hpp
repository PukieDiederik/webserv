#ifndef HTTP_MESSAGE_HPP
#define HTTP_MESSAGE_HPP

#include <map>
#include <string>

class HttpMessage {
public:
    typedef std::map<std::string, std::string> headers_t;
private:
    int _major_version;
    int _minor_version;

    headers_t _headers;

    std::string _body;
protected:
    std::string& body();
    headers_t& headers();
    headers_t::iterator add_header(const std::string& name, const std::string& value);

    //helper functions for toString method
    virtual std::string toStringStart() const = 0;
    virtual std::string toStringHeaders() const;
    virtual std::string toStringBody() const;
public:

    // Constructors
    HttpMessage();
    HttpMessage(const HttpMessage& copy);

    virtual ~HttpMessage();

    HttpMessage& operator=(const HttpMessage& copy);

    // Getters/Setters
    int major_version() const;
    int minor_version() const;
    std::string version_string() const; // Returns version string (for example: "HTTP/1.1")

    const std::string* header(const std::string& field_name) const;
    const std::string& body() const;

    // This will convert it back to a text based http message
    virtual std::string toString() const;
};

#endif
