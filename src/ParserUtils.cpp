#include "ParserUtils.hpp"

int	ParserUtils::atoi(std::string str) {
	int	value = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] < '0' || str[i] > '9') return (-42);
		else { value *= 10; value += (str[i] - 48); }
	}
	return (value);
}

std::string	ParserUtils::removeDelimiters(std::string str) {
	str = str.substr(1);
	str = str.substr(0, str.length() -1);

	return (str);
}

int	ParserUtils::countCharOccurs(char needle, std::string stack) {
	int	count = 0;
	for (int i = 0; stack[i] != '\0'; i++)
		if (stack[i] == needle)
			count++;
	return (count);
}

std::string ParserUtils::removeMultipleSpaces(const std::string& str) {
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

std::string	ParserUtils::parseLine(std::string rline, std::string s1, std::string s2) {
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
	return (removeMultipleSpaces(rline));
}

bool	ParserUtils::isValidPath(std::string path) {
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

bool	ParserUtils::isValidAuth(std::string auth) {
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

bool	ParserUtils::isValidIp(std::string url) {
	if (url.length() < 7 || ParserUtils::countCharOccurs('.', url) != 3) return (false);

	std::string		token, ntoken;
	std::istringstream	iss_curr_line(url);

	for (int i = 0; i < 3; i++) {
		std::getline(iss_curr_line, token, '.');
		int	value = ParserUtils::atoi(token.c_str());
	      	if ( value < 0 || value > 255) return (false);
	}
	return (true);
}

bool	ParserUtils::isValidURL(std::string url) {
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

std::string ParserUtils::intToString(int number) {
	std::ostringstream oss;
	oss << number;
	return oss.str();
}

bool	ParserUtils::strAllSpaces(std::string str) {
	int	c1 = 0, c2 = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		c1++;
		if (str[i] == 32) c2++;
	}
	if (c1 == c2) return (true);
	return (false);
}
