#ifndef USERCONTROL
#define USERCONTROL
#include <tango.h>
#include "common.h"

namespace WebSocketDS_ns
{
    class ConnectionData;
    struct ParsedInputJson;

    class UserControl
    {
    public:
        UserControl(string authDS);
        ~UserControl(){};

        string check_permission(const ParsedInputJson& parsedInput, const ConnectionData *connData, string deviceName, bool isGroup);

        void setCommandNameForCheckUser(const string& new_command_name);
        void setCommandNameForCheckPermission(const string& new_command_name);
        void setCommandNameForLog(const string& new_command_name);

        void setTypeOfIdent(const TYPE_OF_IDENT& toi);

        void setLogActive();

        void check_user(const string& login, const string& password);

    private:
        vector<string> getPermissionData(const ParsedInputJson &parsedInput, const ConnectionData *connData, const string &deviceName);
        bool sendLog(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, bool isGroup, bool status);

    private:
        string _authDS;
        TYPE_OF_IDENT _toi;
        bool _isLogActive;
        string _command_name_for_user_control;
        string _command_name_for_check_permission;
        string _command_name_for_log;
    };
}

#endif

