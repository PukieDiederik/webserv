#include "SessionManager.hpp"
#include <cstdlib>
#include <iostream>
#include <map>
#include <cstdarg>
#include <cstring>

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
    new_session->addCookie( "jsjsjs", createCookie( "jsjsjs", session_id, "/", "Secure" ) );
    
    // add session to map
    _sessions[session_id] = new_session;
    _sessions_ip[ip] = new_session;

    // update session data
    new_session->setIp( ip );

    // add stamp to session
    new_session->updateStamp();

    return session_id;
}

SessionManager::sessions_t   SessionManager::getSessions() {
    return _sessions;
}

SessionManager::sessions_t   SessionManager::getSessionsIp() {
    return _sessions_ip;
}

void    debugss( const std::string& event, const std::string& trace ) {
    std::cout << "[SessionManager]" << "[" << event << "][" << trace << "]" << std::endl;
}
