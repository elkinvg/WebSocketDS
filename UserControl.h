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

        bool check_permission(map<string, string>& parsedGet, const string& commandJson, std::pair<string,string> deviceAndCommandName, bool isGroup);
        bool check_user(map<string, string>& parsedGet);

    private:
        vector<string> getPermissionData(map<string, string>& parsedGet, std::pair<string, string> deviceAndCommandName);
        bool sendLogCommand(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, pair<bool,bool> isAuthAndIsGroup);
        bool checkKeysFromParsedGet(const map<string, string>& parsedGet);  
    
    private:
        string _authDS;
        TYPE_OF_IDENT _toi;
        bool _isLogActive;
    };
}

#endif

