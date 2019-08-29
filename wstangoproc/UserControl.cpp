#include "UserControl.h"
#include <chrono>

WebSocketDS_ns::UserControl::UserControl(string authDS, TYPE_OF_IDENT toi, bool isLogActive)
{
    _authDS = authDS;
    _toi = toi;
    _isLogActive = isLogActive;
    _command_name_for_user_control = "check_user";
    _command_name_for_check_permission = "check_permissions";
}

bool WebSocketDS_ns::UserControl::check_permission(const ParsedInputJson& parsedInputJson, const ConnectionData *connData, string deviceName, bool isGroup, string &mess, TYPE_WS_REQ typeWsReq) {
    bool isAuth = false;

    vector <string> permission_data = getPermissionData(parsedInputJson, connData, deviceName, typeWsReq);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << permission_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        if (_toi != TYPE_OF_IDENT::PERMISSION_WWW) {
            argout = authProxy->command_inout(_command_name_for_check_permission, argin);
            argout >> isAuth;
        }
        else {
            authProxy->command_inout("check_permissions_www", argin);
            isAuth = true;
        }
        

        if (_toi != TYPE_OF_IDENT::PERMISSION_WWW) {
            std::stringstream ss;
            ss << "User " << permission_data[3] << " tried to run the command " << permission_data[1] << ". Access status is " << std::boolalpha << isAuth;
            mess = ss.str();
        }
        else {
            std::stringstream ss;
            ss << "User " << permission_data[0] << " tried to run the command " << permission_data[3] << ". Access status is " << std::boolalpha << isAuth;
            mess = ss.str();
        }

        // Если включён режим отправления данных в журналы.
        // Если задан uselog в Property "Options". Подробнее в README.md
        // Отправляется только если isAuth == false
        if (_isLogActive && !isAuth)
            sendLog(authProxy, permission_data, parsedInputJson.inputJson, make_pair(isAuth, isGroup));
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        std::stringstream ss;
        ss << "Exception from AuthDS " << _authDS << " .. Desc:";
        for (unsigned int i = 0; i < e.errors.length(); i++)
            ss << " " << e.errors[0].desc << ".";
        mess = ss.str();
        if (authProxy!=nullptr)
            delete authProxy;
    }
    catch (std::exception &e)
    {
        std::stringstream ss;
        ss << "Exception from AuthDS " << _authDS << " .. Desc: ";
        ss << e.what();
        mess = ss.str();
        if (authProxy != nullptr)
            delete authProxy;
    }
    return isAuth;
}

void WebSocketDS_ns::UserControl::setCommandNameForCheckUser(const string& new_command_name) {
    _command_name_for_user_control = new_command_name;
}

void WebSocketDS_ns::UserControl::setCommandNameForCheckPermission(const string& new_command_name) {
    _command_name_for_check_permission = new_command_name;
}

pair<bool, string> WebSocketDS_ns::UserControl::check_user(const string& login, const string& password) {

    pair<bool, string> authStatus;
    authStatus.first = false;
    vector<string> auth_data;

    auth_data.push_back(login);
    auth_data.push_back(password);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << auth_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        argout = authProxy->command_inout("check_user", argin);
        argout >> authStatus.first;
        delete authProxy;
        
        if (!authStatus.first) {
            authStatus.second = "Incorrect password for " + login + " or user is not registered";
        }
    }
    catch (Tango::DevFailed &e) {
        std::stringstream ss;
        ss << "Could not connect to auth-device-server " << _authDS << " .. Desc:";
        for (unsigned int i = 0; i < e.errors.length(); i++)
            ss << " " << i << ": " << e.errors[0].desc << ".";
        authStatus.second = ss.str();
        if (authProxy != nullptr)
            delete authProxy;
    }

    return authStatus;
}

