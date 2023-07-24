#include "Server.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <algorithm>

Server::Server( ServerCfg& cfg, ServerConfig& gen_cfg ) :_cfg( cfg ), _gen_cfg( gen_cfg ) {
}

Server::Server( const Server& copy ) :_cfg( copy._cfg ), _gen_cfg( copy._gen_cfg ) {
}

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
        std::string::size_type p = req.target().find(i->route_path);
        if (p == 0 && i->route_path.length() > route_match.first)
            route_match = std::pair<int, RouteCfg*>(i->route_path.length(), &(*i));
    }

    return route_match.second;
}

/*
*   @is_accepted_method:
*    Checks if the requested method is accepted by the route
*/
bool is_accepted_method(RouteCfg* route, const std::string method) {
    return std::find(route->accepted_methods.begin(), route->accepted_methods.end(), method) != route->accepted_methods.end();
}

/*
*   @get_path:
*       If auto_index is true, returns user request
*       If not, return predefined index
*
*/
std::string	get_path(const HttpRequest& req, RouteCfg* route)
{
    // If root ends in '/', remove last char
    if (!route->root.empty() && route->root[route->root.size() - 1] == '/')
        route->root = route->root.substr(0, route->root.size() - 1);

    // Request is equal to relative path ('.') + root path + route path
    std::string	request = route->root + route->route_path;

    if ( route->auto_index ) {
        // If target is '' or '/' , return request as it is
        if ( req.target() == "/" ) return request;
        // If route_path == '/', return request as it is + target (what file the request is requesting)
        if ( route->route_path == "/") return request + req.target().substr(1, req.target().size() - 1);

        // If none of the above, return relative root path + requested file (which already has route path)
        return route->root + req.target();
    }
    // If index was set, return request + predifined index
	return request + "/" + route->index;
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

    path = get_path(req, route);

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

    // Check if requested method is available
    if ( !(is_accepted_method( route, req.method() ) ) )  {
        res.set_status( 405, "Method Not Allowed" );
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
