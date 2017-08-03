#ifndef CONNECTIONDATA
#define CONNECTIONDATA

#include <string>
#include <unordered_map>

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
//#include <websocketpp/common/thread.hpp>
#include "EventProc.h"
#include "TangoConnForClient.h"


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

    struct TimingStruct {
        bool isTimerOn{ false };
        unsigned long msec;
        websocketpp::server<websocketpp::config::asio>::timer_ptr m_timer;
    };

    class ConnectionData {
    public:
        ConnectionData(){}
        ~ConnectionData(){}

        ConnectionData& operator=(ConnectionData& data) = delete;
        ConnectionData(ConnectionData&& other) = delete;

        ConnectionData& operator=(ConnectionData&& data)
        {
            this->tangoConnForClient = std::move(data.tangoConnForClient);
            this->remoteConf = std::move(data.remoteConf);
            this->userCheckStatus = std::move(data.userCheckStatus);
            this->sessionId = data.sessionId;
            return *this;
        }

        unsigned long sessionId;
        bool userStatus{ false };
        json_val_map remoteConf;
        std::pair<bool, std::string> userCheckStatus;
        ForRandIdent2 forRandIdent2;
        std::string name;
        unique_ptr<TangoConnForClient> tangoConnForClient = nullptr;
        unique_ptr<TimingStruct> timing = nullptr;
        unique_ptr<EventProc> eventProc = nullptr;

        unsigned long timerInd{ 0 };
    };
}

#endif
