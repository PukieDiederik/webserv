#include <iostream>
#include "ServerConfig.hpp"

int main(int argc, char **argv)
{
    (void) argc, (void) argv;

    ServerConfig sc("server.cfg");
    std::cout << "Hello World!" << std::endl;
}
