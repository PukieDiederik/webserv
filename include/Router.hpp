#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "ServerConfig.hpp"
#include "Server.hpp"
#include <vector>

#define MAX_EVENTS 100
#define BUFFER_SIZE 4096

class Router {
private:
    ServerConfig _cfg;
    std::map<int, int> _socket_fds; // Key = port, Value = socket fd
//    int* _socket_fds;

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
