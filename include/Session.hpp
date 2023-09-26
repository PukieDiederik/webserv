#ifndef __SESSION_HPP__
#define __SESSION_HPP__

# include "JSON.hpp"
# include <string>
# include <map>
# include <ctime>

std::string urldecode( const std::string &value );
std::string urlencode( const std::string &value );
const std::string   timeToString( time_t timestamp );

class Session {
    public:
        typedef std::map<std::string, std::string> cookies_t;
    private:
        std::string      _session_id;
        cookies_t        _cookies;
        std::string      _client_ip;
        time_t           _last_log;
        time_t           _genesis;

    public:
        Session();
        ~Session();
        Session( const Session& ref );
        Session& operator=( const Session& ref );
        std::string createSessionID();
        
        void    setSessionID( const std::string& new_id );
        std::string getSessionID();

        void    addCookie( std::string type, const std::string& new_cookie );
        void    addCookies( const std::map<std::string, std::string>& new_cookies );
        void    updateCookies( const Session::cookies_t& update_cookies);
        Session::cookies_t getCookies() const;
        void    removeCookie( const std::string& name );

        void            setIp( const std::string& ip );
        std::string     getIp() const;

        time_t     getGenesis() const;

        void    updateStamp();
};

#endif
