#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>
#include <string>
#include <vector>


namespace WebSocketDS_ns
{
    // тип запроса. 
    enum class TYPE_WS_REQ { ATTRIBUTE, COMMAND, PIPE, PIPE_COMM , UNKNOWN};
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

    enum class TYPE_OF_VAL { VALUE, ARRAY, NONE };

    struct ParsedInputJson {
        TYPE_OF_VAL check_key(std::string key) const {
            if (otherInpStr.find(key) != otherInpStr.end())
                return TYPE_OF_VAL::VALUE;
            if (otherInpVec.find(key) != otherInpVec.end())
                return TYPE_OF_VAL::ARRAY;
            else
                return TYPE_OF_VAL::NONE;
        };

        bool isOk{ false };
        std::string errMess;
        std::string id;
        std::string type_req;
        std::string inputJson;
        std::unordered_map<std::string, std::vector<std::string>> otherInpVec;
        std::unordered_map<std::string, std::string> otherInpStr;
        std::unordered_map<std::string, std::string> remoteConf;
    };
}

#endif // COMMON_H
