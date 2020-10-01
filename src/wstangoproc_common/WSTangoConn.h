#ifndef WSTANGOCONN_H
#define WSTANGOCONN_H

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include <tango.h>
#include "common.h"
#include "ParsingInputJson.h"
#include "TaskInfo.h"

namespace WebSocketDS_ns
{

    class UserControl;
    class ConnectionData;
    class WSThread;

    class WSTangoConn
    {
    public:
        WSTangoConn();
        virtual log4tango::Logger *get_logger(void) = 0;

        unsigned short getMaxNumberOfConnections();
        unsigned int getMaxBuffSize();
        TYPE_OF_IDENT getTypeOfIdent();
        void checkUser(ConnectionData* connData);
        virtual string sendRequest(const ParsedInputJson& parsedInput, ConnectionData *connData) = 0;

        virtual vector<pair<long, TaskInfo>> sendRequestAsync(const ParsedInputJson& parsedInput, ConnectionData *connData, vector<string>& errorsFromGroupReq) = 0;

        virtual string checkResponse(const std::pair<long, TaskInfo>& idInfo) = 0;

        virtual void setNumOfConnections(unsigned long num) = 0;
        virtual void setFalsedConnectionStatus() = 0;

        string commonRequsts(const ParsedInputJson& parsedInput, ConnectionData* connData);

        bool getConnectionStatus() { return _connectionStatus; };

        virtual ~WSTangoConn();
    protected:
        virtual vector<string> getDevOptions() = 0; // TODO: not virtual. Replace to WSTangoConn.cpp
        string _forCheckResponse(const std::pair<long, TaskInfo>& idInfo, Tango::Group* groupForWs);
        string _forCheckResponse(const std::pair<long, TaskInfo>& idInfo, Tango::DeviceProxy* deviceForWs);
        string fromException(Tango::DevFailed &e, string func);
        void removeSymbolsForString(string &str);

        void checkPermissionForRequest(const ParsedInputJson& parsedInput, ConnectionData* connData, const string& device_name, bool isGroupReqest);

        // for authentification
        string sendRequest_ForAuth(const ParsedInputJson& parsedInput, ConnectionData* connData);

        // check status
        string sendRequest_UserStatus(const ParsedInputJson& parsedInput, ConnectionData* connData);

        void initOptions();

    protected:

        unsigned long _numOfConnections{ 0 };
        bool _connectionStatus{ true };
#ifdef SERVER_MODE
        bool _isGroup{ false };
#endif
        std::unique_ptr<UserControl> uc;
        TYPE_OF_IDENT typeOfIdent{ TYPE_OF_IDENT::SIMPLE };
        bool _isOldVersionOfJson{ false };

    private:
        unsigned short _maxNumOfConnection{ 0 };
        unsigned short _maxBufferSize{ 1000 };
    };
}
#endif
