#ifndef __PARSERUTILS_HPP__
#define __PARSERUTILS_HPP__

# include <string>
# include <vector>

/* ParserUtils:
 * 	@atoi:
 * 		Only handles positive numbers
 *	@removeDelimiters:
 *		Removes 1st and last char from given string
*/


namespace ParserUtils {
	int	identifyKeyword(const std::string &line, bool &keywd_bracket);

	void	getParams(std::string &str, std::vector<std::string> &params, int &bad_line);

	std::string	removeArraySpaces(std::string &str);
	
	std::string	parseLine(std::string &rline, std::string s1, std::string s2);
	
	int	atoi(const std::string &str);

	std::string	removeDelimiters(std::string &str);

	int	countCharOccurs(char needle, const std::string &stack);

	std::string removeMultipleSpaces(const std::string& str);
	
	bool	isValidPath(const std::string &path);

	bool	isValidAuth(const std::string &auth);

	bool	isValidIp(const std::string &url);

	bool	isValidURL(const std::string &url);

	std::string	intToString(const int &number);

	bool	strAllSpaces(const std::string &line);
}
#endif
