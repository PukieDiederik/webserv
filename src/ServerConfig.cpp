#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

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
	this->redirect_to = "notset";
	this->root = "nopath";
	this->cgi_enabled = false;
	this->auto_index = false;
	this->index = "notgiven";
}
//RouteCfg::RouteCfg(const RouteCfg& copy) { (void)copy; }
RouteCfg::~RouteCfg() { }
//RouteCfg RouteCfg::operator=(const RouteCfg& copy) { (void)copy; return *this; }

ServerCfg::ServerCfg() {
	this->host = "notdefined";
	this->port = -42;
	this->max_body_size = 0;
	this->root_dir = "nopath";
}
//ServerCfg::ServerCfg(const ServerCfg& copy) { (void)copy; }
ServerCfg::~ServerCfg() { }
//ServerCfg ServerCfg::operator=(const ServerCfg& copy) { (void)copy; return *this; }

void	ServerConfig::parseMime(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf) {
	std::string	curr_line;
	std::string	token, ltoken;

	while (!keywd_bracket) {
		std::getline(fd_conf, curr_line); bad_line++;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') throw std::runtime_error("Error: missing opening bracket: line " + ParserUtils::intToString(bad_line));
		else if (token.compare("{") == 0) keywd_bracket = true;
	}
	while (std::getline(fd_conf, curr_line)) {
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		
		std::vector<std::string>	file_extensions;
		if (token.compare("mime_add") == 0) {
			std::getline(iss_curr_line, token, ' ');
			try { ParserUtils::getParams(token, file_extensions, bad_line);
			} catch (std::exception &ex) {
				throw ;
			}
			std::getline(iss_curr_line, token, ' ');
			if (token[0] != '"' || token[token.length() - 1] != '"' || token.length() < 3) throw std::runtime_error("Error: bad mime config: line: " + ParserUtils::intToString(bad_line));
			for (std::vector<std::string>::iterator it = file_extensions.begin(); it != file_extensions.end(); it++) {
				_mime.insert(std::make_pair(*it, ParserUtils::removeDelimiters(token)));
			}
			std::getline(iss_curr_line, ltoken, ' ');
			if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
		}
		else if (token.compare("}") == 0) { keywd_bracket = false; return ; }
		else if (token[0] == '#') continue ;
		else
			throw std::runtime_error("Error: bad server parameter: line: " + ParserUtils::intToString(bad_line));
	}
	if (fd_conf.eof()) throw std::runtime_error("Error: missing closing bracket");
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
	if (fd_conf.is_open()) {
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
				std::cout << "parse cgi" << std::endl;
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

	} else
		throw std::runtime_error("Error: can't open file");
	try {
		checker();
	} catch (const std::exception &ex) {
		std::cout << "Error: checker: " << std::flush;
		throw ;
	}
}


ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
