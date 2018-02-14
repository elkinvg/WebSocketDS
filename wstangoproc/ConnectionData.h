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
    struct ForRandIdent {
        bool isRandSended{ false };
        std::string rand_ident_hash;
        string rand_ident_str;
        // Временное значение логина. Сохраняется в основное только при успешной аутентификации
        string tmp_login;
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

        ConnectionData(ConnectionData&& data) {
            this->tangoConnForClient = std::move(data.tangoConnForClient);
            this->login = std::move(data.login);
            this->password = std::move(data.password);
            this->ip_client = std::move(data.ip_client);
            this->forRandIdent = std::move(data.forRandIdent);
            this->remoteConf = std::move(data.remoteConf);
            this->userCheckStatus = std::move(data.userCheckStatus);
        }

        ConnectionData& operator=(ConnectionData&& data)
        {
            this->tangoConnForClient = std::move(data.tangoConnForClient);
            this->login = std::move(data.login);
            this->password = std::move(data.password);
            this->ip_client = std::move(data.ip_client);
            this->forRandIdent = std::move(data.forRandIdent);
            this->remoteConf = std::move(data.remoteConf);
            this->userCheckStatus = std::move(data.userCheckStatus);
           
            return *this;
        }

        std::string login;
        std::string password;
        std::string ip_client;
        unsigned long sessionId;

        json_val_map remoteConf;
        std::pair<bool, std::string> userCheckStatus;
        
        ForRandIdent forRandIdent;
        unique_ptr<TangoConnForClient> tangoConnForClient = nullptr;

        unique_ptr<TimingStruct> timing = nullptr;
        unique_ptr<EventProc> eventProc = nullptr;

        unsigned long timerInd{ 0 };
    };
}

#endif
