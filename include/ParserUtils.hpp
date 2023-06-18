#ifndef __PARSERUTILS_HPP__
#define __PARSERUTILS_HPP__

# include <string>

/* ParserUtils:
 * 	@atoi:
 * 		Only handles positive numbers
 *	@removeDelimiters:
 *		Removes 1st and last char from given string
*/


namespace ParserUtils {
	std::string removeMultipleSpaces(const std::string& str);

	std::string	parseLine(std::string rline, std::string s1, std::string s2);
	
	int	atoi(std::string str);

	std::string	removeDelimiters(std::string str);

	int	countCharOccurs(char needle, std::string stack);
	
	bool	isValidPath(std::string path);

	bool	isValidAuth(std::string auth);

	bool	isValidIp(std::string url);

	bool	isValidURL(std::string url);

	std::string	intToString(int number);

	bool	strAllSpaces(std::string line);
}
#endif
