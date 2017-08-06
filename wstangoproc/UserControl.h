#ifndef USERCONTROL
#define USERCONTROL
#include <tango.h>
#include "common.h"

#include "ConnectionData.h"

namespace WebSocketDS_ns
{
    class UserControl
    {
    public:
        UserControl(string authDS, TYPE_OF_IDENT toi, bool isLogActive);
        ~UserControl(){};

        bool check_permission(const ParsedInputJson& parsedInputJson, const unordered_map<string, string> &remoteConf, string deviceName, bool isGroup, string &mess, TYPE_WS_REQ typeWsReq);
        pair<bool, string> getInformationFromCheckingUser(const ConnectionData& connectionData);
        bool sendLogCommand(const WebSocketDS_ns::ParsedInputJson &parsedInputJson, const std::unordered_map<std::string, std::string> &remoteConf, string deviceName, bool isGroup, bool status, TYPE_WS_REQ typeWsReq);
    private:
        bool check_user(const unordered_map<string, string> &parsedGet, string& mess);
        bool check_user_rident(string login, string rand_ident, string rand_ident_hash, string& errMess);

        vector<string> getPermissionData(const ParsedInputJson &parsedInputJson, const std::unordered_map<std::string, std::string> &remoteConf, const string &deviceName, TYPE_WS_REQ typeWsReq);
        bool sendLog(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, pair<bool,bool> isAuthOrStatusAndIsGroup);
        bool checkKeysFromParsedGet(const unordered_map<string, string> &parsedGet);
    
    private:
        string _authDS;
        TYPE_OF_IDENT _toi;
        bool _isLogActive;
        bool _hasConf{ false };
    };
}

#endif

