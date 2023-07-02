#include "Server.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>

Server::Server(ServerCfg& cfg, ServerConfig& gen_cfg) :_cfg(cfg), _gen_cfg(gen_cfg) { }
Server::Server(const Server& copy) :_cfg(copy._cfg), _gen_cfg(copy._gen_cfg) { }

Server::~Server() { }

Server& Server::operator=(const Server& copy)
{
    _cfg = copy._cfg;
    _gen_cfg = copy._gen_cfg;
    return *this;
}

// Helper functions
RouteCfg* find_route(const HttpRequest& req, std::vector<RouteCfg>& routes)
{
    std::pair<std::size_t, RouteCfg*> route_match(0, NULL);

    for(std::vector<RouteCfg>::iterator i = routes.begin(); i != routes.end(); ++i)
    {
        std::string::size_type p = req.target().find(i->name);
        if (p == 0 && i->name.length() > route_match.first)
            route_match = std::pair<int, RouteCfg*>(i->name.length(), &(*i));
    }

    return route_match.second;
}

// Will take a request and handle it, which includes calling cgi
HttpResponse Server::handleRequest(const HttpRequest& req)
{
    HttpResponse res;
    RouteCfg* route = find_route(req, _cfg.routes);
    std::string path;

    // Check if a valid route has been found
    if (!route) {
        // TODO: return 404 error page
        res.set_status(404, "File Not Found");
        return res;
    }

    // Get and check path
    path = route->root + "/" + req.target().substr(route->name.length());
    if (::access(path.c_str(), F_OK) < 0)
    {
        // TODO: return 404 error page
        res.set_status(404, "File Not Found");
        return res;
    }
    if (::access(path.c_str(), O_RDONLY) < 0)
    {
        // TODO: return 403 error page
        res.set_status(403, "Forbidden");
        return res;
    }

    // TODO: check for CGI

    if (req.method() == "GET")
    {

        std::ifstream file(path.c_str());
        std::string buff(BUFFER_SIZE, '\0');
        if (!file.is_open())
        {
            // TODO: return 500 error page
            res.set_status(500, "Internal Server Error");
            return res;
        }

        while(file.read(&buff[0], BUFFER_SIZE).gcount() > 0)
            res.body().append(buff, 0, file.gcount());

        std::ostringstream ss;
        ss << res.body().length();
        res.set_status(200, "OK");
        //TODO add MIME type
        res.set_header("Content-Type", "text/plain");
    }
    return res;
}

ServerCfg& Server::cfg() { return _cfg; }
