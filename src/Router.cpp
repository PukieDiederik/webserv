#include "Router.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
#include <fcntl.h>
#include <sstream>

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

    // Loop over each server
    for (std::size_t i = 0; i < _cfg.servers.size(); ++i)
    {
        // Create server objects
        _servers.push_back(Server(_cfg.servers[i], _cfg));

        // Start listening to ports
        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = ntohs(_servers.back().cfg().port);
//        sa.sin_addr.s_addr = (127 << 24) | (0 << 16) | (0 << 8) | (1);

        if(!inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr.s_addr)) //TODO replace this with host once implemented
            throw std::runtime_error("Could not parse IP");

        if ((_socket_fds[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw std::runtime_error("Could not create socket");

        if (fcntl(_socket_fds[i], F_SETFL, O_NONBLOCK) < 0)
            throw std::runtime_error("Could not set socket fd to non blocking");

        if (bind(_socket_fds[i], (struct sockaddr *)&sa, sizeof(sa)) < 0)
            throw std::runtime_error("Could not open ports");

#if DEBUG==1
        std::cout << "Bound to port: " << _servers.back().cfg().port << std::endl;
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

struct event
{
    struct epoll_event event;
    bool is_server;
    Server* server;
    int related_server_fd;
};

void Router::listen()
{
    int epoll_fd = epoll_create(1); // Size is ignored
    // Map which stores all epoll events using fd as a file descriptor
    std::map<int, struct event> event_map;

    if (epoll_fd < 0)
        throw std::runtime_error("Could not create epoll");


    for (std::size_t i = 0; i < _servers.size(); ++i)
    {
        // Add socket to epoll
        struct event e;
        e.event.data.fd = _socket_fds[i];
        e.event.events = EPOLLIN;
        e.is_server = true;
        e.server = &_servers[i];
        e.related_server_fd = _socket_fds[i];
        event_map[_socket_fds[i]] = e;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e.event.data.fd, &event_map[_socket_fds[i]].event))
            throw std::runtime_error("Could not add epoll_event to epoll");

        // Start listening for data
        ::listen(_socket_fds[i], SOMAXCONN);
    }

    struct epoll_event events[MAX_EVENTS];
    char buffer[BUFFER_SIZE];

    while (true)
    {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events < 0)
            throw std::runtime_error("Error while waiting for events");

        for (int i = 0; i < num_events; ++i)
        {
            std::cout << "handling new request" << std::endl;
            struct sockaddr_in client_sockaddr;
            socklen_t client_sockaddr_len = sizeof(client_sockaddr);
            std::memset(&client_sockaddr, 0, sizeof(client_sockaddr));

            int client_socket = accept(events[i].data.fd, (struct sockaddr*)(&client_sockaddr), &client_sockaddr_len);
            int bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
            std::ostringstream req_sstream;
            req_sstream.write(buffer, bytes_read);

            while (bytes_read > 0)
            {
                bytes_read = recv(client_sockaddr_len, &buffer, sizeof(buffer), 0);
                req_sstream.write(buffer, bytes_read);
            }

            std::cout << "origin: " << req_sstream.str() << std::endl;
            HttpResponse res;

            try {
                HttpRequest req(req_sstream.str());
                Server* serv = event_map[events[i].data.fd].server;
                res = serv->handleRequest(req);
            }
            catch (std::exception e)
            {
                std::cerr << "Could not parse request\n";
                res.set_status(400, "Bad request");
                res.set_header("Server", "42-webserv");
            }

            std::string res_s = res.toString();
            int bytes_send = 0;

            std::cout << "res: \n" << res_s;

            for (std::size_t i = 0; i < res_s.length(); i += bytes_send)
            {
                bytes_send = send(client_socket, res_s.c_str() + i, res_s.length() - i, MSG_DONTWAIT);
                if (bytes_send <= 0)
                    break; // Something errored
            }
            close (client_socket);
        }
    }
    close(epoll_fd);
}
