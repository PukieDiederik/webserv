#include "ServerConfig.hpp"
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

//<<<<<<< HEAD
//RouteCfg::RouteCfg() { }
//RouteCfg::RouteCfg(const RouteCfg& copy)
//{
//    name = copy.name;
//    is_redirect = copy.is_redirect;
//    redirect_to = copy.redirect_to;
//    root = copy.root;
//    cgi_enabled = copy.cgi_enabled;
//    auto_index = copy.auto_index;
//    index = copy.index;
//    accepted_methods = copy.accepted_methods;
//}
//RouteCfg::~RouteCfg() { }
//RouteCfg RouteCfg::operator=(const RouteCfg& copy)
//{
//    name = copy.name;
//    is_redirect = copy.is_redirect;
//    redirect_to = copy.redirect_to;
//    root = copy.root;
//    cgi_enabled = copy.cgi_enabled;
//    auto_index = copy.auto_index;
//    index = copy.index;
//    accepted_methods = copy.accepted_methods;
//
//    return *this;
//}
//
//ServerCfg::ServerCfg() { }
//ServerCfg::ServerCfg(const ServerCfg& copy)
//{
//    port = copy.port;
//    server_names = copy.server_names;
//
//    error_pages = copy.error_pages;
//    max_body_size = copy.max_body_size;
//
//    root_dir = copy.root_dir;
//    routes = copy.routes;
//}
//ServerCfg::~ServerCfg() { }
//ServerCfg ServerCfg::operator=(const ServerCfg& copy)
//{
//    port = copy.port;
//    server_names = copy.server_names;
//
//    error_pages = copy.error_pages;
//    max_body_size = copy.max_body_size;
//
//    root_dir = copy.root_dir;
//    routes = copy.routes;
//    return *this;
//}
//
//
//// ServerConfig
//ServerConfig::ServerConfig(const std::string& file) { (void)file; }
//ServerConfig::ServerConfig(const ServerConfig& copy)
//{
//    mime = copy.mime;
//    servers = copy.servers;
//}
//=======
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
 * 		Calls getline until keyword is found
 * 		Sets keywd_bracket true
 * 		Call respective block parser
 * 		Sets keywd_bracket false
*/
ServerConfig::ServerConfig(const std::string& filepath) {
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


ServerConfig::ServerConfig(const ServerConfig& copy) { *this = copy; }
//>>>>>>> master

ServerConfig::~ServerConfig() {
	// Deletion of cmds arrays
	for (std::vector<char **>::iterator it = _cgi_cmds.begin(); it != _cgi_cmds.end(); it++) {
		for (int i = 0; (*it)[i] != NULL; i++)
			delete[] (*it)[i];
		delete[] *it;
	}
}

ServerConfig& ServerConfig::operator=(const ServerConfig& copy)
{
    _mime = copy._mime;
    _servers = copy._servers;
    return *this;
}

// To populate the 'mimeTypes' variable
std::map<std::string, std::string> createMimeTypes() {
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".pdf"] =  "application/pdf";

	mimeTypes[".mp3"] =  "audio/mpeg";

	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".jpg"] =  "image/jpeg";
	mimeTypes[".png"] =  "image/png";
	mimeTypes[".svg"] =  "image/svg+xml";

	mimeTypes[".css"] =  "text/css";
	mimeTypes[".html"] = "text/html";
	mimeTypes[".htm"] =  "text/html";
	mimeTypes[".js"] =   "text/javascript";

	mimeTypes[".mp4"] =  "video/mp4";

	return mimeTypes;
}

const std::map<std::string, std::string> ServerConfig::mimeTypes = createMimeTypes();

std::string ServerConfig::getMimeType(const std::string& filename) {
    size_t dotPos = filename.rfind('.');
    if (dotPos != std::string::npos) {
        std::string extension = filename.substr(dotPos);
        std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
        if (it != mimeTypes.end()) {
            return it->second;
        }
    }

    return "text/plain";
}
