#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

#include <stdexcept>
#include <map>
#include <string>
#include <vector>
#include <sstream>

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
