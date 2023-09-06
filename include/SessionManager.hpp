#ifndef __SESSIONMANAGER_HPP__
#define __SESSIONMANAGER_HPP__

# include "Session.hpp"
# include <string>
# include <map>

# define SM_ON true

class Session;

class SessionManager {
    public:
        typedef std::map<std::string, Session*> sessions_t;
    private:
        static  SessionManager*    _instance;
        SessionManager::sessions_t    _sessions;
        SessionManager::sessions_t   _sessions_ip;
        SessionManager();
        ~SessionManager();

    public:
        static  SessionManager*    getInstance();
        static void initialize();

        std::string    createSession( const std::string& ip );
        SessionManager::sessions_t  getSessions();
        SessionManager::sessions_t  getSessionsIp();
};

#endif