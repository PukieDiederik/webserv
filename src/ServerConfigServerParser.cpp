#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

/*	@parseServerHost:
 *		Checks if host is already set, if so throws error
 *		Atoi's each of ipv4 value so to check if >= 0 && <= 255
 *		Checks for additional token in string
 *
 * */
void	ServerConfig::parseServerHost(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.host.compare("notdefined") != 0) throw std::runtime_error("Error: multiple host ipv4 config: line: " + ParserUtils::intToString(_bad_line));

	std::string		token, ntoken, ltoken;
	std::istringstream	iss_curr_line(curr_line);

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');
	
	if (token.compare("localhost") == 0) {
		server_conf.host = "127.0.0.0";
		std::getline(iss_curr_line, ltoken, ' ');
		if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 
		return ;
	}
	if (token.length() < 8 || ParserUtils::countCharOccurs('.', token) != 3) throw ("Error: bad host config: line: " + ParserUtils::intToString(_bad_line));

	if (!(ParserUtils::isValidIp(token))) throw std::runtime_error("Error: invalid ipv4 address: line :" + ParserUtils::intToString(_bad_line));
	server_conf.host = token;

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 
}

/*	@parseServerPort:
 *		Checks if port is already set, throws error if resetting
 *		Atoi's given port, throws error is bad input
 *		Checks for additional token in string, throws erros if not comments
 * */
void	ServerConfig::parseServerPort(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.port > 0) throw std::runtime_error("Error: multiple port config: line: " + ParserUtils::intToString(_bad_line));

	std::string		token, ltoken;
	std::istringstream	iss_curr_line(curr_line);

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if ((server_conf.port = ParserUtils::atoi(token.c_str())) < 0) throw std::runtime_error("Error: bad port config: line: " + ParserUtils::intToString(_bad_line));
	else if (server_conf.port == 0) server_conf.port = 42; // assign available ports !!

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 
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
void	ServerConfig::parseServerNames(std::string curr_line, ServerCfg &server_conf) {
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	std::vector<std::string>	server_names;
	try { getParams(token, server_names);
	} catch (std::exception &ex) {
		throw ;
	}

	for (std::vector<std::string>::iterator	it = server_names.begin(); it != server_names.end(); it++) {
		if ((*it).length() > 253) throw std::runtime_error("Error: hostname bigger then 253 chars: line: " + ParserUtils::intToString(_bad_line));
		for (int i = 0; (*it)[i] != '\0'; i++) {
			if (((*it)[i] > 47 && (*it)[i] < 58) || ((*it)[i] > 96 && (*it)[i] < 123) || ((*it)[i] == '.' && i > 0) || ((*it)[i] == '-' && i > 0)) {
				if ((*it)[i] == '.' || (*it)[i] == '-') {
					if ((*it)[i - 1] == (*it)[i]) {
						throw std::runtime_error("Error: invalid hostname name: line: " + ParserUtils::intToString(_bad_line));
					}
				}
			}
			else
				throw std::runtime_error("Error: invalid hostname name: line: " + ParserUtils::intToString(_bad_line));
		}
		server_conf.server_names.push_back(*it);
	}

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 
}

/*	parserServerErrorPages:
 *		Calls @getParams to populate status_code_string
 *		Evaluates if status_code_string is valid, throw error if not, converts to short if it is
 *		Calls @getParams to populate page_path
 *		Evaluates if page_path if valid
 *		Checks if same number of status_codes and page_paths
 *		Populates dictionary with status_code -> page_path
 *		Checks if aditional tokens in string, if not comments throws error
 * */
