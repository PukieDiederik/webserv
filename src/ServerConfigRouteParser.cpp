#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

#include <sstream>
#include <iostream>


/*	@parseAutoIndex:
 *		Checks for multiple auto_index definitions
 *		Sets auto_index state to true
 *		Checks for any additional tokens in string, if not comments throws error
 *
 */
void	parseAutoIndex(RouteCfg &route_conf, std::istringstream &iss_c_line, int &bad_line) {
	std::string	nltoken;

	if (route_conf.auto_index == true) throw std::runtime_error("Error: multiple index definitions: line: " + ParserUtils::intToString(bad_line));
	route_conf.auto_index = true;

	std::getline(iss_c_line, nltoken, ' ');
	if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
}

/*	@parseIndex:
 *		Checks if multiple index definitions
 *		Parses and checks if is valid token to route_conf
 *		Checks if any additional tokens in string, if not comments throws error
 *
 */
void	parseIndex(RouteCfg &route_conf, std::istringstream &iss_c_line, int &bad_line) {
	std::string token, ntoken;

	if (!(route_conf.index.empty())) throw std::runtime_error("Error: multiple index definitions: line: " + ParserUtils::intToString(bad_line));

	std::getline(iss_c_line, token, ' ');

	token = ParserUtils::removeDelimiters(token);

	route_conf.index = token;

	std::getline(iss_c_line, ntoken, ' ');
	if (!(ntoken.empty()) && ntoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
}

/*	@parseEnableCgi:
 *		Checks if multiple enable_cgi definitions
 *		Sets the state if cgi_enabled to true
 *		Checks if any additional tokens in string, if not comments throws error
 *
 */
void	parseEnableCgi(RouteCfg &route_conf, std::istringstream &iss_c_line, int &bad_line) {
	std::string	token;

	if (route_conf.cgi_enabled == true) throw std::runtime_error("Error: multiple enable_cgi definitions: line: " + ParserUtils::intToString(bad_line));

	route_conf.cgi_enabled = true;

	std::getline(iss_c_line, token, ' ');
	if (!(token.empty()) && token[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
}

/*	@parseRoot:
 *		Checks if multiple root definitions
 *		Parses and adds the root_path token to route_conf
 *		Checks if any additional tokens in string, if not comments throws error
 *
 */
void	parseRoot(RouteCfg &route_conf, std::istringstream &iss_c_line, int &bad_line) {
	std::string	token, ntoken;
	if (!(route_conf.root.empty())) throw std::runtime_error("Error: multiple root definitions: line: " + ParserUtils::intToString(bad_line));

	std::getline(iss_c_line, token, ' ');
	token = ParserUtils::removeDelimiters(token);
	for (int i = 0; token[i] != '\0'; i++) {
		if (token[i] == '/' && token[i + 1] == '/') throw std::runtime_error("Error: invalid root_path: line: " + ParserUtils::intToString(bad_line));
	}

	route_conf.root = token;

	std::getline(iss_c_line, ntoken, ' ');
	if (!(ntoken.empty()) && ntoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line)); 
}

/*	@parseMethods:
 *		Checks if multiple methods definitions
 *		Parses and adds the root_path token to route_conf
 *		Checks if any additional tokens in string, if not comments throws error
 *
 */
void	parseMethods(RouteCfg &route_conf, std::istringstream &iss_c_line, int &bad_line) {
	std::string	token, ntoken;

	if (!(route_conf.accepted_methods.empty())) throw std::runtime_error("Error: multiple methods definitions: line: " + ParserUtils::intToString(bad_line));

	std::getline(iss_c_line, token, ' ');
	if (token[0] != '[' || token[token.length() - 1] != ']' || token.length() < 5) throw std::runtime_error("Error: invalid methods: line: " + ParserUtils::intToString(bad_line));
	
	ParserUtils::getParams(token, route_conf.accepted_methods, bad_line);

	std::getline(iss_c_line, ntoken, ' ');
	if (!(ntoken.empty()) && ntoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
}

/*	@parseRedirect:
 *		Checks if multiple redirect definitions
 *		Parses and adds the redirect token to route_conf
 *		Checks if any additional tokens in string, if not comments throws error
 *
 */
void	parseRedirect(RouteCfg &route_conf, std::istringstream &iss_c_line, int &bad_line) {
	std::string	token, ntoken;

	if (route_conf.is_redirect == true) throw std::runtime_error("Error: multiple redirection definitions: line: " + ParserUtils::intToString(bad_line));

	std::getline(iss_c_line, token, ' ');
	token = ParserUtils::removeDelimiters(token);
	if (ParserUtils::isValidURL(token)) { route_conf.is_redirect = true; route_conf.redirect_to = token; }
	else
		throw std::runtime_error("Error: invalid redirection: line: " + ParserUtils::intToString(bad_line));

	std::getline(iss_c_line, ntoken, ' ');
	if (!(ntoken.empty()) && ntoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
}

/*	@parseServerRoute:
 *		Looks for opening subkeywd_bracket ({)
 *		Calls getline parsing each subkeywd to the RouteCfg object
 *
 */
void	ServerConfig::parseServerRoute(std::string &curr_line, ServerCfg &server_conf, int &bad_line, std::ifstream &fd_conf) {
	RouteCfg	route_conf;
	bool		subkeywd_bracket = false;

	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if (token[1] != '/')
		throw std::runtime_error("Error: invalid route_path: line: " + ParserUtils::intToString(bad_line));

	route_conf.route_path = ParserUtils::removeDelimiters(token);

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#' && ltoken.compare("{") != 0) {
		throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(bad_line));
	} else if (ltoken.compare("{") == 0) subkeywd_bracket = true;

	while (!subkeywd_bracket) {
		std::getline(fd_conf, curr_line); bad_line++;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') throw std::runtime_error("Error: missing opening bracket: line " + ParserUtils::intToString(bad_line));
		else if (token.compare("{") == 0) subkeywd_bracket = true;
	}

	while (getline(fd_conf, curr_line)) {
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_c_line(curr_line);
		std::string		ntoken, nltoken;

		std::getline(iss_c_line, ntoken, ' ');
		if (ntoken.compare("}") == 0) {
			subkeywd_bracket = false; server_conf.routes.push_back(route_conf); return ;
		} else if (ntoken.compare("auto_index") == 0) {
			parseAutoIndex(route_conf, iss_c_line, bad_line);
		} else if (ntoken.compare("index") == 0) {
			parseIndex(route_conf, iss_c_line, bad_line);
		} else if (ntoken.compare("enable_cgi") == 0) {
			parseEnableCgi(route_conf, iss_c_line, bad_line);
		} else if (ntoken.compare("root") == 0) {
			parseRoot(route_conf, iss_c_line, bad_line);
		} else if (ntoken.compare("methods") == 0) {
			parseMethods(route_conf, iss_c_line, bad_line);
		} else if (ntoken.compare("redirect") == 0) {
			parseRedirect(route_conf, iss_c_line, bad_line);
		} else if ( ntoken != "}" && ntoken[0] != '#' )
			throw std::runtime_error( "Error: missing closing bracket" + ParserUtils::intToString( bad_line) );

	}
	if (fd_conf.eof()) throw std::runtime_error("Error: missing closing bracket in route_config");	
}
