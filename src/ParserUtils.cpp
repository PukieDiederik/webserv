#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

# include <iostream>
# include <sstream>
# include <string>

namespace ParserUtils {
	std::string	removeSlashDups( std::string str) {
		std::string result = str;
		std::string::size_type pos = 0;	
		while ((pos = result.find("//", pos)) != std::string::npos)
			result.erase(pos, 1);
		return result;
	}

	/*	@identifyKeyword:
	*		Evaluates if given line contains a keyword and its respective '{' (or just the keyword)
	*		Returns error (5) for all other cases
	*/
	int	identifyKeyword(const std::string &line, bool &keywd_bracket) {
		int			keyword = ERROR;
		std::string		token, ntoken;
		std::istringstream	iss(line);

		std::getline(iss, token, ' ');
		if (token.compare("server") == 0) keyword = SERVER;
		else if (token.compare("cgi") == 0) keyword = CGI;
		else if (token.compare("mime") == 0) keyword = MIME;
		else if (token[0] == '#') return (COMMENT);

		std::getline(iss, ntoken, ' ');

		if (ntoken.compare(token) == 0) return (keyword);
		else if (ntoken[0] == '{') keywd_bracket = true;
		else if (!ntoken.empty() && ntoken[0] != '#') return (ERROR);

		return (keyword);
	}

	/*	@getParams:
	 *		Checks if string is delimited by brackets ('[]') and removes them
	 *		Checks if each param is separated by a comma and removes them
	 *		Checks if each param is delimited by quotes and removes them
	 *		Populates a vector<string> with each param
	*/
	void	getParams(std::string &str, std::vector<std::string> &params, int &bad_line) {
		std::string 		param;

		if (str[0] != '[' || str[str.length() - 1] != ']' || str.size() < 5) throw std::runtime_error("Error: bad array config: line: " + ParserUtils::intToString(bad_line));
		else {
			str = ParserUtils::removeDelimiters(str);
			//comma edge cases & remove spaces between commas and next token
			for (int i = 0; str[i] != '\0'; i++) {
				if (str[i] == ',' && (str[i - 1] != '"' || str[i + 1] != '"')) throw std::runtime_error("Error: invalid parameter config: line: " + ParserUtils::intToString(bad_line));
			}
		}

		std::istringstream 	iss(str);

		while (std::getline(iss, param, ',')) {
			if (!param.empty()) {
				if (param[0] != '\"' || param[param.length() - 1] != '\"' || param.size() < 3) {
					throw std::runtime_error("Error: bad parameter config: line: " + ParserUtils::intToString(bad_line));
					return ;
				} else 	param = ParserUtils::removeDelimiters(param);
				params.push_back(param);
			}
		}
	}

