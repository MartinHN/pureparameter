//
// Created by Martin Hermant on 24/10/2018.
//

#ifndef PMCPP_PARAMETERCONF_H
#define PMCPP_PARAMETERCONF_H
#ifndef CUSTOM_IDENTIFIER
#include <string>
typedef std::string Identifier;
#include "StringUtils.hpp"
#endif




#include <vector>
#include <algorithm> // for remofe if
#include <memory> // for smartptr
#include <typeindex> // for typeidx
#include <unordered_map>
#define DO_DBG 0
#ifndef USE_STR_CONV
#define USE_STR_CONV 0
#endif
#if USE_STR_CONV
#include <sstream>
#endif
//using namespace std;
#ifndef USE_DEBUG_PARAM
#define USE_DEBUG_PARAM 1
#endif
bool debugParamsSetting = false; // send debug message each time a param is modified


#ifdef __GNUG__ // GCC

#include <cxxabi.h>
#include <cstdlib>

static std::string readable_name( const char* mangled_name )
{
    int status ;
    char* temp = __cxxabiv1::__cxa_demangle( mangled_name, nullptr, nullptr, &status ) ;
    if(temp)
    {
        std::string result(temp) ;
        std::free(temp) ;
        return result ;
    }
    else return mangled_name ;
}

#else // not GCC

inline std::string readable_name( const char* mangled_name ) { return mangled_name ; }

#endif // __GNUG__

template < typename T > std::string type_to_string()
{ return readable_name( typeid(T).name() ) ; }

template < typename T > std::string type_to_string( const T& obj )
{ return readable_name( typeid(obj).name() ) ; }

#endif //PMCPP_PARAMETERCONF_H
