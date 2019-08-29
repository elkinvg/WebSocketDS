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

        bool check_permission(const ParsedInputJson& parsedInputJson, const ConnectionData *connData, string deviceName, bool isGroup, string &mess, TYPE_WS_REQ typeWsReq);

        void setCommandNameForCheckUser(const string& new_command_name);
        void setCommandNameForCheckPermission(const string& new_command_name);

        bool sendLogCommand(const WebSocketDS_ns::ParsedInputJson &parsedInputJson, const ConnectionData *connData, string deviceName, bool isGroup, bool status, TYPE_WS_REQ typeWsReq);
        pair<bool, string> check_user(const string& login, const string& password);
        pair<bool, string> check_user_rident(string login, string rand_ident, string rand_ident_hash);
    private:
        vector<string> getPermissionData(const ParsedInputJson &parsedInputJson, const ConnectionData *connData, const string &deviceName, TYPE_WS_REQ typeWsReq);
        bool sendLog(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, pair<bool,bool> isAuthOrStatusAndIsGroup);
    
    private:
        string _authDS;
        TYPE_OF_IDENT _toi;
        bool _isLogActive;
        bool _hasConf{ false };
        string _command_name_for_user_control;
        string _command_name_for_check_permission;
    };
}

#endif

