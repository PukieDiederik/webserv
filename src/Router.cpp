#include "Router.hpp"
#include "ServerConfig.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

Router::Router(ServerConfig& cfg) : _cfg(cfg), _socket_fds(NULL)
{
    // TODO: This should be removed, this is just for testing purposes REJECT PR IF THIS IS PRESENT
    ServerCfg s1;
    ServerCfg s2;
    s1.port = 9991;
    s2.port = 9992;
    s1.server_names.push_back("www.test.com");
    s1.server_names.push_back("sub.website.nl");
    s2.server_names.push_back("www.test.com");
    s2.server_names.push_back("sub.website.nl");
    s1.root_dir = "/data/programming/webserv";
    s2.root_dir = "/data/programming/webserv";

    RouteCfg r1;
    RouteCfg r2;

    r1.name = "/";
    r1.root = s1.root_dir + "/include";
    r2.name = "/";
    r2.root = s1.root_dir + "/src";

    r1.auto_index = true;
    r2.auto_index = true;

    s1.routes.push_back(r1);
    s2.routes.push_back(r2);

    ServerConfig c("test");
    c.servers.push_back(s1);
    c.servers.push_back(s2);

    _cfg = c;

    // Create servers
    _socket_fds = new int[_cfg.servers.size()];
    std::memset(_socket_fds, -1, sizeof(int) * _cfg.servers.size());

    std::cout << _cfg.servers.size() << c.servers.size() << std::endl;
    std::cout << c.servers[1].port << " " << s2.port << std::endl;
    std::cout << _cfg.servers[1].port << std::endl;

    // Loop over each server
    for (std::size_t i = 0; i < _cfg.servers.size(); ++i)
    {
        // Create server objects
        _servers.push_back(Server(_cfg.servers[i], _cfg));

        // Start listening to ports
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = ntohs(_servers.back().cfg().port);
        inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr.s_addr); //TODO: error check this
//        sa.sin_addr.s_addr = INADDR_ANY;

        _socket_fds[i] = socket(AF_INET, SOCK_STREAM, 0);

        if (bind(_socket_fds[i], (struct sockaddr *)&sa, sizeof(sa)) < 0)
            std::cerr << "could not bind\n";
#if DEBUG==1
        std::cout << "Started listening on port: " << _servers.back().cfg().port << std::endl;
#endif
    }

    // Start listening to ports
}

Router::Router(const Router& copy) :_cfg(copy._cfg) { }

Router::~Router()
{
    if (_socket_fds)
    {
        for(std::size_t i = 0; i < _cfg.servers.size(); ++i)
        {
            if (_socket_fds[i] >= 0)
            {
#if DEBUG==1
                std::cout << "Closing fd: " << _socket_fds[i] << std::endl;
#endif
                close(_socket_fds[i]);
            }
        }
        delete[] _socket_fds;
    }
}

Router& Router::operator=(const Router& copy)
{
    _cfg = copy._cfg;
    return *this;
}

void Router::listen()
{
    ::listen(_socket_fds[0], SOMAXCONN);

    while (true)
    {
        struct sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(_socket_fds[0], (struct sockaddr*)&clientAddress, &clientAddressLength);

        char buffer[100];
        recv(clientSocket, buffer, sizeof(buffer),0);
        write(0, buffer, 100);
    }
    // TODO: start listening to ports
}
