#include "ServerConfig.hpp"
#include "RemoveTabs.hpp"

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

ServerCfg::ServerCfg() { }
ServerCfg::ServerCfg(const ServerCfg& copy) { (void)copy; }
ServerCfg::~ServerCfg() { }
ServerCfg ServerCfg::operator=(const ServerCfg& copy) { (void)copy; return *this; }

// evaluates if given line contains a keyword and its respective '{' (or just the keyword); 
// returns error (5) for all other cases
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

void	ServerConfig::parseServer() {
	std::string	curr_line;
	std::string	token, ntoken;

	_subkeywd_bracket = false;
	
	// check if initial '{' was found, if not find it or return error
	while (!_keywd_bracket) {
		std::getline(_fd_conf, curr_line); _bad_line++;
		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');

		if (token[0] != '{' && token[0] != '#') { std::cout << "throw error: missing opening bracket: line " << _bad_line << std::endl; return ; }
		else if (token[0] == '{') _keywd_bracket = true;
	}

	// read next sub blocks
	while (std::getline(_fd_conf, curr_line)) {
		_bad_line++;
		if (curr_line.empty()) continue ;

		// same process as before: which subkeyword (block) it is
		// then accordingly parse said block
		// '}' after subkeywd still to handle

		std::istringstream	iss_curr_line(curr_line);
		std::getline(iss_curr_line, token, ' ');
		if (token.compare("port") == 0) std::cout << "set port" << std::endl;
		else if (token.compare("server_names") == 0) std::cout << "set server names" << std::endl;
		else if (token.compare("error_page") == 0) std::cout << "set error page" << std::endl;
		else if (token.compare("max_body_size") == 0) std::cout << "set max_body_size" << std::endl;
		else if (token.compare("root") == 0) std::cout << "set root" << std::endl;
		else if (token.compare("route") == 0) std::cout << "set route" << std::endl;
		else if (token[0] == '#') continue ;
		else if (token[0] == '}') return ;
		else { std::cout << "throw error: bad server parameter: line: " << _bad_line << std::endl; return ; }
	}

	if (_fd_conf.eof()) { std::cout << "throw error: missing closing bracket" << std::endl; return ; }
}

/* ServerConfig constructor:
 * 	getline until keyword is found
 * 	set _keywd_bracket true
 * 	call respective block parser
 * 	set _keywd_bracket false
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
			switch (value) {
				case 1:
					parseServer();
					break ;
				case 2:
					std::cout << "parse cgi" << std::endl;
					break ;
				case 3:
					std::cout << "parse mime" << std::endl;
					break ;
				case 4:
					std::cout << "just comment" << std::endl;
					break ;
				case 5:
					std::cout << "throw error: bad_line[" << _bad_line << "]: " << curr_line << std::endl;
					break ;
			}
			//exit(0);
		}
	
		if (_bad_line == 0) std::cout << "throw error: config file empty" << std::endl;
		_fd_conf.close();

	} else {
		std::cout << "throw error: can't open file" << std::endl;
	}

	std::cout << "ServerConfig parsing done." << std::endl;
}


ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
