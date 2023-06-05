#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "ServerConfig.hpp"
#include "Server.hpp"
#include <vector>

class Router {
private:
    ServerConfig _cfg;
    int* _socket_fds;

    std::vector<Server> _servers;

    Router();
public:
    Router(ServerConfig& cfg);
    Router(const Router& copy);


    ~Router();

    Router& operator=(const Router& copy);

    // Will start listening to the ports defined in _cfg and route requests to appropriate servers
    void listen();
};

#endif