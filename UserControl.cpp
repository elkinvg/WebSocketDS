#include "UserControl.h"
#include <chrono>


WebSocketDS_ns::UserControl::UserControl(string authDS, TYPE_OF_IDENT toi, bool isLogActive)
{
    _authDS = authDS;
    _toi = toi;
    _isLogActive = isLogActive;
}

bool WebSocketDS_ns::UserControl::check_permission(map<string, string>& parsedGet, const string& commandJson, std::pair<string, string> deviceAndCommandName, bool isGroup) {
    bool isAuth = false;

    if (!checkKeysFromParsedGet(parsedGet))
        return isAuth;

    if (_toi == TYPE_OF_IDENT::SIMPLE)
    {
        bool cuser = check_user(parsedGet);

        if (!cuser) {
            //ERROR_STREAM << "incorrect login or password " << endl;
            return isAuth;
        }
    }

    vector <string> permission_data = getPermissionData(parsedGet, deviceAndCommandName);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << permission_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        if (_toi == TYPE_OF_IDENT::RANDIDENT)
            argout = authProxy->command_inout("check_permissions_ident", argin);
        else
            argout = authProxy->command_inout("check_permissions", argin);
        argout >> isAuth;
        //INFO_STREAM << "User " << permission_data[3] << " tried to run the command " << commandName  <<". Access status is " << std::boolalpha << isAuth << endl;
        // Если включён режим отправления данных в журналы.
        // Если задан uselog в Property "Options". Подробнее в README.md
        if (_isLogActive)
            sendLogCommand(authProxy, permission_data, commandJson, make_pair(isAuth,isGroup));
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        //ERROR_STREAM << "Could not connect to device-server '" <</*ds->authDS*/_authDS << "'.. Desc: " << e.errors[0].desc.in();
        if (authProxy!=nullptr)
            delete authProxy;
    }
    return isAuth;
}

bool WebSocketDS_ns::UserControl::check_user(map<string, string>& parsedGet) {
    bool isAuth = false;

    vector<string> auth_data;

    if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end())
        return isAuth;

    auth_data.push_back(parsedGet["login"]);
    auth_data.push_back(parsedGet["password"]);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << auth_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        argout = authProxy->command_inout("check_user", argin);
        argout >> isAuth;
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        //ERROR_STREAM << "Could not connect to device-server '" << ds->authDS << "'.. Desc: " << e.errors[0].desc.in();
        if (authProxy != nullptr)
            delete authProxy;
    }

    return isAuth;
}

bool WebSocketDS_ns::UserControl::sendLogCommand(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string& commandJson, pair<bool, bool> isAuthAndIsGroup)
{
    // toLogData[0] = timestamp_string UNIX_TIMESTAMP
    // toLogData[1] = login
    // toLogData[2] = deviceName
    // toLogData[3] = IP
    // toLogData[4] = commandName
    // toLogData[5] = commandJson
    // toLogData[6] = statusBool
    // toLogData[7] = isGroup
    
    bool isSuccess;
    
    Tango::DevULong tv; // TIMESTAMP
    std::chrono::seconds  unix_timestamp = std::chrono::seconds(time(NULL));
    tv = unix_timestamp.count();
    string timestamp_string = to_string(tv);

    vector <string> permission_data_for_log;

    permission_data_for_log.push_back(timestamp_string);
    permission_data_for_log.push_back(permission_data[3]); // login
    permission_data_for_log.push_back(permission_data[0]); // device_name
    permission_data_for_log.push_back(permission_data[2]); // ip
    permission_data_for_log.push_back(permission_data[1]); // command_name
    permission_data_for_log.push_back(commandJson);
    permission_data_for_log.push_back(to_string(isAuthAndIsGroup.first)); // isAuth
    permission_data_for_log.push_back(to_string(isAuthAndIsGroup.second)); // isGroup
    
    Tango::DeviceData argin, argout;

    try {
        argin << permission_data_for_log;
        argout = authProxy->command_inout("send_log_command_ex", argin);
        argout >> isSuccess;
    }
    catch (Tango::DevFailed &e) {
        //ERROR_STREAM << "Could not connect to device-server '" << ds->authDS << "'.. Desc: " << e.errors[0].desc.in();
        isSuccess = false;
    }
    return isSuccess;
}

vector<string> WebSocketDS_ns::UserControl::getPermissionData(map<string, string>& parsedGet, std::pair<string, string> deviceAndCommandName)
{
    vector <string> permission_data;

    if (_toi == TYPE_OF_IDENT::RANDIDENT)
        permission_data.resize(7);
    else
        permission_data.resize(4);

    permission_data[0] = deviceAndCommandName.first; // device
    permission_data[1] = deviceAndCommandName.second; // commandName
    permission_data[2] = parsedGet["ip"];
    permission_data[3] = parsedGet["login"];
    
    if (_toi == TYPE_OF_IDENT::RANDIDENT) {
        permission_data[4] = parsedGet["id_ri"];
        permission_data[5] = parsedGet["rand_ident_hash"];
        permission_data[6] = parsedGet["rand_ident"];
    }
    
    return permission_data;
}

bool WebSocketDS_ns::UserControl::checkKeysFromParsedGet(const map<string, string>& parsedGet)
{
    if (_toi == TYPE_OF_IDENT::RANDIDENT)
    {
        if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("id_ri") == parsedGet.end())
            return false;

        if (parsedGet.find("rand_ident_hash") == parsedGet.end() || parsedGet.find("rand_ident") == parsedGet.end())
            return false;
    }
    else {
        if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end() || parsedGet.find("ip") == parsedGet.end()) {
            //ERROR_STREAM << "login or password or ip not found" << endl;
            return false;
        }
    }
    return true;    
}
