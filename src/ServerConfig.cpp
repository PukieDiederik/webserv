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


RouteCfg::RouteCfg() { }
RouteCfg::RouteCfg(const RouteCfg& copy) { (void)copy; }
RouteCfg::~RouteCfg() { }
RouteCfg RouteCfg::operator=(const RouteCfg& copy) { (void)copy; return *this; }

ServerCfg::ServerCfg() {
	this->port = -42;
	this->max_body_size = 0;
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
	else if (token[0] == '#') value = 4;

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



/*	@parseServer:
 * 		Checks if inital '{' was found, if not finds it or throws error
 * 		If one subkeyword (block) is found, calls for its parsing
 * 		Stops if '}' is found or EOF
*/

void	ServerConfig::parseServer(ServerCfg &server_conf) {
	std::string	curr_line;
	std::string	token, ntoken;

	_subkeywd_bracket = false;
	
	while (!_keywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token[0] != '{' && token[0] != '#') { std::cout << "throw error: missing opening bracket: line " << _bad_line << std::endl; return ; }
		else if (token[0] == '{') _keywd_bracket = true;
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
		else if (token.compare("root") == 0) std::cout << "set root" << std::endl;
		else if (token.compare("route") == 0) std::cout << "set route" << std::endl;
		else if (token[0] == '#') continue ;
		else if (token[0] == '}') { _keywd_bracket = false; return ; }
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
				ServerCfg	server_conf;
				parseServer(server_conf);
				std::cout << "\tServer port: [" << server_conf.port << "]" << std::endl;
				_servers.push_back(server_conf);
			} else if (value == 2) {
				std::cout << "parse cgi" << std::endl;
			} else if (value == 3) {
				std::cout << "parse mime" << std::endl;
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

		std::cout << "\tServer max_body_size:\n\t[" << (*it).max_body_size << "]" << std::endl;
	}
}


ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
