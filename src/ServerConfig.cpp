#include "ServerConfig.hpp"
#include <string>


RouteCfg::RouteCfg() { }
RouteCfg::RouteCfg(const RouteCfg& copy)
{
    name = copy.name;
    is_redirect = copy.is_redirect;
    redirect_to = copy.redirect_to;
    root = copy.root;
    cgi_enabled = copy.cgi_enabled;
    auto_index = copy.auto_index;
    index = copy.index;
    accepted_methods = copy.accepted_methods;
}
RouteCfg::~RouteCfg() { }
RouteCfg RouteCfg::operator=(const RouteCfg& copy)
{
    name = copy.name;
    is_redirect = copy.is_redirect;
    redirect_to = copy.redirect_to;
    root = copy.root;
    cgi_enabled = copy.cgi_enabled;
    auto_index = copy.auto_index;
    index = copy.index;
    accepted_methods = copy.accepted_methods;

    return *this;
}

ServerCfg::ServerCfg() { }
ServerCfg::ServerCfg(const ServerCfg& copy)
{
    port = copy.port;
    server_names = copy.server_names;

    error_pages = copy.error_pages;
    max_body_size = copy.max_body_size;

    root_dir = copy.root_dir;
    routes = copy.routes;
}
ServerCfg::~ServerCfg() { }
ServerCfg ServerCfg::operator=(const ServerCfg& copy)
{
    port = copy.port;
    server_names = copy.server_names;

    error_pages = copy.error_pages;
    max_body_size = copy.max_body_size;

    root_dir = copy.root_dir;
    routes = copy.routes;
    return *this;
}


// ServerConfig
ServerConfig::ServerConfig(const std::string& file) { (void)file; }
ServerConfig::ServerConfig(const ServerConfig& copy)
{
    mime = copy.mime;
    servers = copy.servers;
}

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy)
{
    mime = copy.mime;
    servers = copy.servers;
    return *this;
}
