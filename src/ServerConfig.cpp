#include "ServerConfig.hpp"
#include <string>


RouteCfg::RouteCfg() { }
RouteCfg::RouteCfg(const RouteCfg& copy) { (void)copy; }
RouteCfg::~RouteCfg() { }
RouteCfg RouteCfg::operator=(const RouteCfg& copy) { (void)copy; return *this; }

ServerCfg::ServerCfg() { }
ServerCfg::ServerCfg(const ServerCfg& copy) { (void)copy; }
ServerCfg::~ServerCfg() { }
ServerCfg ServerCfg::operator=(const ServerCfg& copy) { (void)copy; return *this; }


// ServerConfig
ServerConfig::ServerConfig(const std::string& file) { (void)file; }
ServerConfig::ServerConfig(const ServerConfig& copy) { (void)copy; }

ServerConfig::~ServerConfig() { }

ServerConfig& ServerConfig::operator=(const ServerConfig& copy) { (void)copy; return *this; }
