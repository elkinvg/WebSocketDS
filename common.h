#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>
#include <string>

namespace WebSocketDS_ns
{
    // тип запроса. 
    enum class TYPE_WS_REQ { ATTRIBUTE, COMMAND, PIPE };
    // форматы для IOS
    enum class TYPE_IOS_OPT { PREC, PRECF, PRECS };
    // тип 
    enum class OUTPUT_DATA_TYPE { JSON, BINARY };

    enum class TYPE_OF_IDENT { SIMPLE, RANDIDENT};

    const std::string NONE = "\"NONE\"";

    typedef std::unordered_multimap < std::string, std::string > stringunmap;
    typedef std::pair<stringunmap::iterator, stringunmap::iterator> stringunmap_iter;

    typedef std::unordered_map < std::string, std::string > stringmap;
    typedef std::pair<stringmap::iterator, stringmap::iterator> stringmap_iter;
}

#endif // COMMON_H
