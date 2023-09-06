#include "Session.hpp"
#include "SessionManager.hpp"
#include "ServerUtils.hpp"
#include "HttpResponse.hpp"
#include <ctime>
#include <sstream>
#include <cstdlib>
#include <vector>
#include <iostream>

std::string createCookie(const std::string& name, const std::string& value, const std::string& path, const std::string& flag);
std::string generateExpirationDate();
void    debugss( const std::string& msg );

/* Constructors */
Session::Session() {
    _session_id = createSessionID();
    _last_log = std::time(0);
}

Session::Session( const Session& ref ) {
    *this = ref;
}

Session& Session::operator=( const Session& ref ) {
    _session_id = ref._session_id;
    _cookies = ref._cookies;
    _last_log = ref._last_log;
    return *this;
}

/* Destructors */
Session::~Session() {
}

/* Setters and getters */
void    Session::setSessionID( const std::string& new_id ) {
    _session_id = new_id;
}

void    Session::addCookie( std::string type, const std::string& new_cookie ) {
    if ( _cookies[type].empty() )
        _cookies[type] = new_cookie;
}

void    Session::addCookies( const std::map<std::string, std::string>& new_cookies ) {
    for ( std::map<std::string, std::string>::const_iterator cookie = new_cookies.begin(); cookie != new_cookies.end(); cookie++ )
        addCookie( cookie->first, cookie->second );
}


std::string Session::getSessionID() {
    return _session_id;
}

std::map<std::string, std::string>    Session::getCookies() const {
    return _cookies;
}

void    Session::updateCookies( const Session::cookies_t& update_cookies ) {
    _cookies = update_cookies;
    updateStamp();
}

void    Session::removeCookie( const std::string& name ) {
    _cookies.erase( name );
}

void    Session::updateStamp() {
    _last_log = std::time(0);
}

void    Session::setIp( const std::string& ip ) {
    _client_ip = ip;
}

std::string Session::getIp() const {
    return _client_ip;
}

/* Helpers */
std::string Session::createSessionID() {
    unsigned long a = 1664525;
    unsigned long c = 1013904223;
    unsigned long m = 4294967296;

    unsigned int    seed = std::time(0);

    seed = (a * seed + c) % m;

    // Use the current time as a seed for randomization
    srand( seed );

    std::ostringstream os;

    // Add some random values
    for ( int i = 0; i < 8; ++i ) {
        int randomValue = rand() % 256;
        os << std::hex << randomValue;
    }

    return os.str();
}

std::string generateExpirationDate() {
    return "Wed, 09 Jun 2024 10:18:14 GMT";
}

std::string createCookie(const std::string& name, const std::string& value, const std::string& path, const std::string& flag) {
    std::stringstream cookieStream;
    cookieStream << name << "=" << value << "; Path=" << path << ";";
    
    if (flag == "Expires")
        cookieStream << " " << flag << "=" << generateExpirationDate() << ";";
    else if (!flag.empty())
        cookieStream << " " << flag << ";";

    return cookieStream.str();
}

Session::cookies_t parseCookies( const Session::cookies_t& headers ) {
    Session::cookies_t  cookies;

    Session::cookies_t::const_iterator it = headers.find( "Cookie" );

    if ( it != headers.end() ) {
        std::stringstream ss( it->second );
        std::string cookie;
        while ( std::getline( ss, cookie, ';' ) ) {
            if ( cookie.find( '=' ) == std::string::npos ) continue;
            const std::string name = removeAfterChar( cookie, '=' );
            const std::string value = removeBeforeChar( cookie, '=');
            debugss( "Cookie found with pair: key-> " + name + " | value-> " + value );
            cookies[name] = value;
        }
    }

    return cookies;
}

bool    validateClientID( const std::map<std::string, Session*>& sessions, Session::cookies_t& req_cookies ) {
    return ( sessions.empty() || sessions.find( req_cookies["session_id"] ) == sessions.end() ); 
}

std::string    handleCookies( const HttpRequest& req, HttpResponse& res ) {
    std::string         session_id;

    // make sure to only continue if request from browser ( Firefox or Chrome )
    try {
        std::string browser = req.headers( "User-Agent" );
        std::string origin = req.headers( "Sec-Fetch-Site" );
        if ( browser.find( "Mozilla" ) == std::string::npos && origin.find( "Mozilla" ) == std::string::npos ) return "";
        //if ( origin.find( "same-origin" ) != std::string::npos ) return ""; // dont read repeated requests from same client
    } catch ( std::exception &ex ) { return ""; }

    std::cout << "ola" << std::endl;

    SessionManager* session_manager = SessionManager::getInstance();

    // parse cookies from request
    Session::cookies_t  req_cookies = parseCookies( req.headers() );

    std::cout << "ola2" << std::endl;

    // no cookies || validation fails ( validation can be further improved by tracking others headers info )
    if ( req_cookies.empty() || req_cookies.find( "session_id" ) == req_cookies.end() || validateClientID( session_manager->getSessions(), req_cookies ) ) {

        // find client ip
        std::string ip;
        Session::cookies_t::const_iterator host_ip = req.headers().find( "Host" );
        if ( host_ip != req.headers().end() ) ip = host_ip->second;

        std::cout << "ola3" << std::endl;

        if ( !( session_manager->getSessions().empty() ) ) {
                    std::cout << "ola4" << std::endl;

            Session::cookies_t::iterator it = session_manager->getSessionsIp().find( ip );
            if ( it != session_manager->getSessionsIp().end() )
                session_id = session_manager->getSessions()[it->second]->getSessionID();
        } else {
        std::cout << "ola5" << std::endl;

            // create new session
            session_id = session_manager->createSession( ip );
            session_manager->getSessions()[session_id]->setIp( ip );
            debugss( "Created new session witg ID: " + session_id + " and ip: " + ip );

        }
    } else {
        session_id = req_cookies["session_id"];
        debugss( "Session found with ID: " + session_id + " and ip: " + session_manager->getSessions()[session_id]->getIp() );
        
        // update cookies in session
        session_manager->getSessions()[session_id]->updateCookies( req_cookies );

        // update session data

    }

    // update res cookies
    res.setCookies( session_manager->getSessions()[session_id]->getCookies() );

    return session_id;
}
