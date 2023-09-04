#ifndef __SESSION_HPP__
#define __SESSION_HPP__

# include <string>
# include <map>
# include <ctime>

class Session {
    public:
        typedef std::map<std::string, std::string> cookies_t;
    private:
        std::string      _session_id;
        cookies_t        _cookies;
        time_t           _last_log;
        int              _pid;
    public:
        Session();
        ~Session();
        Session( const Session& ref );
        Session& operator=( const Session& ref );
        
        void    setSessionID( const std::string& new_id );
        std::string getSessionID();

        void    addCookie( std::string type, const std::string& new_cookie );
        void    addCookies( const std::map<std::string, std::string>& new_cookies );
        void    updateCookies( const std::map<std::string, std::string>& update_cookies);
        Session::cookies_t getCookies() const;
        void    removeCookie( const std::string& name );


        std::string createSessionID();

        int     getPid() const;
        void    incrementPid();
        void    resetPid();
};

#endif