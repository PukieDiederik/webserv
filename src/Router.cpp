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
#include <queue>
#include <sys/time.h>

// Timeout in seconds
#define TIMEOUT_TIME 15

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
    // Should only be set if not a server
    struct timeval timeout_at;
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
    std::map<int, std::string> out_buffer; // Holds everything that needs to be written to a file descriptor
    char buffer[BUFFER_SIZE];
    std::queue<event*> timeouts;

    std::cout << "CPS: " << CLOCKS_PER_SEC << std::endl;

    while (true)
    {
        int next_timeout = -1;
        if (!timeouts.empty()) {
            struct timeval t;
            gettimeofday(&t, NULL);
            next_timeout = (timeouts.front()->timeout_at.tv_sec - t.tv_sec) * 1000 +
                           (timeouts.front()->timeout_at.tv_usec - t.tv_usec) / 1000;
        }
        std::cout << "next_timeout: " << next_timeout << std::endl;
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, next_timeout);
        if (num_events < 0)
            throw std::runtime_error("Error while waiting for events");

        for (int i = 0; i < num_events; ++i)
        {
            if (events[i].events & EPOLLIN)
            {
                std::cout << "handling new request on: " << events[i].data.fd << std::endl;
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

                if (!req_sstream.str().empty())
                {
                    HttpResponse res;

                    try {
                        HttpRequest req(req_sstream.str());
                        Server* serv = event_map[events[i].data.fd].server;
                        res = serv->handleRequest(req);
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << "Could not parse request\n";
                        res.set_status(400, "Bad request");
                        res.set_header("Server", "42-webserv");
                    }

                    std::string res_s = res.toString();
                    out_buffer[client_socket] += res_s;

                    struct event e;
                    e.event.data.fd = client_socket;
                    e.event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLHUP;
                    e.is_server = false;
                    e.server = event_map[events[i].data.fd].server;
                    e.related_server_fd = events[i].data.fd;
                    gettimeofday(&e.timeout_at, NULL);
                    e.timeout_at.tv_sec += TIMEOUT_TIME;

                    event_map[client_socket] = e;
                    timeouts.push(&event_map[client_socket]);

                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e.event.data.fd, &event_map[client_socket].event);
                }
            }
            if (events[i].events & EPOLLOUT)
            {
                std::map<int, std::string>::iterator out_it;

                out_it = out_buffer.find(events[i].data.fd);
                if (out_it == out_buffer.end())
                    continue;

                int bytes_send = 0;
                // start writing
                for (std::size_t s = 0; s < out_it->second.length(); s += bytes_send)
                {
                    bytes_send = send(events[i].data.fd, out_it->second.c_str() + i, out_it->second.length() - i, MSG_DONTWAIT);
                    if (bytes_send <= 0)
                        break; // Something errored
                }
                out_buffer.erase(out_it);

                // Update epoll
                event_map[events[i].data.fd].event.events &= ~EPOLLOUT; //TODO: fix this
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &event_map[events[i].data.fd].event);
            }
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP))
            {
                std::cout << "Socket closed, cleaning up" << std::endl;
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &event_map[events[i].data.fd].event);
                event_map.erase(events[i].data.fd);
                out_buffer.erase(events[i].data.fd);
            }
            if (events[i].events & EPOLLHUP)
            {
                std::cout << "Found HUP" << std::endl;
            }
        }

        struct timeval t;
        gettimeofday(&t, NULL);
        // Remove timed out fds
        while(!timeouts.empty() && timeouts.front()->timeout_at.tv_sec <= t.tv_sec && timeouts.front()->timeout_at.tv_usec <= t.tv_usec)
        {
            std::cout << "File descriptor (" << timeouts.front()->event.data.fd << ") timed out, Cleaning up." << std::endl;
            if (!event_map.count(timeouts.front()->event.data.fd))
            {
                std::cout << "File descriptor (" << timeouts.front()->event.data.fd
                          << ") has already been cleaned up, skipping" << std::endl;
                timeouts.pop();
                continue;
            }
            std::cout << "File descriptor (" << timeouts.front()->event.data.fd << ") timed out, Cleaning up." << std::endl;
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, timeouts.front()->event.data.fd, &timeouts.front()->event);
            event_map.erase(timeouts.front()->event.data.fd);
            out_buffer.erase(timeouts.front()->event.data.fd);
            timeouts.pop();
        }
    }
    close(epoll_fd);
}
