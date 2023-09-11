#include "Server.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "ServerUtils.hpp"
#include "SessionManager.hpp"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstdio>

#define VALIDPATH 0
#define AUTOINDEX 1
#define INVALIDPATH 2


// BEGIN: Helper Functions Prototypes
RouteCfg*       find_route(const HttpRequest& req, std::vector<RouteCfg>& routes);
HttpResponse    list_dir_res(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route);
HttpResponse    response_get(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route);
HttpResponse    response_head(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route);
HttpResponse    response_delete(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route);
HttpResponse    response_error(const HttpRequest& req, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route, const int statusCode);
std::string     handleCookies( const HttpRequest& req, HttpResponse& res );
// END: Helper Functions Prototypes


// BEGIN: Canonical Form Functions
Server::Server(const ServerCfg& cfg) :_cfg(cfg) {}

Server::Server(const Server& copy) :_cfg(copy._cfg) {}

Server::~Server() {}

Server& Server::operator=(const Server& copy)
{
    _cfg = copy._cfg;
    return *this;
}
// END: Canonical Form Functions

// BEGIN: Class Functions
// Will take a request and handle it, which includes calling CGI
HttpResponse    Server::handleRequest( const HttpRequest& req )
{
    HttpResponse    res;
    RouteCfg*       route = find_route(req, _cfg.routes);
    std::string     path;
    std::string     session_id;

    // Check if a valid route has been found
    if (!route) return response_error( req, res, _cfg, route, 404);

    if ( route->is_redirect ) {
        res.set_status( 301 );
        res.set_header("Location" , route->redirect_to );
        return res;
    }

    path = get_path(req, route);
    if ( SM_ON )
        session_id = handleCookies( req, res );

    // Check if file exists
    if (!is_directory(path) && ::access(path.c_str(), F_OK) < 0)
        return response_error(req, res, _cfg, route, 404);

    // Check if we have access to file
    else if (::access(path.c_str(), O_RDONLY) < 0)
        return response_error(req, res, _cfg, route, 403);

    // Check if requested method is available
    else if (!route->accepted_methods.empty() && !is_accepted_method(route, req.method()))
        return response_error(req, res, _cfg, route, 405);

    // Handle GET method
    else if (req.method() == "GET") {
        return response_get(req, path, res, _cfg, route);
    }

    // Handle HEAD method
    else if (req.method() == "HEAD") {
        return response_head(req, path, res, _cfg, route);
    }

    // Handle DELETE method
    else if (req.method() == "DELETE")
        return response_delete(req, path, res, _cfg, route);

    else {
        std::cout << "Returning response error" << std::endl;
        return response_error(req, res, _cfg, route, 501);
    }

    // TODO: check for CGI
}

ServerCfg&  Server::cfg()
{
    return _cfg;
}
// END: Class Functions


// BEGIN: Helper Functions
RouteCfg*   find_route(const HttpRequest& req, std::vector<RouteCfg>& routes)
{
    std::pair<std::size_t, RouteCfg*>   route_match(0, NULL);

    for(std::vector<RouteCfg>::iterator i = routes.begin(); i != routes.end(); ++i) {
	    std::string	available_route = i->route_path;
	    if ( available_route[available_route.size() - 1] == '/' ) available_route.substr( 0, available_route.size() - 1 );
	    
	    std::string::size_type  p = req.target().find( available_route );
	    std::string::size_type  q = req.target().find( available_route  + "/" );

	    if (p == 0) {
	    	if ( req.target() != available_route && q != 0 && available_route != "/")
			available_route = "";
		if ( available_route.length() > route_match.first )
            		route_match = std::pair<int, RouteCfg*>(available_route.length(), &(*i));
	    }
    }

    return route_match.second;
}

