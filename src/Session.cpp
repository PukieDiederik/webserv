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
    // check if exists and update
    for ( Session::cookies_t::const_iterator update_cookie = update_cookies.begin(); update_cookie != update_cookies.end(); update_cookie++ ) {
        if ( _cookies.size() < 200 )
                _cookies[update_cookie->first] = createCookie( update_cookie->first, update_cookie->second, "/", "Expires" );
    }

}

void    Session::removeCookie( const std::string& name ) {
    _cookies.erase( name );
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
    return "Wed, 09 Jun 2021 10:18:14 GMT";
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

    for ( Session::cookies_t::const_iterator header = headers.begin(); header != headers.end(); header++ ) {
            if ( header->first.find( "Cookie" ) != std::string::npos ) {
                if ( header->second.empty() ) continue;
                std::string name = removeAfterChar( header->first, '=' );
                std::string value = removeBeforeChar( header->second, '=' );
                if ( !name.empty() && !value.empty() )
                    cookies[name] = value;
            }
    }
    return cookies;
}

void    handleCookies( const HttpRequest& req, HttpResponse& res ) {
    std::string browser_bool = req.headers( "User-Agent" );
    std::string origin_bool = req.headers( "Sec-Fetch-Site" );
    if ( browser_bool.find( "Mozilla" ) == std::string::npos && browser_bool.find( "Chrome" ) == std::string::npos) return ;
    if ( origin_bool.find( "same-origin" ) != std::string::npos ) return ;

    SessionManager* sessions = SessionManager::getInstance();


    // Parse headers -> cookies
    Session::cookies_t  cookies = parseCookies( req.headers() );

    std::cout << "Reading cookies..." << std::endl;
    for ( Session::cookies_t::iterator it = cookies.begin(); it != cookies.end(); it++ )
        std::cout << "Cookie -> " << it->second << std::endl;
    std::cout << "wtf" << std::endl;

    std::string session_id = cookies["sessionID"];

    // Check if session exits if not create
    if ( session_id.empty() || sessions->getSessions()[session_id] == NULL ) session_id = sessions->createSession();

    // Update cookies
    sessions->getSessions()[session_id]->updateCookies( cookies );

    res.set_header( std::string( "Set-Cookie" ), sessions->getSessions()[session_id]->getCookies()["sessionID"] );
}
