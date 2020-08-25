#ifndef CONNECTIONDATA
#define CONNECTIONDATA

#include <string>
#include <unordered_map>
#include <unordered_set>

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>

#include "common.h"
#include "TaskInfo.h"
#include "CurrentMode.h"


namespace WebSocketDS_ns
{
    class ConnectionData {
    public:
        ConnectionData() {}
        ~ConnectionData() {}

        ConnectionData& operator=(ConnectionData& data) = delete;

        ConnectionData(ConnectionData&& data) {

            this->login = std::move(data.login);
            this->password = std::move(data.password);
            this->ip_client = std::move(data.ip_client);
            this->remoteConf = std::move(data.remoteConf);
            this->userCheckStatus = std::move(data.userCheckStatus);
            this->idCommandInfo = std::move(data.idCommandInfo);
        }

        ConnectionData& operator=(ConnectionData&& data)
        {
            this->login = std::move(data.login);
            this->password = std::move(data.password);
            this->ip_client = std::move(data.ip_client);
            this->remoteConf = std::move(data.remoteConf);
            this->userCheckStatus = std::move(data.userCheckStatus);
            this->idCommandInfo = std::move(data.idCommandInfo);

            return *this;
        }

        std::string login;
        std::string password;
        std::string ip_client;
        unsigned long sessionId;

        json_val_map remoteConf;
        bool userCheckStatus;

        std::unordered_map<long, TaskInfo> idCommandInfo;
    };
}
#endif
