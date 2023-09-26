#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

# include <map>
# include <string>
# include <vector>
# include <fstream>

// Parser
# define SERVER 1
# define CGI 2
# define MIME 3
# define COMMENT 4
# define ERROR 5

# define DIRLISTING	"./var/www/templates/dir_listing.html"
# define MIME_DEFAULT "text/plain"

# ifndef VERBOSE
# define VERBOSE false
# endif

// Has config about routes
struct RouteCfg {
	//Route path
	std::string			route_path;

	// Redirects
	bool				is_redirect;
	std::string			redirect_to;

	// This will be an absolute path
	std::string			root;

	bool				cgi_enabled;

	bool				auto_index;

	// This should raise an error if not given and autoindex is false
	std::string			index;

	// If this is empty it will accept any method
	std::vector<std::string>	accepted_methods;

	RouteCfg();
	RouteCfg(const RouteCfg& copy);
	~RouteCfg();
	RouteCfg	&operator=(const RouteCfg& copy);

};

// Has config about server blocks
struct ServerCfg {
	std::string			host;
	short				port;
	std::vector<std::string>	server_names;

	// First argument is the error code, the second argument is the path to a file.

	std::map<short, std::string>	error_pages;
	unsigned int		    		max_body_size;

	// This should be an absolute path
	std::string			root_dir;
	std::vector<RouteCfg>		routes;

	// Constructors/Destructor
	ServerCfg();
	ServerCfg(const ServerCfg& copy);
	~ServerCfg();
	ServerCfg	&operator=(const ServerCfg& copy);
};

class ServerConfig {
    public:
        typedef std::map<std::string, std::string> mime_tab_t;
	private:
        static ServerConfig _instance;

		// A list of commands which will be used in 'cgi'. Having one collecion
		// store all of these will prevent us from having to store many duplicate
		// commands. This vector will store null terminated arrays.
		std::vector<char **>			_cgi_cmds;
		// The first argument is the file extension, the second is a pointer to
		// an argv array for the command to launch.
		std::map<std::string, char**>		_cgi;

		// This will store mime types and their respective content-type. The first
		// argument is a file extensions, and the second argument is a content-type
		mime_tab_t _mime;


        // Constructors/Destructors
		ServerConfig();
        ServerConfig(const ServerConfig& copy);

	protected:// Parser utils
		void	parseCgi(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf);
		void	parseMime(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf);
		void	parseServer(int &bad_line, bool &keywd_bracket, std::ifstream &fd_conf);
		void	parseServerHost(const std::string &, ServerCfg &, int &bad_line);
		void	parseServerPort(const std::string &, ServerCfg &, int &bad_line);
		void	parseServerNames(const std::string &, ServerCfg &, int &bad_line);
		void	parseServerErrorPages(const std::string &, ServerCfg &, int &bad_line);
		void	parseServerMaxBodySize(const std::string &, ServerCfg &, int &bad_line);
		void	parseServerRoot(const std::string &curr_line, ServerCfg &, int &bad_line);
		void	parseServerRoute(std::string &curr_line, ServerCfg &, int &bad_line, std::ifstream &fd_conf);

		void	checker();
	public:
		std::vector<ServerCfg>	_servers;

 		~ServerConfig();

		ServerConfig& operator=(const ServerConfig& copy);

		static std::string getMimeType(const std::string& filename);

        void initialize(const std::string& filepath);
        static ServerConfig& getInstance();

        static bool	isCgiScript(std::string filename);
        static const std::string	getExecutablePath(std::string filename);
};

#endif
