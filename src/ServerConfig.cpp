#include "ServerConfig.hpp"
#include "RemoveTabs.hpp"
#include "ParserUtils.hpp"

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
	this->root = "nopath";
	this->cgi_enabled = false;
	this->auto_index = false;
	this->index = "notgiven";
}
//RouteCfg::RouteCfg(const RouteCfg& copy) { (void)copy; }
RouteCfg::~RouteCfg() { }
//RouteCfg RouteCfg::operator=(const RouteCfg& copy) { (void)copy; return *this; }

ServerCfg::ServerCfg() {
	this->port = -42;
	this->max_body_size = 0;
	this->root_dir = "nopath";
}
//ServerCfg::ServerCfg(const ServerCfg& copy) { (void)copy; }
ServerCfg::~ServerCfg() { }
//ServerCfg ServerCfg::operator=(const ServerCfg& copy) { (void)copy; return *this; }



/*	@isKeyword:
*		Evaluates if given line contains a keyword and its respective '{' (or just the keyword)
*		Returns error (5) for all other cases
*/
int	ServerConfig::isKeyword(std::string line) {
	int			value = 5;
	std::string		token, ntoken;
	std::istringstream	iss(line);

	std::getline(iss, token, ' ');
	if (token.compare("server") == 0) value = 1;
	else if (token.compare("cgi") == 0) value = 2;
	else if (token.compare("mime") == 0) value = 3;
	else if (token[0] == '#') return (4);

	std::getline(iss, ntoken, ' ');

	if (ntoken.compare(token) == 0) return (value);
	else if (ntoken[0] == '{') _keywd_bracket = true;
	else if (!ntoken.empty() && ntoken[0] != '#') value = 5;

	return (value);
}

/*	@getParams:
 *		Checks if string is delimited by brackets ('[]') and removes them
 *		Checks if each param is separated by a comma and removes them
 *		Checks if each param is delimited by quotes and removes them
 *		Populates a vector<string> with each param
*/
void	ServerConfig::getParams(std::string str, std::vector<std::string> &params) {
	std::string 		param;

	if (str[0] != '[' || str[str.length() - 1] != ']' || str.size() < 5) { std::cout << "throw error: bad parameter config: line: " << _bad_line << std::endl; return ; }
	else (str = ParserUtils::removeDelimiters(str));

	std::istringstream 	iss(str);

	while (std::getline(iss, param, ',')) {
		if (!param.empty()) {
			if (param[0] != '\"' || param[param.length() - 1] != '\"' || param.size() < 3) {
				std::cout << "throw error: bad parameter config: line: " << _bad_line << std::endl;
				return ;
			} else 	param = ParserUtils::removeDelimiters(param);
			params.push_back(param);
		}
	}
}

/*	@parseServerPort:
 *		Checks if port is already set, throws error if resetting
 *		Atoi's given port, throws error is bad input
 *		Checks for addiciontal token in string, throws erros if not comments
 * */
void	ServerConfig::parseServerPort(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.port > 0) { std::cout << "throw error: multiple port config: line: " << _bad_line << std::endl; return ; }

	std::string		token, ltoken;
	std::istringstream	iss_curr_line(curr_line);

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if ((server_conf.port = ParserUtils::atoi(token.c_str())) < 0) { std::cout << "throw error: bad port config: line: " << _bad_line << std::endl; return ; }
	else if (server_conf.port == 0) server_conf.port = 42; // assign available ports !!

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; } 
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
	getParams(token, server_names);

	for (std::vector<std::string>::iterator	it = server_names.begin(); it != server_names.end(); it++) {
		if ((*it).length() > 253) { std::cout << "throw error: hostname bigger then 253 chars: line: " << _bad_line << std::endl; return ; }
		for (int i = 0; (*it)[i] != '\0'; i++) {
			if (((*it)[i] > 47 && (*it)[i] < 58) || ((*it)[i] > 96 && (*it)[i] < 123) || ((*it)[i] == '.' && i > 0) || ((*it)[i] == '-' && i > 0)) {
				if ((*it)[i] == '.' || (*it)[i] == '-') {
					if ((*it)[i - 1] == (*it)[i]) {
						std::cout << "throw error: invalid hostname name: line: " << _bad_line << std::endl;
						return ;
					}
				}
			}
			else { std::cout << "throw error: invalid hostname name: line: " << _bad_line << std::endl; return ; }
		}
		server_conf.server_names.push_back(*it);
	}

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; } 
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
	std::string		token, ltoken;

	std::vector<std::string>	status_code_string;
	std::vector<short>		status_code;
	std::vector<std::string>	page_path;

	int				counter[2];
	counter[0] = 0;
	counter[1] = 0;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	getParams(token, status_code_string);

	for (std::vector<std::string>::iterator it = status_code_string.begin(); it != status_code_string.end(); it++) {
		short	st_cd = ParserUtils::atoi(*it);
		if (st_cd < 100 || st_cd > 599) { std::cout << "throw error: invalid http_status_code: line: " << _bad_line << std::endl; return ; }
		status_code.push_back(st_cd);
		counter[0]++;
	}
	
	std::getline(iss_curr_line, ltoken, ' ');
	if (ltoken.empty()) { std::cout << "throw error: missing http_status_code_pages_path: line: " << _bad_line << std::endl; return ; }

	getParams(ltoken, page_path);

	for (std::vector<std::string>::iterator it = page_path.begin(); it != page_path.end(); it++) {
		// evaluate path
		counter[1]++;
	}

	if (counter[0] != counter[1]) { std::cout << "throw error: not same number of http_status_codes and page_path: line " << _bad_line << std::endl; return ; }

	std::vector<std::string>::iterator	st = page_path.begin();
	for (std::vector<short>::iterator it = status_code.begin(); it != status_code.end(); it++) {
		server_conf.error_pages.insert(std::make_pair(*it, *st));
		st++;
	}

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; } 
}

