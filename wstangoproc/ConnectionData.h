#ifndef CONNECTIONDATA
#define CONNECTIONDATA

#include <string>
#include <unordered_map>

namespace WebSocketDS_ns
{
    struct ForRandIdent2 {
        bool identState { false };
        bool isRandSended{ false };
        std::string rand_ident_hash;
        std::string login;
        int rand_ident;
        string rand_ident_str;
    };

    struct ConnectionData {
        unsigned long sessionId;
        bool userStatus{ false };
        std::unordered_map<std::string, std::string> remoteConf;
        std::pair<bool, std::string> userCheckStatus;
        ForRandIdent2 forRandIdent2;
        std::string name;
    };
}

#endif
