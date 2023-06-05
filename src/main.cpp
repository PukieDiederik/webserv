#include <iostream>
#include "ServerConfig.hpp"
#include "Router.hpp"
#include <exception>

int main(int argc, char **argv)
{
    (void) argc, (void) argv;

    ServerConfig sc("server.cfg");
    Router r(sc);
    r.listen();
    std::cout << "Hello World!" << std::endl;
}