/*	@parseServerMaxBodySize:
 *		Checks if value already set (multiple configs)
 *		Checks if its a valid number
 *		Checks if aditional tokens in string, if not comments throws error
 * */
void	ServerConfig::parseServerMaxBodySize(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.max_body_size > 0) { std::cout << "throw error: multiple max_body_size values: line: " << _bad_line << std::endl; return ; }
	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	int	value = ParserUtils::atoi(token);

	if (value < 0) {  std::cout << "throw error: invalid max_body_size value: line: " << _bad_line << std::endl; return ; }

	server_conf.max_body_size = value;

	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ltoken.empty()) && ltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; } 

}

/*	@parseServerRoot:
 *		Checks if its a valid absolute path, throws error if not or multiple defenitions
 *		Checks if more token and present in line, if not comments throws error
 * */
void	ServerConfig::parseServerRoot(std::string curr_line, ServerCfg &server_conf) {
	if (server_conf.root_dir.compare("nopath") != 0) { std::cout << "throw error: mutiple root_dir definitions: line: " << _bad_line << std::endl; return ; }

	std::istringstream	iss_curr_line(curr_line);
	std::string		token, ltoken;

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if (token[0] != '"' || token[token.size() - 1] != '"' || token.length() < 3 || token[1] != '/') {
		std::cout << "throw error: invalid root_dir path: line: " << _bad_line << std::endl; return ;
	}

	token = ParserUtils::removeDelimiters(token);

	//find last occurence of '/' and remove everything after that
	int lastIndex = -1;
	for (int i = 0; i < token.length(); ++i) {
		if (token[i] == '/') {
			lastIndex = i;
        	}
    	}

	std::string	dir_path = token;
	if (lastIndex != -1) dir_path.erase(lastIndex + 1);

	DIR	*checker = opendir(dir_path.c_str());
	if (checker == NULL) { std::cout << "throw error: invalid root_dir path: line: " << _bad_line << std::endl; return ; }
	closedir(checker);

	server_conf.root_dir = token;

	std::getline(iss_curr_line, ltoken, ' ');

	if (!(ltoken.empty()) && ltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; } 
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

	if (token[0] != '"' || token[token.size() - 1] != '"' || token.length() < 3 || token[1] != '/') {
		std::cout << "throw error: invalid route_path: line: " << _bad_line << std::endl; return ;
	}

	route_conf.route_path = token;

	std::getline(iss_curr_line, ltoken, ' ');
	if (!(ltoken.empty()) && ltoken[0] != '#' && ltoken.compare("{") != 0) {
		std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl;
	} else if (ltoken.compare("{") == 0) _subkeywd_bracket = true;

	while (!_subkeywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') { std::cout << "throw error: missing opening bracket: line " << _bad_line << std::endl; return ; }
		else if (token.compare("{") == 0) _subkeywd_bracket = true;
	}

	// maybe take this loop apart into diff funcs :/
	while (getline(_fd_conf, curr_line)) {
		_bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_c_line(curr_line);
		std::string		ntoken, nltoken;

		std::getline(iss_c_line, ntoken, ' ');
		if (ntoken.compare("}") == 0) {
			_subkeywd_bracket = false; server_conf.routes.push_back(route_conf); return ;
		} else if (ntoken.compare("auto_index") == 0) {
			if (route_conf.auto_index == true) { std::cout << "throw error: multiple index definitions: line: " << _bad_line << std::endl; return ; }
			route_conf.auto_index = true;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; }
		} else if (ntoken.compare("index") == 0) {
			if (route_conf.index.compare("notgiven") != 0) { std::cout << "throw error: multiple index definitions: line: " << _bad_line << std::endl; return ; }
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '"' || ntoken[ntoken.length() - 1] != '"' || ntoken.length() < 3) { std::cout << "throw error: invalid index: line: " << _bad_line << std::endl; return ; }
			ntoken = ParserUtils::removeDelimiters(ntoken);
			route_conf.index = ntoken;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; }
		} else if (ntoken.compare("enable_cgi") == 0) {
			if (route_conf.cgi_enabled == true) { std::cout << "throw error: multiple enable_cgi definitions: line: " << _bad_line << std::endl; return ; }
			route_conf.cgi_enabled = true;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; }
		} else if (ntoken.compare("root") == 0) {
			if (route_conf.root.compare("nopath") != 0) { std::cout << "throw error: multiple root definitions: line: " << _bad_line << std::endl; return ; }
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '"' || ntoken[ntoken.length() - 1] != '"' || ntoken.length() < 3) { std::cout << "throw error: invalid root path: line: " << _bad_line << std::endl; return ; }
			ntoken = ParserUtils::removeDelimiters(ntoken);
			for (int i = 0; ntoken[i] != '\0'; i++) {
				if (ntoken[i] == '/' && ntoken[i + 1] == '/') { std::cout << "throw error: invalid root_path: line: " << _bad_line << std::endl; return ; }
			}
			route_conf.root = ntoken;
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; return ; }
		} else if (ntoken.compare("methods") == 0) {
			if (!(route_conf.accepted_methods.empty())) { std::cout << "thorw error: multiple methods definitions: line: " << _bad_line << std::endl; return ; }
			std::getline(iss_c_line, ntoken, ' ');
			if (ntoken[0] != '[' || ntoken[ntoken.length() - 1] != ']' || ntoken.length() < 5) { std::cout << "throw error: invalid methods: line: " << _bad_line << std::endl; return ; }
			getParams(ntoken, route_conf.accepted_methods);
			std::getline(iss_c_line, nltoken, ' ');
			if (!(nltoken.empty()) && nltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; return ; }
		} else if (ntoken.compare("redirect") == 0) {
			if (!(route_conf.is_redirect == true)) { std::cout << "thorw error: multiple redirection definitions: line: " << _bad_line << std::endl; return ; }
			std::getline(iss_c_line, ntoken, ' ');
			//add redirect
			
		}

	}
	if (_fd_conf.eof()) { std::cout << "throw error: missing closing bracket in route_config" << std::endl; return ; }	
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

		if (token.compare("{") != 0 && token[0] != '#') { std::cout << "throw error: missing opening bracket: line " << _bad_line << std::endl; return ; }
		else if (token.compare("{") == '{') _keywd_bracket = true;
	}

	while (std::getline(_fd_conf, curr_line)) {
		_bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		if (token.compare("port") == 0) parseServerPort(curr_line, server_conf);
		else if (token.compare("server_names") == 0) parseServerNames(curr_line, server_conf);
		else if (token.compare("error_pages") == 0) parseServerErrorPages(curr_line, server_conf);
		else if (token.compare("max_body_size") == 0) parseServerMaxBodySize(curr_line, server_conf);
		else if (token.compare("root") == 0) parseServerRoot(curr_line, server_conf);
		else if (token.compare("route") == 0) parseServerRoute(curr_line, server_conf);
		else if (token[0] == '#') continue ;
		else if (token.compare("}") == 0) { _keywd_bracket = false; _servers.push_back(server_conf); return ; }
		else { std::cout << "throw error: bad server parameter: line: " << _bad_line << std::endl; return ; }
	}

	if (_fd_conf.eof()) { std::cout << "throw error: missing closing bracket" << std::endl; return ; }
}

