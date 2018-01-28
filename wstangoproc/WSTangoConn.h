#ifndef WSTANGOCONN_H
#define WSTANGOCONN_H

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include <tango.h>

#include "common.h"
#include <array>

#include <random>

#include "ConnectionData.h"

using std::array;

namespace WebSocketDS_ns
{
    class WSThread;
    class GroupOrDeviceForWs;
    class WebSocketDS;
    class UserControl;
    class EventProcForServerMode;

    class WSTangoConn: public Tango::LogAdapter
    {
    public:
        WSTangoConn(WebSocketDS *dev, pair<string, vector<string>> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, array<vector<string>, 4> event_subcr);

        WSTangoConn(WebSocketDS *dev, pair<string, vector<string>> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, array<vector<string>, 4> event_subcr, string cert, string key);

        ~WSTangoConn();

        string for_update_data();
        void sendRequest(const ParsedInputJson& inputReq, bool& isBinary, ConnectionData& connData, string& out);

        void checkUser(ConnectionData& connData);

        void setNumOfConnections(unsigned long num) {_numOfConnections = num; }
        unsigned long getNumOfConnections() {return _numOfConnections;}
        TYPE_OF_IDENT getTypeOfIdent() {return typeOfIdent;}
        MODE getMode() {return ws_mode;}
        bool isServerMode();
        bool isLogActive() {return _isLogActive; }

        bool isTm100ms() { return _istm100ms; }
        unsigned int getMaxBuffSize();
        unsigned short getMaxNumberOfConnections();
        string getAuthDS();

        string getDeviceNameFromAlias(string alias, string& errorMessage);

    private:
        void initOptionsAndDeviceServer(pair<string, vector<string>>& dsAndOptions, array<vector<string>, 3> &attrCommPipe);

        bool initDeviceServer();
        bool initDeviceServer(array<vector<string>, 3> &attrCommPipe);

        void initEventSubscrForServerMode(const array<vector<string>, 4> &event_subcr);

        string fromException(Tango::DevFailed &e, string func);
        void  removeSymbolsForString(string &str);

        TYPE_WS_REQ getTypeWsReq(const string& req);

        // commands
        void sendRequest_Command(const ParsedInputJson& inputReq, ConnectionData& connData, bool& isBinary, string& resp_json);
        void sendRequest_Command_DevClient(const ParsedInputJson& inputReq, ConnectionData& connData, bool& isBinary, string& resp_json);
        void sendRequest_PipeComm(const ParsedInputJson& inputReq, ConnectionData& connData, string& resp_json);

        // for rident
        string sendRequest_RidentReq(const ParsedInputJson& inputReq, ConnectionData& connData);
        string sendRequest_RidentAns(const ParsedInputJson& inputReq, ConnectionData& connData);
        string sendRequest_Rident(const ParsedInputJson& inputReq, ConnectionData& connData);

        // for authentification
        string sendRequest_ForAuth(const ParsedInputJson& inputReq, ConnectionData& connData);

        // read attribute
        void sendRequest_AttrRead(const ParsedInputJson& inputReq, ConnectionData& connData, string& resp_json);

        // write attribute
        void sendRequest_AttrClient(const ParsedInputJson& inputReq, ConnectionData& connData, string& resp_json);
        void sendRequest_AttrWrite(const ParsedInputJson& inputReq, ConnectionData& connData, string& resp_json);
        void sendRequest_AttrWrite_DevClient(const ParsedInputJson& inputReq, ConnectionData& connData, string& resp_json);

        string checkPermissionForRequest(const ParsedInputJson &inputReq, ConnectionData &connData, string &commandName, string device_name, TYPE_WS_REQ typeWsReq);

        string checkDeviceNameKey(const ParsedInputJson& inputReq, std::string &errorMessage);

        // check status
        void sendRequest_UserStatus(const ParsedInputJson& inputReq, ConnectionData& connData, string& resp_json);

        bool hasAttrOrPipe;


    private:
        WSThread *wsThread = nullptr;
        WebSocketDS *_wsds;
        std::unique_ptr<EventProcForServerMode> eventSubscrServ;

        std::unique_ptr<GroupOrDeviceForWs> groupOrDevice;
        std::unique_ptr<UserControl> uc;
        TYPE_OF_IDENT typeOfIdent{ TYPE_OF_IDENT::SIMPLE };
        MODE ws_mode { MODE::SERVER};
        string _deviceName;
        bool  _isLogActive{ false };
        bool _isInitDs{ false };
        bool _isGroup{ false };
        bool _isShortAttr{ true };
        bool _istm100ms{ false };
        unsigned long _numOfConnections{0};
        string _errorMessage;
        std::default_random_engine generator;
    };
}
#endif
