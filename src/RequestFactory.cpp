#include "RequestFactory.hpp"
#include "ParsingUtils.hpp"
#include "ParsingException.hpp"
#include <algorithm>
#include <sstream>
#include <iostream>

// Makes sure that the request line is correctly formatted
bool isValidReqLine(const std::string& line)
{
    if (std::count(line.begin(), line.end(), ' ') != 2)
        return false;

    // Check method for valid characters
    std::string _method = line.substr(0, line.find(' '));
    if (_method.find_first_not_of(TOKEN) != std::string::npos)
        return false;

    // target checking not applicable

    // Validate version part
    std::string _version = line.substr(line.rfind(' ') + 1, line.size());
    std::string::size_type period_pos = _version.find('.');
    if (period_pos == std::string::npos ||
        _version.compare(0, 5, "HTTP/") || // Check for it starting with 'HTTP/'
        _version.find_first_not_of(DIGIT, 5) != period_pos || // Check for non-digits in version part
        _version.find_first_not_of(DIGIT, period_pos + 1) != std::string::npos) // ^^
        return false;

    return true;
}

bool isValidHeaderLine(const std::string& line)
{
    if (line.find(static_cast<std::string>(": ")) != std::string::npos)
        return false;

    // Check if the field name is ok and both field and value are not empty
    std::string _field = line.substr(0, line.find(static_cast<std::string>(": ")));
    std::string _value = line.substr(line.find(static_cast<std::string>(": ")) + 2);
    if (_field.empty() || _value.empty() || _field.find_first_not_of(TOKEN) != std::string::npos)
        return false;

    return true;
}

void RequestFactory::parse()
{
    // Parsing request line
    if (m_active_status == RequestFactory::REQ_LINE)
    {
        std::string line;
        // Remove any leading whitespace
        while (m_buffer.find('\n') != std::string::npos && line.empty())
        {
            line = m_buffer.substr(0, m_buffer.find('\n') + 1);
            line = trimSpace(line);

            m_buffer.erase(0, m_buffer.find('\n') + 1);
        }

        if (line.empty())
            return;

        // Check if a line is valid, throw exception otherwise
        if (!isValidReqLine(line))
            throw ParsingException("Invalid request line");

        // Reset active request, it might still have data from a potential previous request.
        m_active_req = HttpRequest();

        // Parse request line
        m_active_req.method() = line.substr(0, line.find(' '));
        m_active_req.target() = line.substr(line.find(' ') + 1, line.rfind(' ') - m_active_req.method().length() - 1);

        std::stringstream ss;
        int maj_v;
        int min_v;

        ss << line.substr(line.rfind('/') + 1, line.rfind('.'));
        ss >> maj_v;
        ss << line.substr(line.rfind('.'));
        ss >> min_v;

        m_active_status = RequestFactory::HEADER;
    }

    if (m_active_status == RequestFactory::HEADER)
    {
        std::cout << "headers" << std::endl;
        std::string line;
        while (m_buffer.find('\n') != std::string::npos)
        {
            line = m_buffer.substr(0, m_buffer.find('\n') + 1);
            line = trimSpace(line);

            m_buffer.erase(0, m_buffer.find('\n') + 1);
            if (line.empty())
                break;
            
            // Add header
            std::string _field = line.substr(0, line.find(static_cast<std::string>(": ")));
            std::string _value = line.substr(line.find(static_cast<std::string>(": ")) + 2);
            m_active_req.headers(_field, _value);
        }

        // If we've reached the end of the headers
        if (line.empty())
        {
            // Check for body headers
            if (m_active_req.headers().count("Content-Length"))
            {
                m_body_type = RequestFactory::LENGTH;
                m_active_status = RequestFactory::BODY;
                if (m_active_req.headers().find("Content-Length")->second.find_first_not_of(DIGIT) != std::string::npos)
                    throw ParsingException("Content-Length not in right format");
            }
            // TODO:
//            else if (m_active_req.headers().count("Transfer-Encoding"))
//            {
//                m_body_type = RequestFactory::CHUNKED;
//                m_active_status = RequestFactory::BODY;
//            }
            else // If no body is provided
            {
                m_req_buffer.push(m_active_req);
                m_active_status = RequestFactory::REQ_LINE;
            }
        }
        return;
    }
    if (m_active_status == RequestFactory::BODY)
    {
        std::cout << "body" << std::endl;
        if (m_body_type == RequestFactory::LENGTH)
        {
            std::stringstream ss;
            ss << m_active_req.headers("Content-Length");

            unsigned int s;
            ss >> s;
            std::cout << m_buffer.length() << ", " << s << std::endl;

            if (m_buffer.length() != s) // Exit if not enough is in the buffer yet
                return;

            m_active_req.body() = m_buffer.substr(0, s);
            m_req_buffer.push(m_active_req);
            m_active_status = RequestFactory::REQ_LINE;
        }
    }
}

RequestFactory::RequestFactory() : m_active_status(RequestFactory::REQ_LINE) { }
RequestFactory::RequestFactory(const RequestFactory& copy) { *this = copy; }

RequestFactory::~RequestFactory() { }

RequestFactory& RequestFactory::operator=(const RequestFactory& copy)
{
    m_active_req = copy.m_active_req;
    m_active_status = copy.m_active_status;
    m_buffer = copy.m_buffer;
    m_req_buffer = copy.m_req_buffer;
    m_body_type = copy.m_body_type;

    return *this;
}

void RequestFactory::in(const std::string& str)
{
    m_buffer += str;
    parse();
}
bool RequestFactory::isReqReady() const { return !m_req_buffer.empty(); }
HttpRequest RequestFactory::getRequest()
{
    HttpRequest req = m_req_buffer.front();
    m_req_buffer.pop();
    return req;
}