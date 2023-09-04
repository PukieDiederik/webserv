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
}

std::string SessionManager::createSession() {
    std::string id = "";
    Session* new_session = new Session();

    // make sure sessionID does not already exist
    while ( _sessions.find( new_session->getSessionID() ) != _sessions.end() )
        new_session->setSessionID( new_session->createSessionID() );

    // add essential cookies
    id = new_session->getSessionID();
    new_session->addCookie( "sessionID", createCookie( "sessionID", id, "/", "Expires" ) );    
    new_session->addCookie( "user", createCookie( "user", "42", "/", "Expires" ) );    
    _sessions.insert( std::pair<std::string, Session*>( id, new_session ) );
    return id;
}

std::map<std::string, Session*>   SessionManager::getSessions() {
    return _sessions;
}