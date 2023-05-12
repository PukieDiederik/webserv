#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

class ServerConfig {
private:
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
    // SERVER
    //   ROUTES

    ServerConfig();
public:
    // Constructors/Destructors
    ServerConfig(const std::string& file); // will take a file to parse
    ServerConfig(const ServerConfig& copy);

    ~ServerConfig();

    ServerConfig& operator=(const ServerConfig& copy);



    // Accessors
};

#endif
