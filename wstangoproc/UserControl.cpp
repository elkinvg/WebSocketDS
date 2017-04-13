#include "UserControl.h"
#include <chrono>

WebSocketDS_ns::UserControl::UserControl(string authDS, TYPE_OF_IDENT toi, bool isLogActive)
{
    _authDS = authDS;
    _toi = toi;
    _isLogActive = isLogActive;
}

bool WebSocketDS_ns::UserControl::check_permission(const ParsedInputJson& parsedInputJson, string deviceName, bool isGroup, string &mess) {
    bool isAuth = false;

    if (!checkKeysFromParsedGet(parsedInputJson.remoteConf)) {
        mess = "login or password or ip not found";
        return isAuth;
    }

    if (_toi == TYPE_OF_IDENT::SIMPLE)
    {
        bool cuser = check_user(parsedInputJson.remoteConf, mess);

        if (!cuser) {
            if (!mess.size())
                mess = "incorrect login or password";
            return isAuth;
        }
    }

    vector <string> permission_data = getPermissionData(parsedInputJson, deviceName);

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
        std::stringstream ss;
        ss << "User " << permission_data[3] << " tried to run the command " << parsedInputJson.otherInpStr.at("command_name")  <<". Access status is " << std::boolalpha << isAuth;
        mess = ss.str();

        // Если включён режим отправления данных в журналы.
        // Если задан uselog в Property "Options". Подробнее в README.md
        // Отправляется только если isAuth == false
        if (_isLogActive && !isAuth)
            sendLogCommand(authProxy, permission_data, parsedInputJson.inputJson, make_pair(isAuth,isGroup));
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        std::stringstream ss;
        ss << "Could not connect to auth-device-server " << _authDS << " .. Desc:";
        for (int i = 0; e.errors.length(); i++)
            ss << " " << e.errors[0].reason << ".";
        mess = ss.str();
        if (authProxy!=nullptr)
            delete authProxy;
    }
    return isAuth;
}

bool WebSocketDS_ns::UserControl::check_user(const unordered_map<string, string>& parsedGet, string& errMess) {
    bool isAuth = false;

    vector<string> auth_data;

    if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end())
        return isAuth;

    auth_data.push_back(parsedGet.at("login"));
    auth_data.push_back(parsedGet.at("password"));

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
        std::stringstream ss;
        ss << "Could not connect to auth-device-server " << _authDS << " .. Desc:";
        for (int i = 0; i < e.errors.length(); i++)
            ss << " " << i << ": " << e.errors[0].desc << ".";
        errMess = ss.str();
        if (authProxy != nullptr)
            delete authProxy;
    }

    return isAuth;
}

bool WebSocketDS_ns::UserControl::sendLogCommand(const WebSocketDS_ns::ParsedInputJson &parsedInputJson, string deviceName, bool isGroup, bool status)
{
    if (!_isLogActive)
        return false;

    vector <string> permission_data = getPermissionData(parsedInputJson, deviceName);
    Tango::DeviceProxy *authProxy = nullptr;
    try {
        authProxy = new Tango::DeviceProxy(_authDS);
        sendLogCommand(authProxy, permission_data, parsedInputJson.inputJson, make_pair(status,isGroup));
        delete authProxy;
        return true;
    }
    catch (Tango::DevFailed){
        if (authProxy != nullptr)
            delete authProxy;
    }
    return false;
}

bool WebSocketDS_ns::UserControl::sendLogCommand(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string& commandJson, pair<bool, bool> isAuthOrStatusAndIsGroup)
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
    permission_data_for_log.push_back(to_string(isAuthOrStatusAndIsGroup.first)); // isAuth
    permission_data_for_log.push_back(to_string(isAuthOrStatusAndIsGroup.second)); // isGroup
    
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

vector<string> WebSocketDS_ns::UserControl::getPermissionData(const ParsedInputJson &parsedInputJson, const string &deviceName)
{
    // Проверка ключей из remoteConf проводится checkKeysFromParsedGet
    auto remoteConf = parsedInputJson.remoteConf;
    vector <string> permission_data;

    if (_toi == TYPE_OF_IDENT::RANDIDENT)
        permission_data.resize(7);
    else
        permission_data.resize(4);

    permission_data[0] = deviceName; // device
    permission_data[1] = parsedInputJson.otherInpStr.at("command_name"); // commandName
    permission_data[2] = remoteConf.at("ip");
    permission_data[3] = remoteConf.at("login");
    
    if (_toi == TYPE_OF_IDENT::RANDIDENT) {
        permission_data[4] = remoteConf.at("id_ri");
        permission_data[5] = remoteConf.at("rand_ident_hash");
        permission_data[6] = remoteConf.at("rand_ident");
    }
    
    return permission_data;
}

bool WebSocketDS_ns::UserControl::checkKeysFromParsedGet(const unordered_map<string, string>& parsedGet)
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
