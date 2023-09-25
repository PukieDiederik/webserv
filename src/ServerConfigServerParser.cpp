#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

#include <sstream>
#include <unistd.h>

// check filepath
# include <sys/types.h>
# include <dirent.h>

/*	@parseServerHost:
 *		Checks if host is already set, if so throws error
 *		Atoi's each of ipv4 value so to check if >= 0 && <= 255
 *		Checks for additional token in string
 *
 * */
void	ServerConfig::parseServerHost(const std::string &curr_line, ServerCfg &server_conf, int &bad_line) {
	if (!(server_conf.host.empty())) throw std::runtime_error("Error: multiple host ipv4 config: line: " + ParserUtils::intToString(bad_line));

	std::string		token, ntoken, ltoken;
	std::istringstream	iss_curr_line(curr_line);

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');
	
	if (token.compare("localhost") == 0) {
		server_conf.host = "127.0.0.1";
		std::getline(iss_curr_line, ltoken, ' ');
		if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
		return ;
	}

	if (!(ParserUtils::isValidIp(token))) throw std::runtime_error("Error: invalid ipv4 address: line :" + ParserUtils::intToString(bad_line));
	server_conf.host = token;

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
}

/*	@parseServerPort:
 *		Checks if port is already set, throws error if resetting
 *		Atoi's given port, throws error is bad input
 *		Checks for additional token in string, throws erros if not comments
 * */
void	ServerConfig::parseServerPort(const std::string &curr_line, ServerCfg &server_conf, int &bad_line) {
	if (server_conf.port > 0) throw std::runtime_error("Error: multiple port config: line: " + ParserUtils::intToString(bad_line));

	std::string		token, ltoken;
	std::istringstream	iss_curr_line(curr_line);

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if ((server_conf.port = ParserUtils::atoi(token.c_str())) < 0) throw std::runtime_error("Error: bad port config: line: " + ParserUtils::intToString(bad_line));
	else if (server_conf.port == 0) server_conf.port = 42; // assign available ports !!

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
}

/*	@parseServerNames:
 *		Calls @getParams to get each server_name from container
 *		Validates name, throws error if invalid
 *			The entire hostname, including the dots, can be at most 253
 *			characters long.  Valid characters for hostnames are ASCII(7)
 *			letters from a to z, the digits from 0 to 9, and the hyphen (-).
 *			A hostname may not start with a hyphen.
 *		Checks for additional tokens in string, throws error if not comments
 * */
void	ServerConfig::parseServerNames(const std::string &curr_line, ServerCfg &server_conf, int &bad_line) {
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	std::vector<std::string>	server_names;
	try { ParserUtils::getParams(token, server_names, bad_line);
	} catch (std::exception &ex) {
		throw ;
	}

	for (std::vector<std::string>::iterator	it = server_names.begin(); it != server_names.end(); it++) {
		if ((*it).length() > 253) throw std::runtime_error("Error: hostname bigger then 253 chars: line: " + ParserUtils::intToString(bad_line));
		for (int i = 0; (*it)[i] != '\0'; i++) {
			if (((*it)[i] > 47 && (*it)[i] < 58) || ((*it)[i] > 96 && (*it)[i] < 123) || ((*it)[i] == '.' && i > 0) || ((*it)[i] == '-' && i > 0)) {
				if ((*it)[i] == '.' || (*it)[i] == '-') {
					if ((*it)[i - 1] == (*it)[i]) {
						throw std::runtime_error("Error: invalid hostname name: line: " + ParserUtils::intToString(bad_line));
					}
				}
			}
			else
				throw std::runtime_error("Error: invalid hostname name: line: " + ParserUtils::intToString(bad_line));
		}
		server_conf.server_names.push_back(*it);
	}

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
}

/*	@parseServerErrorPages:
 *		Reads first token, must ve valid status code (>=100 && <= 599)
 *		Checks if status code redefinition
 *		Reads second token, evaluates if valid path
 *		Populates dictionary _error_pages with one more entry (code->page_path)
 *		Checks if aditional tokens in string, if not comments throws error
 *
 * */
