#ifndef USER_OPTIONS
#define USER_OPTIONS

#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

namespace WebSocketDS_ns
{
    struct UserOptions
    {
        string precision;
        unordered_map<string, string> precisions; // if attributes
        bool isJsonString;
    };
}
#endif

