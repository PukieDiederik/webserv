#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#define BUFFER_SIZE 4096


class Server {
private:
    ServerCfg _cfg;
    ServerConfig _gen_cfg;

    Server();
public:
    // Will take a server config to set up this server, and a general config for stuff like CGI and mime types
    Server(ServerCfg& cfg, ServerConfig& gen_cfg);
    Server(const Server& copy);

    ~Server();

    Server& operator=(const Server& copy);

    // Will take a request and handle it, which includes calling cgi
    HttpResponse handleRequest(const HttpRequest& req);

    ServerCfg& cfg();
};

#endif
