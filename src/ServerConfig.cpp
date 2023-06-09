#include "ServerConfig.hpp"
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
	this->host = "notdefined";
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

void	ServerConfig::parseMime() {
	std::string	curr_line;
	std::string	token, ltoken;

	while (!_keywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token.compare("{") != 0 && token[0] != '#') { std::cout << "throw error: missing opening bracket: line " << _bad_line << std::endl; return ; }
		else if (token.compare("{") == 0) _keywd_bracket = true;
	}
	while (std::getline(_fd_conf, curr_line)) {
		curr_line = ParserUtils::parseLine(curr_line, "	", " ");
		_bad_line++;
		if (curr_line.empty()) continue ;

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		
		std::vector<std::string>	file_extensions;
		if (token.compare("mime_add") == 0) {
			std::getline(iss_curr_line, token, ' ');
			getParams(token, file_extensions);
			std::getline(iss_curr_line, token, ' ');
			if (token[0] != '"' || token[token.length() - 1] != '"' || token.length() < 3) { std::cout << "throw error: bad mime config: line: " << _bad_line << std::endl; }
			for (std::vector<std::string>::iterator it = file_extensions.begin(); it != file_extensions.end(); it++) {
				_mime.insert(std::make_pair(*it, ParserUtils::removeDelimiters(token)));
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
	if (filepath.empty()) {
		std::cout << "throw error: bad config file" << std::endl;
	}

	_fd_conf.open(filepath.c_str());
	if (_fd_conf.is_open()) {
		std::string	curr_line;
		int		value;
		_bad_line = 0;
		while (std::getline(_fd_conf, curr_line)) {
			curr_line = ParserUtils::parseLine(curr_line, "	", " ");
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
		
		std::cout << "\tServer host:\n\t\t[" << (*it).host << "]" << std::endl;

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
			std::cout << "\tRoute[" << j++ << "]" << std::endl;
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
