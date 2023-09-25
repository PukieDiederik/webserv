#include "ServerConfig.hpp"
#include "ServerUtils.hpp"
#include "ParserUtils.hpp"

#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <cstring>
#include <iostream>

/*
 *	Config file parser:
 *		1. Read line by line
 *		2. Comments are defined by hastag (#). Everything from hastag to EOL is comment
 *		3. Find block identifiers as keywords -> cgi; mime; server
 *		4. Each block is confined between brackets -> keywd_bracket; subkeywd_bracket
 *		5. Any configuration errors is thrown as exception:
 *			- Unrecognized keyword / subkeyword (block identifier)
 *			- Unclosed brackets
 *			- Missing parameters
 * */

RouteCfg::RouteCfg() {
	this->is_redirect = false;
	this->redirect_to = "";
	this->root = "";
	this->cgi_enabled = false;
	this->auto_index = false;
	this->index = "";
}

RouteCfg::RouteCfg(const RouteCfg& copy) {
	*this = copy;
}

RouteCfg::~RouteCfg() { }

RouteCfg &RouteCfg::operator=(const RouteCfg& copy) {
	this->route_path = copy.route_path;
	this->is_redirect = copy.is_redirect;
	this->redirect_to = copy.redirect_to;
	this->root = copy.root;
	this->cgi_enabled = copy.cgi_enabled;
	this->auto_index = copy.auto_index;
	this->index = copy.index;
	this->accepted_methods = copy.accepted_methods;

	return (*this);
}

ServerCfg::ServerCfg() {
	this->host = "";
	this->port = -42;
	this->max_body_size = 0;
	this->root_dir = "";
}

ServerCfg::ServerCfg(const ServerCfg& copy) {
	*this = copy;
}

ServerCfg::~ServerCfg() { }

ServerCfg	&ServerCfg::operator=(const ServerCfg& copy) {
	this->host = copy.host;
	this->port = copy.port;
	this->server_names = copy.server_names;
	this->error_pages = copy.error_pages;
	this->max_body_size = copy.max_body_size;
	this->root_dir = copy.root_dir;
	this->routes = copy.routes;

	return (*this);
}

/*	@ServerConfig constructor:
 *		Desc:
 * 			Calls getline until keyword is found
 * 			Sets keywd_bracket true
 * 			Call respective block parser
 * 			Sets keywd_bracket false
 * 		@params:
 * 			const std::string&	-> path to file
 * 		@returns:
 * 			success:
 * 			error: throws corresponding error
*/

ServerConfig::ServerConfig() {}

ServerConfig::ServerConfig(const ServerConfig& copy) { *this = copy; }

ServerConfig::~ServerConfig() {
	// Deletion of cmds arrays
	for (std::vector<char **>::iterator it = _cgi_cmds.begin(); it != _cgi_cmds.end(); it++) {
		for (int i = 0; (*it)[i] != NULL; i++)
			delete[] (*it)[i];
		delete[] *it;
	}
	_cgi_cmds.clear();
}

ServerConfig& ServerConfig::operator=(const ServerConfig& copy)
{
	_cgi_cmds = copy._cgi_cmds;
	_cgi = copy._cgi;
    _mime = copy._mime;
    _servers = copy._servers;

	// deep copy _cgi_cmds
	for ( size_t i = 0; i < copy._cgi_cmds.size(); i++ ) {
		char**	og_cmd = copy._cgi_cmds[i];

		size_t	len = 0;
		while ( og_cmd[len] != NULL )
			len++;

		char**	cmd_array = new char*[len + 1];
		cmd_array[len] = NULL;

		for ( size_t j = 0; j < len; j++ ) {
			cmd_array[j] = new char[std::strlen( og_cmd[j] ) + 1];
			std::strcpy( cmd_array[j], og_cmd[j] );
		}

		_cgi_cmds.push_back( cmd_array );

	}

	_cgi = copy._cgi;

    return *this;
}

std::string ServerConfig::getMimeType(const std::string& filename) {
	std::string	extension = get_filename_extension(filename);
	if (extension.empty())
		return MIME_DEFAULT;

    const ServerConfig& sc = ServerConfig::getInstance();
    ServerConfig::mime_tab_t::const_iterator it = sc._mime.find(extension);
    if (it != sc._mime.end())
        return it->second;
    return MIME_DEFAULT;
}

ServerConfig ServerConfig::_instance;

void ServerConfig::initialize(const std::string &filepath)
{
    // ServerConfig::_instance = ServerConfig(filepath);

	if (filepath.empty()) throw std::runtime_error("Error: bad config file");

	std::ifstream	fd_conf;

	fd_conf.open(filepath.c_str());
	if (!(fd_conf.is_open())) throw std::runtime_error("Error: can't open file");
	std::string	curr_line;
	int		keyword;
	int		bad_line = 0;
	bool		keywd_bracket = false;
	while (std::getline(fd_conf, curr_line)) {
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		bad_line++;
		if (curr_line.empty())
			continue ;
		keyword = ParserUtils::identifyKeyword(curr_line, keywd_bracket);
		if (keyword == SERVER) {
			try {
				parseServer(bad_line, keywd_bracket, fd_conf);
			} catch (const std::exception &ex) {
				std::cout << "Error: server parser: " << std::flush;
				throw ;
			}
		} else if (keyword == CGI) {
			try {
				parseCgi(bad_line, keywd_bracket, fd_conf);
			} catch (const std::exception &ex) {
				std::cout << "Error: cgi parser: " << std::flush;
				throw ;
			}
		} else if (keyword == MIME) {
			try {
				parseMime(bad_line, keywd_bracket, fd_conf);
			} catch (const std::exception &ex) {
				std::cout << "Error: mime parser: " << std::flush;
				throw ;
			}
		} else if (keyword == ERROR) {
			throw std::runtime_error("Error: bad line[" + ParserUtils::intToString(bad_line) + "]");
		}
	}

	if (bad_line == 0)  {
		fd_conf.close();
		throw std::runtime_error("Error: config file empty");
	}
	try {
		checker();
	} catch (const std::exception &ex) {
		std::cout << "Error: checker: " << std::flush;
		throw ;
	}

}
ServerConfig& ServerConfig::getInstance()
{
    return ServerConfig::_instance;
}

bool	ServerConfig::isCgiScript(std::string filename)
{
	std::string	extension = get_filename_extension(filename);
	if (extension.empty())
		return false;

	const ServerConfig&	sc = ServerConfig::getInstance();
	for (std::map<std::string, char **>::const_iterator it = sc._cgi.begin(); it != sc._cgi.end(); it++)
	{
		if (extension == it->first)
			return true;
	}

	return false;
}

const std::string	ServerConfig::getExecutablePath(std::string filename)
{
	std::string	extension = get_filename_extension(filename);
	if (extension.empty())
		return "";

	std::string	result = "";

	const ServerConfig&	sc = ServerConfig::getInstance();
	for (std::map<std::string, char **>::const_iterator it = sc._cgi.begin(); it != sc._cgi.end(); it++)
	{
		if (extension == it->first)
		{
			for (int i = 0; (it->second)[i] != NULL; i++)
			{
				result += (it->second)[i];
				if ((it->second)[i + 1] != NULL)
					result += " ";
			}
			break;
		}
	}

	return result;
}
