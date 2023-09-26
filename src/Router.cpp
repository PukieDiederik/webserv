#include "Router.hpp"
#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RequestFactory.hpp"
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
#include <deque>
#include <sys/time.h>
#include <algorithm>
#include "Server.hpp"
#include <cerrno>

// Timeout in seconds
#define TIMEOUT_TIME 30

Router::Router()
{

    const ServerConfig& cfg = ServerConfig::getInstance();
    // Create servers
    // Loop over each server
    for (std::size_t i = 0; i < cfg._servers.size(); ++i)
    {
        // Create server objects
        _servers.push_back(Server(cfg._servers[i]));

        // Start listening to ports
        if (!_socket_fds.count(_servers.back().cfg().port)) { // If the port hasn't been opened yet, try to open it
            struct sockaddr_in sa;
            sa.sin_family = AF_INET;
            sa.sin_port = ntohs(_servers.back().cfg().port);
//        sa.sin_addr.s_addr = (127 << 24) | (0 << 16) | (0 << 8) | (1);

            if (!inet_pton(AF_INET, cfg._servers[i].host.c_str(), &sa.sin_addr.s_addr))
                throw std::runtime_error("Could not parse IP");

            if ((_socket_fds[_servers.back().cfg().port] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                throw std::runtime_error("Could not create socket");

            if (fcntl(_socket_fds[_servers.back().cfg().port], F_SETFL, O_NONBLOCK) < 0)
                throw std::runtime_error("Could not set socket fd to non blocking");

            if (bind(_socket_fds[_servers.back().cfg().port], (struct sockaddr *) &sa, sizeof(sa)) < 0)
                throw std::runtime_error("Could not open ports");

            std::cout << "Bound to port: " << _servers.back().cfg().port << std::endl;
        }
    }

    // Start listening to ports
}

Router::~Router()
{
    std::cout << "[Router Destructor]" << std::endl;
    for(std::size_t i = 0; i < ServerConfig::getInstance()._servers.size(); ++i)
    {
        close(_socket_fds[i]);
    }
}

// Helper functions
struct event
{
    struct epoll_event event;
    bool is_server;
    Server* server; // Server it is related to if not a serving socket itself
    int port; // The port it is open on, only needs to be set if this is a server
    struct timeval timeout_at; // Timeout for non-server sockets
    RequestFactory rf; // For generating the requests

    bool closing; // If this connection is closing. It will close the sockets once all data has been read
};

typedef std::deque<event*> timeouts_t;
typedef std::map<int, struct event> event_map_t;
typedef std::map<int, std::string> out_buffer_t;

struct event create_event(int fd, uint32_t events, bool is_server, int port, Server *server){
    struct event e;
    e.event.data.fd = fd;
    e.event.events = events;
    e.is_server = is_server;
    e.port = port;
    e.server = server;
    e.closing = false;

    return e;
}


// Creates or updates the timeout of an event
void update_timeout(timeouts_t& timeouts, event* e)
{
    // Set the timeout time
    gettimeofday(&e->timeout_at, NULL);
    e->timeout_at.tv_sec += TIMEOUT_TIME;

    // Erase timeout if it already exists
    timeouts_t::iterator i = std::find(timeouts.begin(), timeouts.end(), e);
    if (i != timeouts.end())
        timeouts.erase(i);

    // Push the updated event
    timeouts.push_back(e);
}

// Removes an event from the timeouts
void remove_timeout(timeouts_t& timeouts, event* e)
{
    timeouts_t::iterator i = std::find(timeouts.begin(), timeouts.end(), e);
    if (i != timeouts.end())
        timeouts.erase(i);
}

// Clears data associated with the given fd
void clear_fd(int fd, int epoll_fd,
              event_map_t& event_map,
              timeouts_t& timeouts,
              out_buffer_t& out_buffer)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event_map[fd].event);
    remove_timeout(timeouts, &event_map[fd]);
    event_map.erase(fd);
    out_buffer.erase(fd);
    close(fd);
}

