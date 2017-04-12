#ifndef USERCONTROL
#define USERCONTROL
#include <tango.h>
#include "common.h"

namespace WebSocketDS_ns
{
    class UserControl
    {
    public:
        UserControl(string authDS, TYPE_OF_IDENT toi, bool isLogActive);
        ~UserControl(){};

        bool check_permission(const ParsedInputJson &parsedInputJson, string deviceName, bool isGroup, string& mess);
        bool check_user(const unordered_map<string, string> &parsedGet, string& mess);
        bool sendLogCommand(const ParsedInputJson &parsedInputJson, string deviceName, bool isGroup, bool status);
    private:
        vector<string> getPermissionData(const ParsedInputJson &parsedInputJson, const string &deviceName);
        bool sendLogCommand(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, pair<bool,bool> isAuthOrStatusAndIsGroup);
        bool checkKeysFromParsedGet(const unordered_map<string, string> &parsedGet);
    
    private:
        string _authDS;
        TYPE_OF_IDENT _toi;
        bool _isLogActive;
    };
}

#endif

