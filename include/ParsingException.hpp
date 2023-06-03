#ifndef PARSING_EXCEPTION_HPP
#define PARSING_EXCEPTION_HPP

#include <exception>
#include <string>

class ParsingException : public std::exception
{
private:
    std::string msg;

public:
    ParsingException(const std::string& message);
    ParsingException(const ParsingException& copy);
    virtual ~ParsingException() throw();

    ParsingException& operator=(const ParsingException& copy) throw();

    const char* what() const throw();
};

#endif
