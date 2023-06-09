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
