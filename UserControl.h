#ifndef USERCONTROL
#define USERCONTROL
#include <tango.h>

namespace WebSocketDS_ns
{
    class WebSocketDS;
    class UserControl : public Tango::LogAdapter
    {
    public:
        UserControl(WebSocketDS *dev);
        ~UserControl(){};

        bool check_permission(map<string, string>& parsedGet, const string& commandJson, const string& commandName);
        bool check_user(map<string, string>& parsedGet);

    private:
        vector<string> getPermissionData(map<string, string>& parsedGet, const string& commandName);
        bool sendLogCommand(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, bool isAuth);
        bool checkKeysFromParsedGet(const map<string, string>& parsedGet);  
    
    private:
        WebSocketDS *ds;
    };
}

#endif