Server* find_server_from_port(int port, std::vector<Server>& servers, HttpRequest& req)
{
    Server* s = NULL;
    std::string host = req.host();


    if (host.find(':'))
        host = host.substr(0, host.find(':')); // Remove port if provided

    for (std::size_t j = 0; j < servers.size(); ++j)
    {
        if (servers[j].cfg().port != port)
            continue;
        // If none has been found yet
        if (s == NULL) {
            s = &servers[j]; // Set as default
            if(std::find(servers[j].cfg().server_names.begin(),
                         servers[j].cfg().server_names.end(),
                         host) != servers[j].cfg().server_names.end())
                break; // If we already found the correct server, no need to search further
        }
        else if(std::find(servers[j].cfg().server_names.begin(),
                          servers[j].cfg().server_names.end(),
                          host) != servers[j].cfg().server_names.end()) {
            s = &servers[j];
            break;
        }
    }
    return s;
}

// Adds data to the output
void add_output(int epoll_fd, out_buffer_t& ob, struct epoll_event& event, const std::string& str)
{
    ob[event.data.fd] += str;

    event.events |= EPOLLOUT;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event.data.fd, &event);
}

void Router::listen()
{
    int epoll_fd;
    event_map_t event_map; // Map which stores all event information using fd as a key
    timeouts_t timeouts; // For managing timeouts of client sockets

    struct epoll_event events[MAX_EVENTS];
    out_buffer_t out_buffer; // Holds everything that needs to be written to a file descriptor
    char buffer[BUFFER_SIZE]; // Buffer for reading input

    if ((epoll_fd = epoll_create(1)) < 0)
        throw std::runtime_error("Could not create epoll");

    // Setup sockets with epoll
    for (_socket_fds_t::iterator i = _socket_fds.begin(); i != _socket_fds.end(); ++i)
    {
        // Create server event
        event_map[i->second] = create_event(i->second,
                                           EPOLLIN,
                                           true,
                                           i->first,
                                           NULL);
        // Add event to epoll
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, i->second,
                      &event_map[i->second].event))
            throw std::runtime_error("Could not add epoll_event to epoll");

        // Start listening for data
        ::listen(i->second, SOMAXCONN);
    }
    std::cout << "Got in created events" << std::endl;

    // Start listening in infinite loop
    while (true)
    {
        int next_timeout = -1;
        // If there are timeouts calculate the time until the next timeout (in ms)
        if (!timeouts.empty()) {
            struct timeval t;
            gettimeofday(&t, NULL);

            next_timeout = (int)((timeouts.front()->timeout_at.tv_sec - t.tv_sec) * 1000 +
                                 (timeouts.front()->timeout_at.tv_usec - t.tv_usec) / 1000);

            // Makes sure that we don't get some weird timing where it waits forever when it shouldn't
            if (next_timeout < 0)
                next_timeout = 0;
        }

        // Wait for events
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, next_timeout);
        if (num_events < 0)
            throw std::runtime_error("Error while waiting for events");

        // Go through each event
        for (int i = 0; i < num_events; ++i)
        {
            event_map_t::mapped_type& em = event_map[events[i].data.fd];
            if (events[i].events & EPOLLIN) {
                // If it is a server socket we should create a client socket
                if (em.is_server) {
                    std::cout << "New connection found" << std::endl;

                    // Create client socket
                    struct sockaddr_in client_sockaddr;
                    socklen_t client_sockaddr_len = sizeof(client_sockaddr);
                    std::memset(&client_sockaddr, 0, sizeof(client_sockaddr));

                    int client_socket = accept(events[i].data.fd,
                                               (struct sockaddr*)(&client_sockaddr),
                                               &client_sockaddr_len);

                    if (fcntl(client_socket, F_SETFL, O_NONBLOCK) < 0)
                        throw std::runtime_error("Could not set socket fd to non blocking");

                    // Create event for client socket
                    struct event e = create_event(client_socket,
                                                  EPOLLIN | EPOLLRDHUP | EPOLLHUP,
                                                  false,
                                                  em.port,
                                                  NULL);

                    event_map[client_socket] = e; // Add it to the list of events
                    update_timeout(timeouts, &event_map[client_socket]); // Add it to list of timeouts

                    // Add client socket to epoll
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e.event.data.fd, &event_map[e.event.data.fd].event);
                    std::cout << "Created client socket with fd: " << client_socket << std::endl;
                }
                // If it's a client socket we should process the request
                else if (!em.closing) {

                    // Start reading data
                    int bytes_read = 1;
                    while (bytes_read > 0) {
                        // Start reading data from the socket
                        bytes_read = recv(events[i].data.fd, buffer, sizeof(buffer), 0);
                        if (bytes_read <= 0)
                            break;
                        std::string s(buffer, bytes_read);
                        try
                        {
                            // Try inputting read data into the request factory
                            em.rf.in(s);
                        }
                        catch (const std::exception& e)
                        {
                            // Generate error 400 response
                            HttpResponse res;
                            res.set_status(400, "Bad request");
                            res.headers("Connection", "Close");

                            // If we don't have a server found for this connection use default settings
                            if (em.server)
                                res = response_error(res, &em.server->cfg(), 400);
                            else
                                res = response_error(res, NULL, 400);

                            // Adds 400 response to the output buffer
                            add_output(epoll_fd, out_buffer, em.event, res.toString());
                            // Will close the connection on a bad request
                            em.closing = true;
                            break;
                        }
                    }
                    std::cout << "handling new data on: " << events[i].data.fd << std::endl;


                    if (!em.closing && em.rf.isReqReady())
                    {
                        HttpResponse res;
                        HttpRequest req = em.rf.getRequest();
                        if (req.headers().count("Connection") && req.headers("Connection") == "close")
                            em.closing = true;

                        std::cout << "Made request" << std::endl;
                        // Figure out which server this request belongs to
                        if (em.server == NULL) {
                            em.server = find_server_from_port(em.port, _servers, req);
                        }
                        res = em.server->handleRequest(req);

                        // Add the server header
                        res.headers("Server", "42-webserv");

                        // Add the server to out_buffer to later be sent
                        add_output(epoll_fd, out_buffer, em.event, res.toString());
                    }
                    update_timeout(timeouts, &em);
                }
            }
            // If the socket is ready for write operations
            if (events[i].events & EPOLLOUT) {
                std::map<int, std::string>::iterator out_it;

                // Make sure there is stuff in the out_buffer for this file descriptor
                out_it = out_buffer.find(events[i].data.fd);
                if (out_it == out_buffer.end())
                    continue;

                int bytes_send = 0;
                // start writing on the socket
                for (std::size_t s = 0; s < out_it->second.length(); s += bytes_send)
                {
                    bytes_send = send(events[i].data.fd,
                                      out_it->second.c_str() + s,
                                      out_it->second.length() - s,
                                      MSG_DONTWAIT);
                    if (bytes_send <= 0)
                        break; // Something errored
                }
                if (bytes_send <= 0)
                {
                    std::cout << "Error on writing, closing fd" << std::endl;
                    clear_fd(events[i].data.fd, epoll_fd, event_map, timeouts, out_buffer);
                }


                // Remove the out_buffer for this socket
                out_buffer.erase(out_it);

                // Update epoll
                em.event.events &= ~EPOLLOUT;
                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &em.event);

                if (em.closing)
                {
                    std::cout << "Closing connection: " << em.event.data.fd << std::endl;
                    clear_fd(events[i].data.fd, epoll_fd, event_map, timeouts, out_buffer);
                }
                else
                    update_timeout(timeouts, &em);
            }
            // On socket closure
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                std::cout << "Socket closed: " << events[i].data.fd << std::endl;
                // Clear up socket data
                clear_fd(events[i].data.fd, epoll_fd, event_map, timeouts, out_buffer);
            }
        }

        // Remove timed-out fds
        struct timeval t;
        gettimeofday(&t, NULL);
        while(!timeouts.empty() && timeouts.front()->timeout_at.tv_sec <= t.tv_sec
                                && timeouts.front()->timeout_at.tv_usec <= t.tv_usec)
        {
            // If it was already closing and it has not closed the socket yet, it will force close the socket
            if (timeouts.front()->closing)
            {
                std::cout << "File descriptor (" << timeouts.front()->event.data.fd
                          << ") timed out, Cleaning up." << std::endl;
                clear_fd(timeouts.front()->event.data.fd, epoll_fd, event_map, timeouts, out_buffer);
            }
            else
            {
                std::cout << "File descriptor (" << timeouts.front()->event.data.fd
                          << ") timed out, sending 408." << std::endl;

                HttpResponse res;
                res.set_status(408, "Request Timeout");
                res.headers("Connection", "close");

                if (timeouts.front()->server)
                    response_error(res, &timeouts.front()->server->cfg(), 408);
                else
                    response_error(res, NULL, 408);

                add_output(epoll_fd, out_buffer, timeouts.front()->event, res.toString());
                timeouts.front()->closing = true;
                update_timeout(timeouts, timeouts.front());
            }
        }
    }
    close(epoll_fd);
}
