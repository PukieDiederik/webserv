#include "ServerConfig.hpp"

int main(int argc, char **argv) {
	(void) argc;

	if (argc != 2) {
		std::cout << "error: missing config file" << std::endl;
		return (0);
	}

	std::cout << "\nIf any possible errors do occur: \n" << std::endl;
	ServerConfig sc(argv[1]);
}
