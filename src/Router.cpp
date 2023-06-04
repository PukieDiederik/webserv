#include "Router.hpp"
#include "ServerConfig.hpp"

Router::Router(ServerConfig& cfg) :_cfg(cfg)
{
    // TODO: This should be removed, this is just for testing purposes REJECT PR IF THIS IS PRESENT
    ServerCfg s1;
    ServerCfg s2;
    s1.port = 3001;
    s1.port = 3002;
    s1.server_names.push_back("www.test.com");
    s1.server_names.push_back("sub.website.nl");
    s2.server_names.push_back("www.test.com");
    s2.server_names.push_back("sub.website.nl");
    s1.root_dir = "/data/programming/webserv";
    s2.root_dir = "/data/programming/webserv";

    RouteCfg r1;
    RouteCfg r2;

    r1.name = "/";
    r1.root = s1.root_dir + "/include";
    r2.name = "/";
    r2.root = s1.root_dir + "/src";

    r1.auto_index = true;
    r2.auto_index = true;

    s1.routes.push_back(r1);
    s2.routes.push_back(r2);
}

Router::Router(const Router& copy) :_cfg(copy._cfg) { }

Router::~Router() { }

Router& Router::operator=(const Router& copy)
{
    _cfg = copy._cfg;
    return *this;
}

void Router::listen()
{
    // TODO: start listening to ports
}