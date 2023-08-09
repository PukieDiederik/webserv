#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "ServerConfig.hpp"
#include "Server.hpp"
#include <vector>

#define MAX_EVENTS 100
#define BUFFER_SIZE 4096

class Router {
private:
    typedef std::map<int, int> _socket_fds_t;

    std::map<int, int> _socket_fds; // Key = port, Value = socket fd

    std::vector<Server> _servers;

    Router(const Router& copy);
    Router& operator=(const Router& copy);
public:
    Router();

    ~Router();

    // Will start listening to the ports defined in _cfg and route requests to appropriate servers
    void listen();
};

#endif
