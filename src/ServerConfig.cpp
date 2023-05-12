#include "ServerConfig.hpp"
#include <string>

ServerConfig::ServerConfig(const std::string& file) { (void)file; }
ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
