#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

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
