#ifndef __SESSIONMANAGER_HPP__
#define __SESSIONMANAGER_HPP__

# include "Session.hpp"
# include <string>
# include <map>

class Session;

class SessionManager {
    private:
        static  SessionManager*    _instance;
        std::map<std::string, Session*>    _sessions;
        SessionManager();
        ~SessionManager();
    public:
        static  SessionManager*    getInstance();
        static void initialize();

        std::string    createSession();
        std::map<std::string, Session*> getSessions();
};

#endif