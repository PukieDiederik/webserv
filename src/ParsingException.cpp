#include "ParsingException.hpp"

ParsingException::ParsingException(const std::string& message) :msg(message) { }
ParsingException::ParsingException(const ParsingException& copy) :msg(copy.msg) { }

ParsingException::~ParsingException() throw() { }

ParsingException& ParsingException::operator=(const ParsingException& copy) throw()
{ msg = copy.msg; return *this; }

const char* ParsingException::what() const throw() { return msg.c_str(); }