pair<bool, string> WebSocketDS_ns::UserControl::check_user_rident(string login, string rand_ident, string rand_ident_hash)
{
    pair<bool, string> authStatus;
    authStatus.first = false;

    vector<string> auth_data;

    auth_data.push_back(login);
    auth_data.push_back(rand_ident);
    auth_data.push_back(rand_ident_hash);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << auth_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        argout = authProxy->command_inout("check_user_ident", argin);
        argout >> authStatus.first;
        delete authProxy;

        if (!authStatus.first) {
            authStatus.second = "Incorrect input data for " + login + " or user is not registered";
        }
    }
    catch (Tango::DevFailed &e) {
        std::stringstream ss;
        ss << "Could not connect to auth-device-server " << _authDS << " .. Desc:";
        for (unsigned int i = 0; i < e.errors.length(); i++)
            ss << " " << i << ": " << e.errors[0].desc << ".";
        authStatus.second = ss.str();
        if (authProxy != nullptr)
            delete authProxy;
    }

    return authStatus;
}

bool WebSocketDS_ns::UserControl::sendLogCommand(const WebSocketDS_ns::ParsedInputJson &parsedInputJson, const ConnectionData *connData, string deviceName, bool isGroup, bool status, TYPE_WS_REQ typeWsReq)
{
    if (!_isLogActive)
        return false;

    vector <string> permission_data = getPermissionData(parsedInputJson, connData, deviceName, typeWsReq);
    Tango::DeviceProxy *authProxy = nullptr;
    try {
        authProxy = new Tango::DeviceProxy(_authDS);
        sendLog(authProxy, permission_data, parsedInputJson.inputJson, make_pair(status,isGroup));
        delete authProxy;
        return true;
    }
    catch (Tango::DevFailed){
        if (authProxy != nullptr)
            delete authProxy;
    }
    return false;
}

bool WebSocketDS_ns::UserControl::sendLog(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string& commandJson, pair<bool, bool> isAuthOrStatusAndIsGroup)
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
    if (_toi != TYPE_OF_IDENT::PERMISSION_WWW) {
        permission_data_for_log.push_back(permission_data[3]); // login
        permission_data_for_log.push_back(permission_data[0]); // device_name
        permission_data_for_log.push_back(permission_data[2]); // ip
        permission_data_for_log.push_back(permission_data[1]); // command_name
    }
    else {
        // Для аутентификации в Егоровом AuthDS в check_permissions_www
        permission_data_for_log.push_back(permission_data[0]); // login
        permission_data_for_log.push_back(permission_data[2]); // device_name
        permission_data_for_log.push_back(permission_data[4]); // ip
        permission_data_for_log.push_back(permission_data[3]); // command_name
    }

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

vector<string> WebSocketDS_ns::UserControl::getPermissionData(const ParsedInputJson &parsedInputJson, const ConnectionData *connData, const string &deviceName, TYPE_WS_REQ typeWsReq)
{
    // Проверка ключей из remoteConf проводится checkKeysFromParsedGet
    vector <string> permission_data;

    if (_toi != TYPE_OF_IDENT::PERMISSION_WWW) {
        permission_data.resize(4);

        permission_data[0] = deviceName; // device
        if (typeWsReq == TYPE_WS_REQ::COMMAND || typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT)
            permission_data[1] = parsedInputJson.otherInpStr.at("command_name"); // commandName
        else if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE || typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT_WR)
            permission_data[1] = parsedInputJson.otherInpStr.at("attr_name"); // commandName
        else
            throw std::runtime_error("check getPermissionData");
        permission_data[2] = connData->ip_client;
        permission_data[3] = connData->login;
    }
    else { // Для аутентификации в Егоровом AuthDS в check_permissions_www
        permission_data.resize(5);
        permission_data[0] = connData->login;
        permission_data[1] = connData->password;
        permission_data[2] = deviceName;

        if (typeWsReq == TYPE_WS_REQ::COMMAND || typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT)
            permission_data[3] = parsedInputJson.otherInpStr.at("command_name") + "/write"; // commandName
        else if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE || typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT_WR)
            permission_data[3] = parsedInputJson.otherInpStr.at("attr_name") + "/write"; // commandName

        permission_data[4] = connData->ip_client;
    }
    
    return permission_data;
}
