#include "SessionManager.hpp"
#include <cstdlib>
#include <iostream>
#include <map>

std::string createCookie(const std::string& name, const std::string& value, const std::string& path, const std::string& flag);

SessionManager::SessionManager() {}

SessionManager*    SessionManager::_instance;

void SessionManager::initialize() {
    _instance = NULL;
}

SessionManager*  SessionManager::getInstance() {
    if ( _instance == NULL )
        _instance = new SessionManager();
    return _instance;
}

SessionManager::~SessionManager() {
	for ( std::map<std::string, Session*>::iterator it = _sessions.begin(); it != _sessions.end(); it++ )
        delete it->second;
}

std::string SessionManager::createSession( const std::string& ip) {
    Session* new_session = new Session();
    std::string session_id;

    // make sure sessionID does not already exist
    while ( _sessions.find( new_session->getSessionID() ) != _sessions.end() )
        new_session->setSessionID( new_session->createSessionID() );

    session_id = new_session->getSessionID();

    // add essential cookies
    new_session->addCookie( "session_id", createCookie( "session_id", session_id, "/", "Secure" ) );
    
    // add stamp to session
    new_session->updateStamp();
    
    // add session to map
    _sessions[session_id] = new_session;

    new_session->setIp( ip );

    _sessions_ip[ip] = session_id;

    return session_id;
}

std::map<std::string, Session*>   SessionManager::getSessions() {
    return _sessions;
}

std::map<std::string, std::string>   SessionManager::getSessionsIp() {
    return _sessions_ip;
}

void    debugss( const std::string& msg ) {
    std::cout << "[SessionManager]" << "[" << msg << "]" << std::endl;
}
