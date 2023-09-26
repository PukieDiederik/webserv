#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#define BUFFER_SIZE 4096

class Server {
    private:
        const ServerCfg&   _cfg;

        Server();
        Server& operator=(const Server& copy);

    public:
        // Will take a server config to set up this server, and a general config for stuff like CGI and mime types
        Server(const ServerCfg& cfg);
        Server(const Server& copy);

        ~Server();


        // Will take a request and handle it, which includes calling CGI
        HttpResponse    handleRequest( const HttpRequest& req);

        const ServerCfg&  cfg();
};

HttpResponse& response_error(HttpResponse& res, const ServerCfg* _cfg, int statusCode);

#endif
