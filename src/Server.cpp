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
#include <string>

// POSIX for @is_file, @is_directory and @list_dir
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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
*   @is_file:
*       Checks if given path points to a file
*       Path must not end in '/'
*/
bool    is_file( const std::string& path ) {
    // Path must not end in '/'
    if ( path[path.size() - 1] == '/' ) return false;

    struct stat buf;
    
    // Any error returns false
    if ( stat(path.c_str(), &buf) != 0 ) return false;

    // Check if is a file
    return S_ISREG(buf.st_mode);
}

/*
*   @is_directory:
*/
bool is_directory(const std::string& path) {
    struct stat buf;
    
    // Any error returns false
    if ( stat(path.c_str(), &buf) != 0 ) return false;
    
    // Check if is a dir
    return S_ISDIR(buf.st_mode);
}

/*
*   @list_dir:
*       Returns a vector of strings with all files/folders inside a dir
*/
std::vector<std::string>    list_dir( const std::string& path ) {
    std::vector<std::string>    dir_listing;

    DIR* dir;
    struct dirent* ent;

    dir = opendir( path.c_str() );
    
    if ( dir != NULL ) {
        while ( ( ent = readdir( dir ) ) != NULL ) {
            dir_listing.push_back( ent->d_name );
        }
        closedir(dir);
    }
    return dir_listing;
}

#include <iostream>
/*
*   @get_path:
*       If auto_index is true, returns user request
*       If not, return predefined index
*
*/
int get_path(const HttpRequest& req, RouteCfg* route, std::string& path ) {
    // If root ends in '/', remove last char
    if (!route->root.empty() && route->root[route->root.size() - 1] == '/')
        route->root = route->root.substr(0, route->root.size() - 1);

    // Request is equal to relative path ('.') + root path + route path
    path = route->root + req.target();

    // if is a file return request
    if ( is_file( path ) ) {
        return 0;
    }
    else if ( is_directory( path ) ) {
        std::vector<std::string>    dir_listing = list_dir( path );
        
        // If index.html exists in said folder, return request
        std::vector<std::string>::iterator it = std::find(dir_listing.begin(), dir_listing.end(), "index.html");
        if ( it != dir_listing.end() ) {
            if (!route->route_path.empty() && route->route_path[route->route_path.size() - 1] == '/')
                route->route_path = route->route_path.substr(0, route->route_path.size() - 1);
            path = route->root + route->route_path + "/" + "index.html";
            return 0;
        } else if ( route->auto_index ) { // else if (auto_index on), return list of contents
            return 1;
        }
    }

    path = "";
    return 2;
}

/*
*   @replace_occurrence:
*       Replace all uccurrences of str2 by str3 in str1
*/
int replace_occurrence( std::string& str, const std::string& occurr, const std::string& replacement) {
    size_t start_pos = 0;
    int i = 0;

    while( ( start_pos = str.find( occurr, start_pos)) != std::string::npos ) {
        str.replace( start_pos, occurr.length(), replacement );
        start_pos += replacement.length();
        i++;
    }
    
    return i;
}

HttpResponse    list_dir_res( HttpResponse& res, const HttpRequest& req, std::string path) {
        res.body().clear();
        
        std::ifstream   file( DIRLISTING );
        std::string     line_buff;

        if ( !file.is_open() ) {
            // TODO: return 500 error page
            res.set_status(500, "Internal Server Error");
            return res;
        }
        
        std::string                 items;
        std::vector<std::string>    dir_listing = list_dir( path );
        for (size_t i = 0; i < dir_listing.size(); i++)
            items.append( "<li><a href=\"" + req.target() + "/" + dir_listing[i] + "\">" + dir_listing[i] + "</a></li>" );

        // Replace all occurs of [DIR] & [ITEMS] with dir name and dir listing
        while( std::getline( file, line_buff) ) {
            replace_occurrence( line_buff, "[DIR]", req.target() );
            replace_occurrence( line_buff, "[ITEMS]", items);
            res.body().append( line_buff );
        }

        res.set_status(200, "OK");
        res.set_header("Content-Type", "text/html");
        file.close();
        return res;
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

    // If req not a file nor index.html found return dir listing
    if ( get_path( req, route, path ) > 0 && !path.empty() ) return list_dir_res( res, req, path);

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
    if ( !(route->accepted_methods.empty()) ) {
        if ( !(is_accepted_method( route, req.method() ) ) )  {
            // TODO: return 405 error page
            res.set_status( 405, "Method Not Allowed" );
            return res;
        }
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