void	ServerConfig::parseMime() {
	std::string	curr_line;
	std::string	token, ltoken;

	while (!_keywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') { std::cout << "throw error: missing opening bracket: line " << _bad_line << std::endl; return ; }
		else if (token.compare("{") == 0) _keywd_bracket = true;
	}
	while (std::getline(_fd_conf, curr_line)) {
		_bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		
		std::vector<std::string>	file_extensions;
		std::vector<std::string>	points_to;
		if (token.compare("mime_add") == 0) {
			std::getline(iss_curr_line, token, ' ');
			getParams(token, file_extensions);
			std::getline(iss_curr_line, token, ' ');
			getParams(token, points_to);
			for (std::vector<std::string>::iterator it = file_extensions.begin(); it != file_extensions.end(); it++) {
				_mime.insert(std::make_pair(*it, points_to[0]));
			}
			std::getline(iss_curr_line, ltoken, ' ');
			if (!(ltoken.empty()) && ltoken[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; }
		}
		else if (token.compare("}") == 0) { _keywd_bracket = false; return ; }
		else if (token[0] == '#') continue ;
		else { std::cout << "throw error: bad server parameter: line: " << _bad_line << std::endl; return ; }
	}
	if (_fd_conf.eof()) { std::cout << "throw error: missing closing bracket" << std::endl; return ; }
}

/*	@ServerConfig constructor:
 * 		Calls getline until keyword is found
 * 		Sets _keywd_bracket true
 * 		Call respective block parser
 * 		Sets _keywd_bracket false
*/
ServerConfig::ServerConfig(const std::string& filepath) {
	RemoveTabs	noSpaces;

	if (filepath.empty()) {
		std::cout << "throw error: bad config file" << std::endl;
	}
	if (!(noSpaces.openFile(filepath))) {
		std::cout << "throw error: can't open file" << std::endl;
	}

	noSpaces.replace();
	noSpaces.writeToFile();

	std::string	dot_filepath = ".";
	dot_filepath += filepath;

	_fd_conf.open(dot_filepath.c_str());
	if (_fd_conf.is_open()) {
		std::string	curr_line;
		int		value;
		_bad_line = 0;
		while (std::getline(_fd_conf, curr_line)) {
			_bad_line++;
			if (curr_line.empty())
				continue ;
			value = isKeyword(curr_line);	
			if (value == 1) {
				parseServer();
			} else if (value == 2) {
				std::cout << "parse cgi" << std::endl;
			} else if (value == 3) {
				parseMime();
			} else if (value == 5) {
				std::cout << "throw error: bad_line[" << _bad_line << "]: " << curr_line << std::endl;
			}
			//exit(0);
		}
	
		if (_bad_line == 0) std::cout << "throw error: config file empty" << std::endl;
		_fd_conf.close();

	} else {
		std::cout << "throw error: can't open file" << std::endl;
	}

	std::cout << "\nServerConfig parsing done." << std::endl;
	std::cout << "\nServerConfig:" << std::endl;
	int	i = 1;
	for (std::vector<ServerCfg>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		std::cout << "\nServer[" << i++ << "]:" << std::endl;

		std::cout << "\tServer port:\n\t\t[" << (*it).port << "]" << std::endl;
		
		std::cout << "\tServer name(s): " << std::endl;
		for (std::vector<std::string>::iterator jit = (*it).server_names.begin(); jit != (*it).server_names.end(); jit++)
			std::cout << "\t\t[" << *jit << "]" << std::endl;

		std::cout << "\tServer error_pages: " << std::endl;
		for (std::map<short, std::string>::iterator jit = (*it).error_pages.begin(); jit != (*it).error_pages.end(); jit++)
			std::cout << "\t\t[" << jit->first << "] -> [" << jit->second << "]" << std::endl;

		std::cout << "\tServer max_body_size:\n\t\t[" << (*it).max_body_size << "]" << std::endl;
		
		std::cout << "\tServer root dir:\n\t\t[" << (*it).root_dir << "]" << std::endl;

		std::cout << "\tServer route(s):" << std::endl; int j = 1;
		for (std::vector<RouteCfg>::iterator jit = (*it).routes.begin(); jit != (*it).routes.end(); jit++) {
			std::cout << "\tRoute[" << j << "]" << std::endl;
			std::cout << "\t\tRoute path:\n\t\t\t[" << (*jit).route_path << "]" << std::endl;
		       	if ((*jit).is_redirect) std::cout << "\t\tRoute redirection:\n\t\t\t[" << (*jit).redirect_to << "]" << std::endl;
			std::cout << "\t\tRoute root:\n\t\t\t[" << (*jit).root << "]" << std::endl;
			std::cout << "\t\tRoute cgi enabled:\n\t\t\t[" << std::flush; 
			if ((*jit).cgi_enabled) std::cout << "yes]" << std::endl;
			else std::cout << "no]" << std::endl;
			if ((*jit).auto_index) std::cout << "\t\tRoute auto_index\n\t\t\t[yes]" << std::endl;
			else std::cout << "\t\tRoute index:\n\t\t\t[" << (*jit).index << "]" << std::endl;
			std::cout << "\t\tRoute accepted methods:" << std::endl;
			for (std::vector<std::string>::iterator vit = (*jit).accepted_methods.begin(); vit != (*jit).accepted_methods.end(); vit++) {
				std::cout << "\t\t\t[" << *vit << "]" << std::endl;
			}
		}

		std::cout << "\tServer mimes:" << std::endl;;
		for (std::map<std::string, std::string>::iterator	jit = _mime.begin(); jit != _mime.end(); jit++) {
			std::cout << "\t\t[" << jit->first << "] -> [" << jit->second << "]" << std::endl;
		}
	}
}


ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