HttpResponse    list_dir_res(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route)
{
    res.body().clear();
    
    std::ifstream   file(DIRLISTING);
    std::string     line_buff;

    if (!file.is_open())
        return response_error(req, res, _cfg, route, 500);
    
    std::string                 items;
    std::vector<std::string>    dir_listing = list_dir(path);

    for (size_t i = 0; i < dir_listing.size(); i++)
        items.append("<li><a href=\"" + removeSlashDups(req.target() + "/") + dir_listing[i] + "\">" + dir_listing[i] + "</a></li>");

    // Replace all occurs of [DIR] & [ITEMS] with dir name and dir listing
    while(std::getline(file, line_buff))
    {
        replace_occurrence(line_buff, "[DIR]", req.target());
        replace_occurrence(line_buff, "[ITEMS]", items);
        res.body().append(line_buff);
    }

    res.set_status(200);
    res.set_header("Content-Type", "text/html");
    file.close();

    return res;
}

HttpResponse    response_get(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route) {
    switch (index_path( route, path)) {
        case AUTOINDEX:
            return list_dir_res(req, path, res, _cfg, route);

        case INVALIDPATH:
            return response_error(req, res, _cfg, route, 404);
    }

    std::ifstream   file(path.c_str());

    if (!file.is_open())
        return response_error(req, res, _cfg, route, 500);

    std::ostringstream ss;

    ss << file.rdbuf();
    file.close();

    res.body() = ss.str();

    res.set_status(200);
    res.set_header("Content-Type", ServerConfig::getMimeType(path));

    return res;
}

HttpResponse    response_head(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route) {
    switch (index_path( route, path)) {
        case VALIDPATH:
            res.set_status(200);
            res.set_header("Content-type", ServerConfig::getMimeType(path));
            return res;

        case AUTOINDEX:
            res.set_status(200);
            res.set_header("Content-type", ServerConfig::getMimeType(DIRLISTING));
            return res;

        case INVALIDPATH:
            return response_error(req, res, _cfg, route, 404);
    }

    return res;
}

HttpResponse    response_delete(const HttpRequest& req, std::string path, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route) {
	switch (index_path( route, path)) {
		case VALIDPATH: {
			if ( remove( path.c_str() ) != 0 ) {
				std::cout << "Error deleting file" << std::endl;
				res.body().append( "\nError deleting file\n\n" );
			}
			else res.body().append( "\nFile deleted\n\n" );

			std::ostringstream  ss;
			ss << res.body().length();

			res.set_status( 200 );
			res.set_header( "Content-type", "text/html" );
			break ;
		}
        	default:
			return response_error(req, res, _cfg, route, 404);
	}

    return res;
}

HttpResponse    response_error(const HttpRequest& req, HttpResponse& res, ServerCfg& _cfg, RouteCfg* route, const int statusCode) {
    std::map<short, std::string>::const_iterator    it = _cfg.error_pages.find(statusCode);
    
    if (it != _cfg.error_pages.end()) {
    	std::string     path = get_path(it->second, _cfg);
    	std::ifstream   file(path.c_str());
    	std::string     buff(BUFFER_SIZE, '\0');

		if (is_file(path) && file.is_open()) {
			while(file.read(&buff[0], BUFFER_SIZE).gcount() > 0)
				res.body().append(buff, 0, file.gcount());
			
			std::ostringstream  ss;
            		ss << res.body().length();

            		res.set_status(200);
            		res.set_header("Content-Type", ServerConfig::getMimeType(path));
        	} else if (statusCode == 500)
			res.set_status(500);
		else
			return response_error(req, res, _cfg, route, 500);
	}
    else {
		res.body().append( "\
				<html><head><title>Error</title></head>\
				<body style=\"\
				display: flex;\
				justify-content: center;\
				align-items: center;\
				height: 100vh; margin: 0;\"\
				><h1>Error</h1></body></html>\
				" );
        res.set_status( statusCode );
        res.set_header("Content-Type", "text/html");
		return res;
	}
	return res;
}
// END: Helper Functions
