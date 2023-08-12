#ifndef __SERVERUTILS_HPP__
#define __SERVERUTILS_HPP__

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include <string>

std::string	remove_slash_dups( std::string str);
std::string	get_path(const HttpRequest& req, RouteCfg* route);
std::string	get_path(std::string req_target, RouteCfg* route);
int		index_path( const HttpRequest& req, RouteCfg* route, std::string& path );
int		replace_occurrence( std::string& str, const std::string& occurr, const std::string& replacement);
bool	is_accepted_method( RouteCfg* route, const std::string method );
bool    is_file( const std::string& path );
bool	is_directory( const std::string& path );
std::vector<std::string>    list_dir( const std::string& path );

#endif
