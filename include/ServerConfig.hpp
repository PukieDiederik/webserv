#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

// Has config about routes
struct RouteCfg {
    std::string name;

    // Redirects
    bool is_redirect;
    std::string redirect_to;

    // This will be an absolute path
    std::string root;

    bool cgi_enabled;

    bool auto_index;
    // This should raise an error if not given and autoindex is false
    std::string index;
    // If this is empty it will accept any method
    std::vector<std::string> accepted_methods;

    RouteCfg();
    RouteCfg(const RouteCfg& copy);
    ~RouteCfg();
    RouteCfg operator=(const RouteCfg& copy);

};

// Has config about server blocks
struct ServerCfg{
    short port;
    std::vector<std::string> server_names;

    // First argument is the error code, the second argument is the path to a
    // file.
    std::map<short, std::string> error_pages;
    int max_body_size;

    // This should be an absolute path
    std::string root_dir;
    std::vector<RouteCfg> routes;

    // Constructors/Destructor
    ServerCfg();
    ServerCfg(const ServerCfg& copy);

    ~ServerCfg();

    ServerCfg operator=(const ServerCfg& copy);
};

class ServerConfig {
public: //TODO: make this private, if this is in a PR it should be rejected
    // A list of commands which will be used in 'cgi'. Having one collecion
    // store all of these will prevent us from having to store many duplicate
    // commands. This vector will store null terminated arrays.
    std::vector<char **> cgi_cmds;
    // The first argument is the file extension, the second is a pointer to
    // an argv array for the command to launch.
    std::map<std::string, char**> cgi;

    // This will store mime types and their respective content-type. The first
    // argument is a file extensions, and the second argument is a content-type
    std::map<std::string, std::string> mime;
    
    std::vector<ServerCfg> servers;

    ServerConfig();
public:
    // Constructors/Destructors
    ServerConfig(const std::string& file); // will take a file to parse
    ServerConfig(const ServerConfig& copy);

    ~ServerConfig();

    ServerConfig& operator=(const ServerConfig& copy);
};

#endif
