#include "ServerConfig.hpp"
#include "ParserUtils.hpp"

void	ServerConfig::checker() {
	if (VERBOSE)
		std::cout << "\nServerConfig parsing done." << std::endl;
	int	i = 1;
	if (_servers.empty())
		throw std::runtime_error("no server config found in config file.");
	for (std::vector<ServerCfg>::iterator it = _servers.begin(); it != _servers.end(); it++) {
		if (VERBOSE)
			std::cout << "\nServer[" << i++ << "]:" << std::endl;
		
		if ((*it).host.compare("notdefined") == 0) throw std::runtime_error("missing ipv4 (hostname) config");
		if (VERBOSE)
			std::cout << "\tServer host:\n\t\t[" << (*it).host << "]" << std::endl;

		if ((*it).port == -42) throw std::runtime_error("missing port config");
		if (VERBOSE)
			std::cout << "\tServer port:\n\t\t[" << (*it).port << "]" << std::endl;

		if ((*it).server_names.empty()) throw std::runtime_error("missing server name(s) config");
		if (VERBOSE)
			std::cout << "\tServer name(s): " << std::endl;
		for (std::vector<std::string>::iterator jit = (*it).server_names.begin(); jit != (*it).server_names.end(); jit++) {
			if (VERBOSE)
				std::cout << "\t\t[" << *jit << "]" << std::endl;
		}

		// not necessary for config
		if (VERBOSE)
			std::cout << "\tServer error_pages: " << std::endl;
		for (std::map<short, std::string>::iterator jit = (*it).error_pages.begin(); jit != (*it).error_pages.end(); jit++)
			if (VERBOSE)
				std::cout << "\t\t[" << jit->first << "] -> [" << jit->second << "]" << std::endl;

		// if 0, any amount is valid
		if (VERBOSE)
			std::cout << "\tServer max_body_size:\n\t\t[" << (*it).max_body_size << "]" << std::endl;

		if ((*it).root_dir.compare("nopath") == 0) throw std::runtime_error("missing root dir config");
		if (VERBOSE)
			std::cout << "\tServer root dir:\n\t\t[" << (*it).root_dir << "]" << std::endl;

		if (VERBOSE)
			std::cout << "\tServer route(s):" << std::endl;
		int j = 1;
		for (std::vector<RouteCfg>::iterator jit = (*it).routes.begin(); jit != (*it).routes.end(); jit++) {
			if (VERBOSE) {
				std::cout << "\tRoute[" << j++ << "]" << std::endl;
				std::cout << "\t\tRoute path:\n\t\t\t[" << (*jit).route_path << "]" << std::endl;
			}

			if ((*jit).is_redirect && (*jit).redirect_to.compare("notset") == 0) throw std::runtime_error("missing redirection config");
		       	if ((*jit).is_redirect && VERBOSE) std::cout << "\t\tRoute redirection:\n\t\t\t[" << (*jit).redirect_to << "]" << std::endl;

			if ((*jit).root.compare("nopath") == 0) throw std::runtime_error("missing route root config");
			if (VERBOSE)
				std::cout << "\t\tRoute root:\n\t\t\t[" << (*jit).root << "]" << std::endl;

			if (VERBOSE)
				std::cout << "\t\tRoute cgi enabled:\n\t\t\t[" << std::flush; 
			if ((*jit).cgi_enabled && VERBOSE) std::cout << "yes]" << std::endl;
			else if (VERBOSE)
				std::cout << "no]" << std::endl;

			if (!((*jit).auto_index) && (*jit).index.compare("notgiven") == 0) throw std::runtime_error("missing index config");
			if ((*jit).auto_index && VERBOSE) std::cout << "\t\tRoute auto_index\n\t\t\t[yes]" << std::endl;
			else if (VERBOSE)
				std::cout << "\t\tRoute index:\n\t\t\t[" << (*jit).index << "]" << std::endl;

			if ((*jit).accepted_methods.empty()) {	
				(*jit).accepted_methods.push_back("GET");
				(*jit).accepted_methods.push_back("POST");
				(*jit).accepted_methods.push_back("PATCH");
				(*jit).accepted_methods.push_back("PUT");
			}
			else {
				if (VERBOSE)
					std::cout << "\t\tRoute accepted methods:" << std::endl;
				for (std::vector<std::string>::iterator vit = (*jit).accepted_methods.begin(); vit != (*jit).accepted_methods.end(); vit++) {
					if (VERBOSE)
						std::cout << "\t\t\t[" << *vit << "]" << std::endl;
					if ((*vit).compare("GET") != 0 && (*vit).compare("POST") != 0 && (*vit).compare("PATCH") != 0 && (*vit).compare("PUT") != 0 )
						throw std::runtime_error("Error: invalid route method");
				}
			}

		}

		if (VERBOSE)
			std::cout << "\tServer mimes:" << std::endl;;
		for (std::map<std::string, std::string>::iterator	jit = _mime.begin(); jit != _mime.end(); jit++) {
			if (VERBOSE)
				std::cout << "\t\t[" << jit->first << "] -> [" << jit->second << "]" << std::endl;
		}
	}
}
