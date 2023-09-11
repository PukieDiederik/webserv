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
void    debugss( const std::string& event, const std::string& trace );

/* Constructors */
Session::Session() {
    _session_id = createSessionID();
    _last_log = std::time(0);
    _genesis = std::time(0);
}

Session::Session( const Session& ref ) {
    *this = ref;
}

Session& Session::operator=( const Session& ref ) {
    _session_id = ref._session_id;
    _cookies = ref._cookies;
    _client_ip = ref._client_ip;
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

time_t  Session::getGenesis() const {
    return _genesis;
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
            //debugss( "Cookie found", "key: " + name + " | value: " + value );
            cookies[name] = value;
        }
    }

    return cookies;
}

bool    validateClientID( const std::map<std::string, Session*>& sessions, Session::cookies_t& req_cookies ) {
    return ( req_cookies.empty() || req_cookies.find( "session_id" ) == req_cookies.end() || sessions.empty() || sessions.find( req_cookies["session_id"] ) == sessions.end() ); 
}

bool   validateClientIp( const std::string& ip, std::string& session_id ) {
    SessionManager* session_manager = SessionManager::getInstance();

    if ( !( session_manager->getSessions().empty() ) ) {
        SessionManager::sessions_t::iterator it = session_manager->getSessionsIp().find( ip );
        if ( it != session_manager->getSessionsIp().end() ) {
            session_id = it->second->getSessionID();
            return true;
        }
    }
    return false;
}

std::string    handleCookies( const HttpRequest& req, HttpResponse& res ) {
    std::string         session_id;
    bool                vip = false, vid = false;

    // make sure to only continue if request from browser ( Firefox or Chrome )
    try {
        const std::string browser = req.headers( "User-Agent" );
        const std::string origin = req.headers( "Sec-Fetch-Site" );
        if ( browser.find( "Mozilla" ) == std::string::npos && origin.find( "Mozilla" ) == std::string::npos ) return "";
        //if ( origin.find( "same-origin" ) != std::string::npos ) return ""; // dont read repeated requests from same client
    } catch ( std::exception &ex ) { return ""; }

    SessionManager* session_manager = SessionManager::getInstance();

    // parse cookies from request
    Session::cookies_t  req_cookies = parseCookies( req.headers() );

    // find client ip
    std::string ip;
    Session::cookies_t::const_iterator host_ip = req.headers().find( "Host" );
    if ( host_ip != req.headers().end() ) ip = host_ip->second;

    // find if already have session for client
    vip = validateClientIp( ip, session_id );
    vid = !( validateClientID( session_manager->getSessions(), req_cookies ) );

    // no cookies || validation fails ( validation can be further improved by tracking others headers info )
    if ( !vip && !vid ) {

        // create new session
        session_id = session_manager->createSession( ip );
        debugss( "Created session", "ID: " + session_id + " | HOST: " + ip );

        // update session data
        session_manager->getSessions()[session_id]->setIp( ip );

    } else {
        // check if client ip was found
        if ( !vip ) session_id = req_cookies["session_id"];
        debugss( "Session found", "ID: " + session_id + " | HOST: " + session_manager->getSessions()[session_id]->getIp() );
        
        // update cookies in session
        if ( vip && vid)
            session_manager->getSessions()[session_id]->updateCookies( req_cookies );

        // update session data
    }

    // update res cookies
    res.setCookies( session_manager->getSessions()[session_id]->getCookies() );

    return session_id;
}

// encode string to url
std::string urlencode( const std::string &value ) {
    std::ostringstream escaped;
    escaped.fill( '0' );
    escaped << std::hex;

    for ( std::string::const_iterator i = value.begin(); i != value.end(); ++i ) {
        char c = (*i);
        if ( isalnum( c ) || c == '-' || c == '_' || c == '.' || c == '~' ) {
            escaped << c;
        } else {
            escaped << '%' << std::uppercase << static_cast<unsigned short>( static_cast<unsigned char>( c ) );
        }
    }

    return escaped.str();
}

// decode url to str
std::string urldecode( const std::string &value ) {
    std::string ret;
    char ch;
    std::string::size_type i;
    unsigned int ii;
    for ( i = 0; i < value.length(); i++ ) {
        if ( value[i] != '%' ) {
            if ( value[i] == '+' ) {
                ret += ' ';
            } else {
                ret += value[i];
            }
        } else {
            sscanf( value.substr( i + 1, 2 ).c_str(), "%x", &ii );
            ch = static_cast<char>( ii );
            ret += ch;
            i = i + 2;
        }
    }
    return ret;
}

const std::string   timeToString( time_t timestamp ) {
    struct tm *time = localtime( &timestamp );
    std::stringstream   ss;
    ss << time->tm_hour << "h:" << time->tm_min << "m, " << time->tm_mday << "/" << time->tm_mon + 1 << "/" << time->tm_year + 1900;
	return ss.str();
}
