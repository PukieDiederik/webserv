#include "ServerConfig.hpp"
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

	std::cout << "\nIf any possible errors do occur: \n" << std::endl;
	try {
		ServerConfig::initialize(argv[1]);

        // Start listening
        Router r;

        std::cout << "Started listening" << std::endl;
        
		r.listen();
		
    } catch (const std::exception &ex) {
		std::cout << ex.what() << std::endl;
	}

	return (0);
}
