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

// Will take a request and handle it, which includes calling cgi
HttpResponse Server::handleRequest(const HttpRequest& req)
{
    HttpResponse res;
    // TODO: handle request
    if (req.method() == "GET")
    {
        std::pair<int, RouteCfg*> match(0, NULL);
        // Get correct route
        for(std::vector<RouteCfg>::iterator i = _cfg.routes.begin(); i != _cfg.routes.end(); ++i)
        {
            std::string::size_type p = req.target().find(i->name);
            if (p == 0)
                match = std::pair<int, RouteCfg*>(i->name.length(), &(*i));

        }
        if (match.first == 0) // If no match is found
        {
            // TODO: return 404 error page
            res.set_status(404, "File Not Found");
            return res;
        }
        std::string path = match.second->root + "/" + req.target().substr(match.second->name.length());

        if (::access(path.c_str(), F_OK) < 0)
        {
            // TODO: return 404 error page
            res.set_header("Content-Length", "0");
            res.set_status(404, "File Not Found");
            return res;
        }
        if (::access(path.c_str(), O_RDONLY) < 0)
        {
            // TODO: return 403 error page
            res.set_status(403, "Forbidden");
            return res;
        }

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
        res.set_header("Content-Length", ss.str());
        res.set_header("Content-Type", "text/plain");
        //TODO add MIME type
    }
    return res;
}

ServerCfg& Server::cfg() { return _cfg; }
