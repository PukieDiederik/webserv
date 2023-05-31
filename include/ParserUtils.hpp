#ifndef __PARSERUTILS_HPP__
#define __PARSERUTILS_HPP__

/* ParserUtils:
 * 	@atoi:	only handles positive numbers;
*/
namespace ParserUtils {
	int	atoi(std::string str) {
		int	value = 0;
		for (int i = 0; str[i] != '\0'; i++) {
			if (str[i] < '0' || str[i] > '9') return (-42);
			else { value *= 10; value += (str[i] - 48); }
		}
		return (value);
	}
}

#endif
