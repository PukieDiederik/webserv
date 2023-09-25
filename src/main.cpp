#include "ServerConfig.hpp"
#include "SessionManager.hpp"
#include "Router.hpp"
#include <exception>
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv) {
	(void) argc;

	if (argc != 2) {
		std::cout << "error: missing config file" << std::endl;
		return (0);
	}

	try {

		ServerConfig::getInstance().initialize(argv[1]);
    SessionManager::initialize();

        // Start listening
        Router r;

        std::cout << "Started listening" << std::endl;

		r.listen();

    } catch (const std::exception &ex) {
		std::cout << ex.what() << std::endl;
	}

	return (0);
}
