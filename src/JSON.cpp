#include "JSON.hpp"

JSON::JSON() {}
JSON::JSON( int intVal ) : _type( INT ), _intVal( intVal ) {}
JSON::JSON( double  doubleVal ) : _type( DOUBLE ), _doubleVal( doubleVal ) {}
JSON::JSON( const char* strVal ) : _type( STRING ), _strVal( strVal ) {}
JSON::JSON( bool boolVal ) : _type( BOOL ), _boolVal( boolVal ) {}

JSON::Type    JSON::getType() {
    return _type;
}
    
std::string JSON::toString() {
    std::stringstream   ss;
    switch ( _type ) {
        case INT:
            ss << _intVal;
            return ss.str();
        case DOUBLE:
            ss << _doubleVal;
            return ss.str();
        case STRING:
            return _strVal;
        case BOOL:
            return ( _boolVal ? "true" : "false" );
    }
    return "";
}

// Serializing map to JSON string
std::string toJson( JSON::map& data ) {
    std::stringstream ss;
    ss << "{";
    for ( std::map<std::string, JSON>::iterator it = data.begin(); it != data.end(); ++it ) {
        if ( it != data.begin() ) {
            ss << ", ";
        }
        ss << "\"" << it->first << "\": ";
        if ( it->second.getType() == JSON::STRING ) {
            ss << "\"";
            ss << it->second.toString();
            ss << "\"";
        }
        else
            ss << it->second.toString();
    }
    ss << "}";
    return ss.str();
}

// Parsing JSON string to map
std::map<std::string, JSON> toMap( const std::string& json ) {
    std::map<std::string, JSON> resultMap;
    std::stringstream ss(json);
    std::string key, value, discard;

    // Assume the first char is '{' and discard it
    ss.get();

    while ( ss.peek() != '}' ) {
        std::getline( ss, key, ':' );
        key = key.substr( key.find_first_of( '\"' ) + 1, key.find_last_of( '\"' ) - 1 );  // Remove quotes and spaces
        
        if ( ss.peek() == ' ' ) ss.get();  // Skip space

        if ( ss.peek() == '\"' ) {
            // STRING
            std::getline( ss, value, ',' );
            //value = value.substr(0, value.length() - 1);  // Remove quotes and potential space/comma
            resultMap[key] = JSON( value.c_str() );
        } else {
            ss >> value;
            if ( value == "true" || value == "true," ) { // bool
                resultMap[key] = JSON( true );
            } else if ( value == "false" || value == "false," ) { // bool
                resultMap[key] = JSON( false );
            } else if ( value.find( '.' ) != std::string::npos ) { // Double
                resultMap[key] = JSON( std::atof( value.c_str() ) );
            } else { //INT
                resultMap[key] = JSON( std::atoi( value.c_str() ) );
            }
        }

        if ( ss.peek() == ',' ) ss.get();  // Skip comma
        if ( ss.peek() == ' ' ) ss.get();  // Skip space
        if ( value[value.size() - 1] == '}' ) break ;
    }

    return resultMap;
}