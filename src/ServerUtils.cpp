#include "ServerUtils.hpp"
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <iostream>

std::string	removeSlashDups( std::string str) {
	std::string result = str;
	std::string::size_type pos = 0;
	while ((pos = result.find("//", pos)) != std::string::npos)
		result.erase(pos, 1);
	return result;
}

/*
 *  startsWith:
 *      Check if string starts with substring
 * */
bool    startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }

    return !str.compare(0, prefix.length(), prefix);
}

std::string	find_remove( const std::string& str, char flag ) {
	std::string	return_str = str;
	std::string::size_type	pos = return_str.find( '?' );

	if ( pos != std::string::npos )
		return_str.erase( pos );

	return return_str;
}

/*
 *  get_path:
 *      Retrieves path from request
 * */
std::string	get_path(const HttpRequest& req, RouteCfg* route)
{
	return get_path(req.target(), route);
}

std::string	get_path(std::string req_target, RouteCfg* route)
{
	std::string path;

	if (startsWith(req_target, route->route_path))
		req_target.erase(0, route->route_path.length());

	// ignore everything after ? or #
	req_target = find_remove( req_target, '?' );
	req_target = find_remove( req_target, '#' );

	path = route->root + "/" + req_target;
	path = removeSlashDups(path);

	return path;
}

std::string	get_path( std::string error_page, ServerCfg& _cfg ) {
	return removeSlashDups( _cfg.root_dir + error_page );
}

/*
*   @get_path_index:
*       If auto_index is true, returns user request
*       If not, return predefined index
*
*/
int	index_path( const HttpRequest& req, RouteCfg* route, std::string& path ) {
    // if is a file return request
    if ( is_file( path ) ) {
        return 0;
    }
    else if ( is_directory( path ) ) {
        std::vector<std::string>    dir_listing = list_dir( path );
        
        // If index.html exists in said folder, return request
        std::vector<std::string>::iterator it = std::find(dir_listing.begin(), dir_listing.end(), route->index);

        if ( it != dir_listing.end() ) {
		    std::string	part_path = route->route_path;
		    if ( !route->route_path.empty() && route->route_path[route->route_path.size() - 1] == '/' )
			    part_path = route->route_path.substr(0, route->route_path.size() - 1);
		    // Remove dup when root is defined in route
		    path = removeSlashDups( route->root + /* "/" + part_path + */ "/" + route->index );
	    	return 0;
        } else if ( route->auto_index ) { // else if (auto_index on), return list of contents
            return 1;
        }
    }

    return 2;
}


/*
*   @replace_occurrence:
*       Replace all uccurrences of str2 by str3 in str1
*/
int	replace_occurrence( std::string& str, const std::string& occurr, const std::string& replacement) {
    size_t start_pos = 0;
    int i = 0;

    while( ( start_pos = str.find( occurr, start_pos)) != std::string::npos ) {
        str.replace( start_pos, occurr.length(), replacement );
        start_pos += replacement.length();
        i++;
    }
    
    return i;
}

/*
*   @is_accepted_method:
*    Checks if the requested method is accepted by the route
*/
bool	is_accepted_method( RouteCfg* route, const std::string method ) {
    return std::find(route->accepted_methods.begin(), route->accepted_methods.end(), method) != route->accepted_methods.end();
}

/*
*   @is_file:
*       Checks if given path points to a file
*        not end in '/'
*/
bool    is_file( const std::string& path ) {
    // Path must not end in '/'
    if ( path[path.size() - 1] == '/' ) return false;

    struct stat buf;
    
    // Any error returns false
    if ( stat(path.c_str(), &buf) != 0 ) return false;

    // Check if is a file
    return S_ISREG( buf.st_mode );
}

/*
*   @is_directory:
*/
bool	is_directory( const std::string& path ) {
    struct stat buf;
    
    // Any error returns false
    if ( stat(path.c_str(), &buf) != 0 ) return false;
    
    // Check if is a dir
    return S_ISDIR( buf.st_mode );
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
