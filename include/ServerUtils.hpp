#ifndef __SERVERUTILS_HPP__
#define __SERVERUTILS_HPP__

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include <string>

std::string	removeSlashDups( std::string str);
std::string	get_path(const HttpRequest& req, const RouteCfg* route);
std::string	get_path(std::string req_target, const RouteCfg* route);
std::string	get_path(std::string error_page, const ServerCfg& _cfg);
int		index_path(const RouteCfg* route, std::string& path );
int		replace_occurrence( std::string& str, const std::string& occurr, const std::string& replacement);
bool	is_accepted_method(const RouteCfg* route, const std::string method );
bool    is_file( const std::string& path );
bool	is_directory( const std::string& path );
std::vector<std::string>    list_dir( const std::string& path );
std::string	get_filename_extension(const std::string& filename);
int			find_delimiter(const std::string str, const std::string delimiter);
int			is_error_code(const std::string str);

#endif
