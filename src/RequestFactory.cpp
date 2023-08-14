#include "RequestFactory.hpp"
#include "ParsingUtils.hpp"
#include "ParsingException.hpp"
#include <algorithm>
#include <sstream>

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
        _version.find_first_not_of(DIGIT) != period_pos || // Check for non-digits in version part
        _version.find_first_not_of(DIGIT, period_pos) != std::string::npos) // ^^
        return false;

    return true;
}

void RequestFactory::parse()
{
    // Parsing request line
    if (m_active_status == RequestFactory::REQ_LINE)
    {
        // Remove any leading whitespace

        // Check if a line is ready, if not we exit this function
        if (m_buffer.find('\n') == std::string::npos)
            return;
        std::string line = m_buffer.substr(0, m_buffer.find_first_not_of('\n') + 1);
        line = trimSpace(line);
        // Check if a line is valid, throw exception otherwise
        if (!isValidReqLine(line))
            throw ParsingException("Invalid request line");

        // Reset active request, it might still have data from a potential previous request.
        m_active_req = HttpRequest();

        // Parse request line
        m_active_req.method() = line.substr(0, line.find(' '));
        m_active_req.target() = line.substr(line.find(' ') + 1, line.rfind(' '));

        std::stringstream ss;
        int maj_v;
        int min_v;

        ss << line.substr(line.rfind('/') + 1, line.rfind('.'));
        ss >> maj_v;
        ss << line.substr(line.rfind('.'));
        ss >> min_v;
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

    return *this;
}

void RequestFactory::in(const std::string& str) { (void)str; }
bool RequestFactory::isReqReady() const { return !m_req_buffer.empty(); }
HttpRequest RequestFactory::getRequest()
{
    HttpRequest req = m_req_buffer.front();
    m_req_buffer.pop();
    return req;
}