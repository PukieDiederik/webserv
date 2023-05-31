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

ServerCfg::ServerCfg() { }
ServerCfg::ServerCfg(const ServerCfg& copy) { (void)copy; }
ServerCfg::~ServerCfg() { }
ServerCfg ServerCfg::operator=(const ServerCfg& copy) { (void)copy; return *this; }

/* evaluates if given line contains a keyword and its respective '{' (or just the keyword); 
 * returns error (5) for all other cases
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

/*	@parseServerPort:
 *		Checks if port is already set, throws error if resetting
 *		Atoi's given port, throws error is bad input
 *		Checks for addiciontal token in string, throws erros if not comments
 * */
void	ServerConfig::parseServerPort(std::string curr_line, ServerCfg server_conf) {
	if (server_conf.port > 0) { std::cout << "throw error: multiple port config: line: " << _bad_line << std::endl; return ; }

	std::string	token;
	std::istringstream	iss_curr_line(curr_line);

	std::getline(iss_curr_line, token, ' ');
	std::getline(iss_curr_line, token, ' ');

	if ((server_conf.port = ParserUtils::atoi(token.c_str())) < 0) { std::cout << "throw error: bad port config: line: " << _bad_line << std::endl; return ; }
	else if (server_conf.port == 0) server_conf.port = 42; // assign available ports !!

	std::getline(iss_curr_line, token, ' ');
	if (token.empty() && token[0] != '#') { std::cout << "throw error: unexpected token: line: " << _bad_line << std::endl; } 
}

/* parseServer:
 * 	Checks if inital '{' was found, if not finds it or throws error
 * 	If one subkeyword (block) is found, calls for its parsing
 * 	Stops if '}' is found or EOF
*/
void	ServerConfig::parseServer() {
	std::string	curr_line;
	std::string	token, ntoken;
	ServerCfg	server_conf; // this object constructor can be used to set all values to deafult (to then compare with expected values)

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
		else if (token.compare("server_names") == 0) std::cout << "set server names" << std::endl;
		else if (token.compare("error_page") == 0) std::cout << "set error page" << std::endl;
		else if (token.compare("max_body_size") == 0) std::cout << "set max_body_size" << std::endl;
		else if (token.compare("root") == 0) std::cout << "set root" << std::endl;
		else if (token.compare("route") == 0) std::cout << "set route" << std::endl;
		else if (token[0] == '#') continue ;
		else if (token[0] == '}') { _keywd_bracket = false; return ; }
		else { std::cout << "throw error: bad server parameter: line: " << _bad_line << std::endl; return ; }
	}

	if (_fd_conf.eof()) { std::cout << "throw error: missing closing bracket" << std::endl; return ; }

	_servers.push_back(server_conf);
}

/* ServerConfig constructor:
 * 	Calls getline until keyword is found
 * 	Sets _keywd_bracket true
 * 	Call respective block parser
 * 	Sets _keywd_bracket false
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
