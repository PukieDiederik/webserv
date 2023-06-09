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
	
	int	atoi(std::string str); /*{
		int	value = 0;
		for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] < '0' || str[i] > '9') return (-42);
			else { value *= 10; value += (str[i] - 48); }
		}
		return (value);
	}
*/
	std::string	removeDelimiters(std::string str); /*{
		str = str.substr(1);
		str = str.substr(0, str.length() -1);

		return (str);
	}
*/
	int	countCharOccurs(char needle, std::string stack); /*{
		int	count = 0;
		for (int i = 0; stack[i] != '\0'; i++)
			if (stack[i] == needle)
				count++;
		return (count);
	}*/
}
#endif