void	ServerConfig::parseServerErrorPages(std::string curr_line, ServerCfg &server_conf) {
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken, ctoken;

	std::vector<std::string>	status_code_string;
	std::vector<short>		status_code;
	std::vector<std::string>	page_path;

	int				counter[2];
	counter[0] = 0;
	counter[1] = 0;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	try { getParams(token, status_code_string);
	} catch (std::exception &ex) {
		throw ;
	}

	for (std::vector<std::string>::iterator it = status_code_string.begin(); it != status_code_string.end(); it++) {
		short	st_cd = ParserUtils::atoi(*it);
		if (st_cd < 100 || st_cd > 599) throw std::runtime_error("Error: invalid http_status_code: line: " + ParserUtils::intToString(_bad_line));
		status_code.push_back(st_cd);
		counter[0]++;
	}
	
	std::getline(iss_curr_line, ltoken, ' ');
	if (ltoken.empty()) throw std::runtime_error("Error: missing http_status_code_pages_path: line: " + ParserUtils::intToString(_bad_line));

	try { getParams(ltoken, page_path);
	} catch (std::exception &ex) {
		throw ;
	}

	for (std::vector<std::string>::iterator it = page_path.begin(); it != page_path.end(); it++) {
		if (!(ParserUtils::isValidPath(*it))) throw std::runtime_error("Error: invalid error_page path: line: " + ParserUtils::intToString(_bad_line));
		counter[1]++;
	}

	if (counter[0] != counter[1]) throw std::runtime_error("Error: not same number of http_status_codes and page_path: line "  + ParserUtils::intToString(_bad_line));

	std::vector<std::string>::iterator	st = page_path.begin();
	for (std::vector<short>::iterator it = status_code.begin(); it != status_code.end(); it++) {
		server_conf.error_pages.insert(std::make_pair(*it, *st));
		st++;
	}

	std::getline(iss_curr_line, ctoken, ' ');
	if (!(ctoken.empty()) && ctoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 
}

/*	@parseServerMaxBodySize:
 *		Checks if value already set (multiple configs)
 *		Checks if its a valid number
 *		Checks if aditional tokens in string, if not comments throws error
 * */
void	ServerConfig::parseServerMaxBodySize(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.max_body_size > 0) throw std::runtime_error("Error: multiple max_body_size values: line: " + ParserUtils::intToString(_bad_line));
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	int	value = ParserUtils::atoi(token);

	if (value < 0) throw std::runtime_error("Error: invalid max_body_size value: line: " + ParserUtils::intToString(_bad_line));

	server_conf.max_body_size = value;

	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 

}

/*	@parseServerRoot:
 *		Checks if its a valid absolute path, throws error if not or multiple defenitions
 *		Checks if more token and present in line, if not comments throws error
 * */
void	ServerConfig::parseServerRoot(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.root_dir.compare("nopath") != 0) throw std::runtime_error("Error: mutiple root_dir definitions: line: " + ParserUtils::intToString(_bad_line));

	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if (token[0] != '"' || token[token.size() - 1] != '"' || token.length() < 3 || token[1] != '/')
		throw std::runtime_error("Error: invalid root_dir path: line: " + ParserUtils::intToString(_bad_line));

	token = ParserUtils::removeDelimiters(token);

	//find last occurence of '/' and remove everything after that
	int lastIndex = -1;
	for (int i = 0; token[i] != '\0'; ++i) {
		if (token[i] == '/') {
			lastIndex = i;
        	}
    	}

	std::string	dir_path = token;
	if (lastIndex != -1) dir_path.erase(lastIndex + 1);

	DIR	*checker = opendir(dir_path.c_str());
	if (checker == NULL) throw std::runtime_error("Error: invalid root_dir path: line: " + ParserUtils::intToString(_bad_line));
	closedir(checker);

	server_conf.root_dir = token;

	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ltoken.empty()) && ltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line)); 
}

/*	@parseServerRoute:
 *		Looks for opening subkeywd_bracket ({)
 *		Calls getline parsing each subkeywd to the RouteCfg object
 *
 * */
