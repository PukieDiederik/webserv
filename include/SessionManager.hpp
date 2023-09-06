#ifndef __SESSIONMANAGER_HPP__
#define __SESSIONMANAGER_HPP__

# include "Session.hpp"
# include <string>
# include <map>

# define SM_ON true

class Session;

class SessionManager {
    private:
        static  SessionManager*    _instance;
        std::map<std::string, Session*>    _sessions;
        std::map<std::string, std::string> _sessions_ip;
        SessionManager();
        ~SessionManager();

    public:
        static  SessionManager*    getInstance();
        static void initialize();

        std::string    createSession( const std::string& ip );
        std::map<std::string, Session*> getSessions();
        std::map<std::string, std::string>  getSessionsIp();
};

#endif