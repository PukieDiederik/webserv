#include "Router.hpp"
#include "ServerConfig.hpp"

Router::Router(ServerConfig& cfg) :_cfg(cfg) { }
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