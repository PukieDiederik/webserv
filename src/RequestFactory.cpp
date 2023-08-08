#include "RequestFactory.hpp"

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