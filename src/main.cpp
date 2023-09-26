#include "ServerConfig.hpp"
#include "Router.hpp"
#include <exception>
#include <cstdlib>
#include <iostream>
#include <csignal>

class SignalException : public std::exception {
	public:
		SignalException() {
			std::cout << "Signal: " << _msg << std::endl;
		}
		virtual const char* what() const throw() { return _msg; }

	private:
	char	_msg[1024];
};

void	sighandler(int signum) {
	(void)signum;
	throw SignalException();
}


int main(int argc, char **argv) {
	(void) argc;

	std::signal(SIGINT, sighandler);
	std::signal(SIGQUIT, sighandler);
	std::signal(SIGTERM, sighandler);
	std::signal(SIGKILL, sighandler);
	std::signal(SIGSTOP, sighandler);
	std::signal(SIGTSTP, sighandler);

	if (argc != 2) {
		std::cout << "error: missing config file" << std::endl;
		return (0);
	}

	try {
		ServerConfig::getInstance().initialize(argv[1]);
        // Start listening
        Router r;

        std::cout << "Started listening" << std::endl;

		r.listen();

    } catch (const std::exception &ex) {
		std::cout << ex.what() << std::endl;
	}

	return (0);
}
