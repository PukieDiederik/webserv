#include "Server.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

Server::Server(ServerCfg& cfg, ServerConfig& gen_cfg) :_cfg(cfg), _gen_cfg(gen_cfg) { }
Server::Server(const Server& copy) :_cfg(copy._cfg), _gen_cfg(copy._gen_cfg) { }

Server::~Server() { }

Server& Server::operator=(const Server& copy)
{
    _cfg = copy._cfg;
    _gen_cfg = copy._gen_cfg;
    return *this;
}

// Will take a request and handle it, which includes calling cgi
HttpResponse Server::handleRequest(const HttpRequest& req)
{
    // TODO: handle request
    (void)req;
    return HttpResponse();
}