void	ServerConfig::parseServerRoute(std::string curr_line, ServerCfg &server_conf) {
	RouteCfg	route_conf;

	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if (token[0] != '"' || token[token.size() - 1] != '"' || token.length() < 3 || token[1] != '/')
		throw std::runtime_error("Error: invalid route_path: line: " + ParserUtils::intToString(_bad_line));

	route_conf.route_path = token;

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#' && ltoken.compare("{") != 0) {
		throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
	} else if (ltoken.compare("{") == 0) _subkeywd_bracket = true;

	while (!_subkeywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') throw std::runtime_error("Error: missing opening bracket: line " + ParserUtils::intToString(_bad_line));
		else if (token.compare("{") == 0) _subkeywd_bracket = true;
	}

	// maybe take this loop apart into diff funcs :/
	while (getline(_fd_conf, curr_line)) {
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		_bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_c_line(curr_line);
		std::string		ntoken, nltoken;

		std::getline(iss_c_line, ntoken, ' ');
		if (ntoken.compare("}") == 0) {
			_subkeywd_bracket = false; server_conf.routes.push_back(route_conf); return ;
		} else if (ntoken.compare("auto_index") == 0) {
			if (route_conf.auto_index == true) throw std::runtime_error("Error: multiple index definitions: line: " + ParserUtils::intToString(_bad_line));
			route_conf.auto_index = true;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
		} else if (ntoken.compare("index") == 0) {
			if (route_conf.index.compare("notgiven") != 0) throw std::runtime_error("Error: multiple index definitions: line: " + ParserUtils::intToString(_bad_line));
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '"' || ntoken[ntoken.length() - 1] != '"' || ntoken.length() < 3) throw std::runtime_error("Error: invalid index: line: " + ParserUtils::intToString(_bad_line));
			ntoken = ParserUtils::removeDelimiters(ntoken);
			route_conf.index = ntoken;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
		} else if (ntoken.compare("enable_cgi") == 0) {
			if (route_conf.cgi_enabled == true) throw std::runtime_error("Error: multiple enable_cgi definitions: line: " + ParserUtils::intToString(_bad_line));
			route_conf.cgi_enabled = true;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
		} else if (ntoken.compare("root") == 0) {
			if (route_conf.root.compare("nopath") != 0) throw std::runtime_error("Error: multiple root definitions: line: " + ParserUtils::intToString(_bad_line));
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '"' || ntoken[ntoken.length() - 1] != '"' || ntoken.length() < 3) throw std::runtime_error("Error: invalid root path: line: " + ParserUtils::intToString(_bad_line));
			ntoken = ParserUtils::removeDelimiters(ntoken);
			for (int i = 0; ntoken[i] != '\0'; i++) {
				if (ntoken[i] == '/' && ntoken[i + 1] == '/') throw std::runtime_error("Error: invalid root_path: line: " + ParserUtils::intToString(_bad_line));
			}
			route_conf.root = ntoken;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
		} else if (ntoken.compare("methods") == 0) {
			if (!(route_conf.accepted_methods.empty())) throw std::runtime_error("Error: multiple methods definitions: line: " + ParserUtils::intToString(_bad_line));
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '[' || ntoken[ntoken.length() - 1] != ']' || ntoken.length() < 5) throw std::runtime_error("Error: invalid methods: line: " + ParserUtils::intToString(_bad_line));
			try { getParams(ntoken, route_conf.accepted_methods);
			} catch (std::exception &ex) {
				throw ;
			}
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
		} else if (ntoken.compare("redirect") == 0) {
			if (route_conf.is_redirect == true) throw std::runtime_error("Error: multiple redirection definitions: line: " + ParserUtils::intToString(_bad_line));
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '"' || ntoken[ntoken.length() - 1] != '"' || ntoken.length() < 3) throw std::runtime_error("Error: invalid redirection: line: " + ParserUtils::intToString(_bad_line));
			ntoken = ParserUtils::removeDelimiters(ntoken);
			if (ParserUtils::isValidURL(ntoken)) { route_conf.is_redirect = true; route_conf.redirect_to = ntoken; }
			else
				throw std::runtime_error("Error: invalid redirection: line: " + ParserUtils::intToString(_bad_line));
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') throw std::runtime_error("Error: unexpected token: line: " + ParserUtils::intToString(_bad_line));
			
		}

	}
	if (_fd_conf.eof()) throw std::runtime_error("Error: missing closing bracket in route_config");	
}

/*	@parseServer:
 * 		Checks if inital '{' was found, if not finds it or throws error
 * 		If one subkeyword (block) is found, calls for its parsing
 * 		Stops if '}' is found or EOF
*/

void	ServerConfig::parseServer() {
	std::string	curr_line;
	std::string	token, ntoken;

	ServerCfg	server_conf;

	_subkeywd_bracket = false;
	
	while (!_keywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') throw std::runtime_error("Error: missing opening bracket: line " + ParserUtils::intToString(_bad_line));
		else if (token.compare("{") == '{') _keywd_bracket = true;
	}

	while (std::getline(_fd_conf, curr_line)) {
		_bad_line++;
		if (curr_line.empty()) continue ;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		if (ParserUtils::strAllSpaces(curr_line)) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		try {
			if (token.compare("host") == 0) parseServerHost(curr_line, server_conf);
			else if (token.compare("port") == 0) parseServerPort(curr_line, server_conf);
			else if (token.compare("server_names") == 0) parseServerNames(curr_line, server_conf);
			else if (token.compare("error_pages") == 0) parseServerErrorPages(curr_line, server_conf);
			else if (token.compare("max_body_size") == 0) parseServerMaxBodySize(curr_line, server_conf);
			else if (token.compare("root") == 0) parseServerRoot(curr_line, server_conf);
			else if (token.compare("route") == 0) parseServerRoute(curr_line, server_conf);
			else if (token[0] == '#') continue ;
			else if (token.compare("}") == 0) { _keywd_bracket = false; _servers.push_back(server_conf); return ; }
			else
				throw std::runtime_error("Error: bad server parameter: line: " + ParserUtils::intToString(_bad_line));
		} catch (std::exception &ex) {
			throw ;
		}
	}

	if (_fd_conf.eof()) throw std::runtime_error("Error: missing closing bracket");
}
