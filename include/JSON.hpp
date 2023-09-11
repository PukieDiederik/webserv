#ifndef __JSON_HPP__
#define __JSON_HPP__

# include <iostream>
# include <map>
# include <string>
# include <sstream>
# include <cstdlib> 

// TODO: add object and array
class   JSON {
    public:
        typedef std::map<std::string, JSON>   map;

        enum Type { INT, DOUBLE, STRING, BOOL }; //, OBJECT, ARRAY };

        JSON();
        JSON( int intVal );
        JSON( double  doubleVal );
        JSON( const char* strVal );
        JSON( bool boolVal );

        Type    getType();
    
        std::string toString();

    private:
        Type _type;

        int _intVal;
        double  _doubleVal;
        std::string _strVal;
        bool    _boolVal;
};

std::string toJson( JSON::map& data );
std::map<std::string, JSON> toMap( const std::string& json );

#endif 