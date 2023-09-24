#include <iostream>

class SignalException : public std::exception {
	public:
		SignalException(int signum) : _signum(signum) {
			std::cout << "Signal: " << _msg << std::endl;
		}
		virtual const char* what() const throw() { return _msg; }

	private:
	int	_signum;
	char	_msg[1024];
};

void	sighandler(int signum) {
	throw SignalException(signum);
}