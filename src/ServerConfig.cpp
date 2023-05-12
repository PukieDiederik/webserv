#include "ServerConfig.hpp"
#include <string>


RouteCfg::RouteCfg() { }
RouteCfg::RouteCfg(const RouteCfg& copy) { (void)copy; }
RouteCfg::~RouteCfg() { }

ServerCfg::ServerCfg() { }
ServerCfg::ServerCfg(const ServerCfg& copy) { (void)copy; }
ServerCfg::~ServerCfg() { }

// ServerConfig
ServerConfig::ServerConfig(const std::string& file) { (void)file; }
ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
