#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

#include <cstdlib>
#include <iostream>

void	route_checker( ServerCfg& server, bool cgi_config ) {
		int j = 1;
		for ( std::vector<RouteCfg>::iterator route = server.routes.begin(); route != server.routes.end(); route++ ) {
			if ( VERBOSE ) {
				std::cout << "\tRoute[" << j++ << "]" << std::endl;
				std::cout << "\t\tRoute path:\n\t\t\t[" << (*route).route_path << "]" << std::endl;
			}

			if ( ( *route ).is_redirect && ( *route ).redirect_to.empty() ) throw std::runtime_error( "missing redirection config" );
		    if ( ( *route ).is_redirect && VERBOSE) std::cout << "\t\tRoute redirection:\n\t\t\t[" << ( *route ).redirect_to << "]" << std::endl;

			if ( ( *route ).root.empty() )  {
				// if root tag was not given, then the route_root = server_root
				( *route ).root = ParserUtils::removeSlashDups( server.root_dir + ( *route ).route_path );
			}
			if ( !( ParserUtils::isValidPath( ( *route ).root ) ) ) throw std::runtime_error( "invalid route root path (path must be absolute)" );
			if ( ( *route ).root[0] != '/' ) ( *route ).root = ParserUtils::removeSlashDups( server.root_dir + "/" + ( *route ).root );

			if ( VERBOSE )
				std::cout << "\t\tRoute root:\n\t\t\t[" << ( *route ).root << "]" << std::endl;

			if ( VERBOSE )
				std::cout << "\t\tRoute cgi enabled:\n\t\t\t[" << std::flush;
			if (( *route ).cgi_enabled && VERBOSE ) {
				std::cout << "yes]" << std::endl;
				if ( cgi_config ) throw std::runtime_error( "cgi enabled but missing cgi config" );
			}
			else if ( VERBOSE )
				std::cout << "no]" << std::endl;

			if ( !( ( *route ).auto_index ) && ( *route ).index.empty() && !( ( *route ).is_redirect ) ) throw std::runtime_error( "missing index config" );
			if ( ( *route ).auto_index && VERBOSE ) std::cout << "\t\tRoute auto_index\n\t\t\t[yes]" << std::endl;

			if ( ( *route ).auto_index && !( ( *route ).index.empty() ) ) throw std::runtime_error( "both auto_index and manual index set" );
			if ( VERBOSE )
				std::cout << "\t\tRoute index:\n\t\t\t[" << ( *route ).index << "]" << std::endl;

			if ( !( ( *route ).accepted_methods.empty() ) ) {
				if ( VERBOSE )
					std::cout << "\t\tRoute accepted methods:" << std::endl;
				for ( std::vector<std::string>::iterator method = ( *route ).accepted_methods.begin(); method != ( *route ).accepted_methods.end(); method++ ) {
					if ( VERBOSE )
						std::cout << "\t\t\t[" << *method << "]" << std::endl;
				}
			}
		}

}

void	server_checker( std::vector<ServerCfg>& servers, bool cgi_cmds ) {

	int	server_count = 1;
	std::map<int, std::string>	ports;

	for ( std::vector<ServerCfg>::iterator server = servers.begin(); server != servers.end(); server++ ) {
		if ( VERBOSE )
			std::cout << "\nServer[" << server_count++ << "]:" << std::endl;

		if ( ( *server ).host.empty() ) throw std::runtime_error( "missing ipv4 (hostname) config" );

		if ( VERBOSE )
			std::cout << "\tServer host:\n\t\t[" << ( *server ).host << "]" << std::endl;

		if ( ( *server ).port == -42 ) throw std::runtime_error( "missing port config" );
		std::map<int, std::string>::iterator it = ports.find( (*server).port );
		if ( it != ports.end() && it->second == (*server).host) throw std::runtime_error( "port already in use by host: " + it->second);
		ports.insert( std::make_pair( (*server).port, (*server).server_names[0] ));
		if  (VERBOSE )
			std::cout << "\tServer port:\n\t\t[" << ( *server ).port << "]" << std::endl;

		if ( ( *server ).server_names.empty() ) throw std::runtime_error( "missing server name(s) config" );
		if ( VERBOSE )
			std::cout << "\tServer name(s): " << std::endl;
		for ( std::vector<std::string>::iterator server_name = ( *server ).server_names.begin(); server_name != ( *server ).server_names.end(); server_name++ ) {
			if ( VERBOSE )
				std::cout << "\t\t[" << *server_name << "]" << std::endl;
		}

		// not necessary for config
		if ( VERBOSE )
			std::cout << "\tServer error_pages: " << std::endl;
		for ( std::map<short, std::string>::iterator error_page = ( *server ).error_pages.begin(); error_page != ( *server ).error_pages.end(); error_page++ )
			if ( VERBOSE )
				std::cout << "\t\t[" << error_page->first << "] -> [" << error_page->second << "]" << std::endl;

		// if 0, any amount is valid
		if ( VERBOSE )
			std::cout << "\tServer max_body_size:\n\t\t[" << ( *server ).max_body_size << "]" << std::endl;


		if ( ( *server ).root_dir.empty()) throw std::runtime_error( "missing root dir config" );
		if ( VERBOSE )
			std::cout << "\tServer root dir:\n\t\t[" << ( *server ).root_dir << "]" << std::endl;

		if ( ( *server ).routes.empty()) throw std::runtime_error( "server must have one route" );
		if ( VERBOSE )
			std::cout << "\tServer route(s):" << std::endl;
		route_checker( *server, cgi_cmds );
	}
}

void	ServerConfig::checker() {
	if ( _servers.empty() ) throw std::runtime_error("no server config found in config file.");

		server_checker( _servers, _cgi_cmds.empty() );

		if ( VERBOSE && !( _cgi_cmds.empty() ) ) {
			std::cout << "\tServer cgi: " << std::endl;
			for ( std::map<std::string, char **>::iterator cgi_cmd = _cgi.begin(); cgi_cmd != _cgi.end(); cgi_cmd++ ) {
				std::cout << "\t\t[" << cgi_cmd->first << "] -> [" << std::flush;
				for ( int i = 0; ( cgi_cmd->second )[i] != NULL; i++ ) {
					std::cout << ( cgi_cmd->second )[i] << std::flush;
					if ( ( cgi_cmd->second)[i + 1] != NULL )
						std::cout << " " << std::flush;
				}
				std::cout << "]" << std::endl;
			}
		}

		if ( VERBOSE )
			std::cout << "\tServer mimes:" << std::endl;;
		for ( std::map<std::string, std::string>::iterator mime = _mime.begin(); mime != _mime.end(); mime++ ) {
			if ( VERBOSE )
				std::cout << "\t\t[" << mime->first << "] -> [" << mime->second << "]" << std::endl;
		}
}