void	ServerConfig::parseServerErrorPages(const std::string &curr_line, ServerCfg &server_conf, int &bad_line) {
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken, nltoken;

	short			status_code;
	std::string		page_path;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	status_code = ParserUtils::atoi(token);

	// validate status_code
	if (status_code < 100 || status_code > 599) throw std::runtime_error("Error: invalid http_status_code: line: " + ParserUtils::intToString(bad_line));
	// check if status code redefinition
	if (server_conf.error_pages.count(status_code) > 0) throw std::runtime_error("Error: http_status_code redefinition: line: " + ParserUtils::intToString(bad_line));
 
	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ParserUtils::isValidPath(ParserUtils::removeDelimiters(ltoken)))) throw std::runtime_error("Error: invalid error_page path: line: " + ParserUtils::intToString(bad_line));

	server_conf.error_pages.insert(std::make_pair(status_code, ltoken));

	std::getline(iss_curr_line, nltoken, ' ');
	if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
}

/*	@parseServerMaxBodySize:
 *		Checks if value already set (multiple configs)
 *		Checks if its a valid number
 *		Checks if aditional tokens in string, if not comments throws error
 * */
void	ServerConfig::parseServerMaxBodySize(const std::string &curr_line, ServerCfg &server_conf, int &bad_line) {
	if (server_conf.max_body_size > 0) throw std::runtime_error("Error: multiple max_body_size values: line: " + ParserUtils::intToString(bad_line));
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	int	value = ParserUtils::atoi(token);

	if (value < 0) throw std::runtime_error("Error: invalid max_body_size value: line: " + ParserUtils::intToString(bad_line));

	server_conf.max_body_size = value;

	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 

}


/*	@parseServerRoot:
 *		Checks if its a valid absolute path, throws error if not or multiple defenitions
 *		Checks if more token and present in line, if not comments throws error
 * */
void	ServerConfig::parseServerRoot(const std::string &curr_line, ServerCfg &server_conf, int &bad_line) {
	if (!(server_conf.root_dir.empty())) throw std::runtime_error("Error: mutiple root_dir definitions: line: " + ParserUtils::intToString(bad_line));

	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if (token[1] != '/')
		throw std::runtime_error("Error: invalid root_dir path: line: " + ParserUtils::intToString(bad_line));

	token = ParserUtils::removeDelimiters(token);

	// Check if permission to access path is enough
	if ( access( token.c_str(), R_OK ) != 0 ) throw std::runtime_error( "Error: invalid root_dir path: line: " + ParserUtils::intToString( bad_line ) );

	// Check if path is valid
	DIR	*checker = opendir( token.c_str() );
	if ( checker == NULL ) throw std::runtime_error("Error: invalid root_dir path: line: " + ParserUtils::intToString(bad_line));
	closedir( checker );



	server_conf.root_dir = token;

	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
}

/*	@parseServer:
 * 		Checks if inital '{' was found, if not finds it or throws error
 * 		If one subkeyword (block) is found, calls for its parsing
 * 		Stops if '}' is found or EOF
*/

void	ServerConfig::parseServer(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf) {
	std::string	curr_line;
	std::string	token, ntoken;

	ServerCfg	server_conf;
	
	while (!keywd_bracket) {
		std::getline(fd_conf, curr_line); bad_line++;
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') throw std::runtime_error("Error: missing opening bracket: line " + ParserUtils::intToString(bad_line));
		else if (token.compare("{") == '{') keywd_bracket = true;
	}

	while (std::getline(fd_conf, curr_line)) {
		bad_line++;
		if (curr_line.empty()) continue ;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		if (ParserUtils::strAllSpaces(curr_line)) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		try {
			if (token.compare("host") == 0) parseServerHost(curr_line, server_conf, bad_line);
			else if (token.compare("port") == 0) parseServerPort(curr_line, server_conf, bad_line);
			else if (token.compare("server_names") == 0) parseServerNames(curr_line, server_conf, bad_line);
			else if (token.compare("error_page") == 0) parseServerErrorPages(curr_line, server_conf, bad_line);
			else if (token.compare("max_body_size") == 0) parseServerMaxBodySize(curr_line, server_conf, bad_line);
			else if (token.compare("root") == 0) parseServerRoot(curr_line, server_conf, bad_line);
			else if (token.compare("route") == 0) parseServerRoute(curr_line, server_conf, bad_line, fd_conf);
			else if (token[0] == '#') continue ;
			else if (token.compare("}") == 0) { keywd_bracket = false; _servers.push_back(server_conf); return ; }
			else
				throw std::runtime_error("Error: bad server parameter: line: " + ParserUtils::intToString(bad_line));
		} catch (std::exception &ex) {
			throw ;
		}
	}

	if (fd_conf.eof()) throw std::runtime_error("Error: missing closing bracket");
}
