#include <string>
#include <cstddef>
#include "ParsingUtils.hpp"


std::string trimSpace(std::string s)
{
    const std::string whitespaces (" \n\t\v\f\r");

    std::size_t start = s.find_first_not_of(whitespaces);
    if (start == std::string::npos)
        return std::string();
    std::size_t end = s.find_last_not_of(whitespaces);

    return s.substr(start, end + 1 - start);
}