	/*	@atoi:
	 *		Simple implementation of atoi, returns -42 if error ou negative value
	 *		Stops if dot '.' is found
	 *
	 */
	int	atoi(const std::string &str) {
		int	value = 0;
		for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] == '.' && i > 0) return (value);
			if (str[i] < '0' || str[i] > '9') return (-42);
			else { value *= 10; value += (str[i] - 48); }
		}
		return (value);
	}

	/*	@removeDelimiters:
	 *		Remove 1st and last char in given str
	 *		Throws error if string_size < 3
	 * */
	std::string	removeDelimiters(std::string &str) {
		if (str.size() < 3) throw std::runtime_error("Error: (func)removeDelimiters");
		str = str.substr(1);
		str = str.substr(0, str.length() -1);

		return (str);
	}

	int	countCharOccurs(char needle, const std::string &stack) {
		int	count = 0;
		for (int i = 0; stack[i] != '\0'; i++)
			if (stack[i] == needle)
				count++;
		return (count);
	}

	std::string	removeMultipleSpaces(const std::string& str) {
		std::string result;
		bool previousSpace = false;

		for (std::size_t i = 0; i < str.length(); ++i) {
			if (str[i] != ' ') {
				result += str[i];
				previousSpace = false;
			} else {
				if (!previousSpace) {
					result += str[i];
					previousSpace = true;
				}
			}
		}

		if (!result.empty() && result[0] == ' ')
			result = result.substr(1);

		std::string::size_type	pos = result.find_last_not_of(' ');

		if ( pos != std::string::npos)
			result.erase(pos + 1);

		return result;
	}

	/*	@removeArraySpaces:
	 *		Removes spaces from string if outside quotes and inside brackets ([])
	 *
	 * */
	std::string	removeArraySpaces(std::string &str) {
		bool	insideQuotes = false;
		for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] == '[') {
				for (; str[i] != '\0' && str[i] != ']'; i++) {
					if (str[i] == '"' && insideQuotes) insideQuotes = false;
					else if (str[i] == '"') insideQuotes = true;
					else if ((str[i] == ' ' || str[i] == '	') && !(insideQuotes)) {
						int	j = i, size = 0;
						while (str[j] == ' ' || str[j] == '	') {
							j++;
							size++;
						}
						str.erase(i, size);
						i--;
					}
				}
			}
		}
		return (str);
	}

	std::string	parseLine(std::string &rline, std::string s1, std::string s2) {
		if (rline == "")
			return ("");
		size_t	s1Len = s1.length();
		size_t		pos = 0;
		while (pos < rline.length()) {
			if (rline.compare(pos, s1Len, s1) == 0) {
				rline.erase(pos, s1Len);
				rline.insert(pos, s2);
				pos += s2.length();
			}
			else
				pos++;
		}
		removeArraySpaces( rline );
		rline = removeMultipleSpaces( rline );
		return ( rline );
	}

	/*	@isValidPath:
	 *		Only valid chars: (a-z; A-Z; 0-9; '/'; '.'; '_'; '-')
	 *		Checking for doubble slashes "//" and doubble dots ".."
	 *
	 */
	bool	isValidPath(const std::string &path) {
		for (int i = 0; path[i] != '\0'; i++) {
			if ((path[i] < 97 || path[i] > 122) && (path[i] < 65 || path[i] > 90) && (path[i] < 48 || path[i] > 57)\
				       	&& path[i] != '/' && path[i] != '.' && path[i] != '_' && path[i] != '-')
				return (false);
			if (path[i] == '/' && path[i + 1] == '/')
				return (false);
			if (path[i] == ',' && path[i + 1] == '.')
				return (false);
		}
		return (true);
	}

	bool	isValidAuth(const std::string &auth) {
		if (ParserUtils::countCharOccurs('.', auth) < 1) return (false);
		std::string	extra = auth.substr(0, auth.find('.'));
		if (ParserUtils::countCharOccurs('.', auth) == 1 && extra.compare("www") == 0) return (false);
		for (int i = 0; auth[i] != '\0'; i++) {
			if (auth[i] == '/') break ;
			if ((auth[i] < 97 || auth[i] > 122) && (auth[i] < 48 || auth[i] > 57) && auth[i] != '.')
				return (false);
			if (auth[i] == '.' && auth[i + 1] == '.')
				return (false);
		}
		return (true);
	}

	bool	isValidIp(const std::string &ip) {
		int	value = -42;

		if (ip.length() < 7 || ParserUtils::countCharOccurs('.', ip) != 3) return (false);

		for (int i = 0; ip[i] != '\0'; i++) {
			if (ip[i] < '0' || ip[i] > '9') return (false);
			value = ParserUtils::atoi(ip.c_str() + i);
			if (value < 0 || value > 255) return (false);
			while (ip[i] != '.' && ip[i + 1] != '\0')
				i++;
			if (ip[i] == '.' && ip[i + 1] == '\0') return (false);
		}
		return (true);
	}

	bool	isValidURL(const std::string &url) {
		// Check if the URI starts with a valid scheme
		if (url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://") {
			if (url.length() <= 7 || url.find("://") != url.rfind(":"))
				return (false);

			// Check if the auth of the URL contains valid chars
			std::string auth = url.substr(url.find("://") + 3);
			if (!(isValidAuth(auth)) && !(isValidIp(auth)))
				return (false);
			// Check if the path of the URL contains valid chars
			if (auth.find("/") != std::string::npos) {
				std::string path = auth.substr(auth.find("/"));
				if (path.empty()) return (true);
				return (isValidPath(path));
			}
			return (true);
		}

		// Check if the path of the URL contains valid chars
		if (url.find("/") != std::string::npos && url[0] == '/') {
			std::string path = url.substr(url.find("/"));
			if (path.empty()) return (true);
			return (isValidPath(path));
		}

		return (false);
	}

	std::string	intToString(const int &number) {
		std::ostringstream oss;
		oss << number;
		return oss.str();
	}

	bool	strAllSpaces(const std::string &str) {
		int	c1 = 0, c2 = 0;
		for (int i = 0; str[i] != '\0'; i++) {
			c1++;
			if (str[i] == 32) c2++;
		}
		if (c1 == c2) return (true);
		return (false);
	}

	std::vector<std::string>	string_split( const std::string &str, char delimiter ) {
		std::vector<std::string>	result;
		std::stringstream		ss( str );
		std::string			token;

		while ( std::getline( ss, token, delimiter ) ) {
			if ( !( token.empty() ) )
				result.push_back( token );
		}

	    return	result;
	}

}

// Parsing utils from "ParsingUtils.hpp"
std::string trimSpace(std::string s)
{
    const std::string whitespaces (" \n\t\v\f\r");

    std::size_t start = s.find_first_not_of(whitespaces);
    if (start == std::string::npos)
        return std::string();
    std::size_t end = s.find_last_not_of(whitespaces);

    return s.substr(start, end + 1 - start);
}

// Check if string starts with substring
bool	startsWith(const std::string& str, const std::string& prefix)
{
    if (str.length() < prefix.length())
        return false;

    return !str.compare(0, prefix.length(), prefix);
}
