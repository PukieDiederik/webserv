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
#include <cstring>

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

/*	@parseCgi:
 *		Checks if a cgi block was already found, if so throws error
 *		Checks for opening and closing brackets, throws error if not found
 *		Reads line by line until token "cgi_add is found"
 *		Adds command to std::vector<char **> _cgi_cmds
 *		Populates std::map<std::string, std::string> _mime with all the file extensions corresponding to said command
 *
 * */
void	ServerConfig::parseCgi(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf) {
	if (!(_cgi_cmds.empty())) throw std::runtime_error("Error: multiple cgi blocks: line: " + ParserUtils::intToString(bad_line));
	std::string	curr_line;
	std::string	token, ltoken, nltoken;

	while (!keywd_bracket) {
		std::getline(fd_conf, curr_line); bad_line++;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') throw std::runtime_error("Error: missing opening bracket: line " + ParserUtils::intToString(bad_line));
		else if (token.compare("{") == 0) keywd_bracket = true;
	}

	std::vector<std::string>	file_extensions;
	std::vector<std::string>	command;
	while (std::getline(fd_conf, curr_line)) {
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token == "cgi_add") {
			//get file extensions from next token
			std::getline(iss_curr_line, token, ' ');
			ParserUtils::getParams(token, file_extensions, bad_line);
			
			//get command from next token
			std::getline(iss_curr_line, ltoken, ' ');
			ParserUtils::getParams(ltoken, command, bad_line);
			
			//add command to list of commands
			char	**cmd_array = new char*[command.size() + 1]; //add to destructor!!
			cmd_array[command.size()] = NULL;
			for (size_t i = 0; i < command.size(); i++) {
				cmd_array[i] = new char[command[i].size() + 1];
				std::strcpy(cmd_array[i], command[i].c_str());
			}
			_cgi_cmds.push_back(cmd_array);

			//populate _cgi with all the file_extensions to the corresponding cmd
			for (std::vector<std::string>::iterator it = file_extensions.begin(); it != file_extensions.end(); it++) {
				_cgi.insert(std::make_pair(*it, cmd_array));
			}

		}

		else if (token.compare("}") == 0) { keywd_bracket = false; return ; }
		else if (token[0] == '#') continue ;
		else
			throw std::runtime_error("Error: bad server parameter: line: " + ParserUtils::intToString(bad_line));
	}
	if (fd_conf.eof()) throw std::runtime_error("Error: missing closing bracket");
}

/*	@parseMime:
 *		Checks for opening and closing brackets, throws error if not found
 *		Reads line by line until token "mime_add is found"
 *		Populates std::map<std::string, std::string> _mime with all the file extensions corresponding to one single content-type
 *
 * */
void	ServerConfig::parseMime(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf) {
	std::string	curr_line;
	std::string	token, ltoken, nltoken;

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
			ParserUtils::getParams(token, file_extensions, bad_line);
			std::getline(iss_curr_line, ltoken, ' ');
			ltoken = ParserUtils::removeDelimiters(ltoken);
			for (std::vector<std::string>::iterator it = file_extensions.begin(); it != file_extensions.end(); it++) {
				_mime.insert(std::make_pair(*it, ltoken));
			}
			std::getline(iss_curr_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
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


ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() {
	// deletion of cmds arrays
	for (std::vector<char **>::iterator it = _cgi_cmds.begin(); it != _cgi_cmds.end(); it++) {
		for (int i = 0; (*it)[i] != NULL; i++)
			delete[] (*it)[i];
		delete[] *it;
	}
}

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
