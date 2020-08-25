#include "UserControl.h"
#include <chrono>
#include "ConnectionData.h"
#include "ParsingInputJson.h"
#include "StringProc.h"

WebSocketDS_ns::UserControl::UserControl(string authDS):
    _authDS(authDS),
    _toi(TYPE_OF_IDENT::SIMPLE),
    _isLogActive(false),
    _command_name_for_user_control("check_user"),
    _command_name_for_check_permission("check_permissions"),
    _command_name_for_log("send_log_command_ex")
{
}

string WebSocketDS_ns::UserControl::check_permission(const ParsedInputJson& parsedInput, const ConnectionData *connData, string deviceName, bool isGroup) {
    string mess;
    bool isAuth = false;
    vector <string> permission_data = getPermissionData(parsedInput, connData, deviceName);

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

        // Если включён режим отправления данных в журналы.
        // Если задан uselog в Property "Options". Подробнее в README.md
        if (_isLogActive)
            sendLog(authProxy, permission_data, parsedInput.inputJson, isGroup, isAuth);


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

        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        vector<string> errMess;
        std::stringstream ss;
        errMess.push_back(string("Exception from AuthDS " + _authDS));
        ss << "Exception from AuthDS " << _authDS << " .. Desc:";
        for (unsigned int i = 0; i < e.errors.length(); i++)
            errMess.push_back(string(e.errors[i].desc));

        if (authProxy!=nullptr)
            delete authProxy;

        // TODO: CHECK
        if (_toi == TYPE_OF_IDENT::PERMISSION_WWW && _isLogActive) {
            sendLog(authProxy, permission_data, parsedInput.inputJson, isAuth, isGroup);
        }

        throw std::runtime_error(
            StringProc::exceptionStringOut(
                ERROR_TYPE::AUTH_SERVER_ERR,
                parsedInput.id,
                errMess,
                parsedInput.type_req_str
            ));
    }
    catch (std::exception &e)
    {
        std::stringstream ss;
        ss << "Exception from AuthDS " << _authDS << " .. Desc: ";
        ss << e.what();
        mess = ss.str();
        if (authProxy != nullptr)
            delete authProxy;

        throw std::runtime_error(
            StringProc::exceptionStringOut(
                ERROR_TYPE::AUTH_PERM,
                parsedInput.id,
                mess,
                parsedInput.type_req_str
            ));
    }


    if (!isAuth) {
        throw std::runtime_error(
            StringProc::exceptionStringOut(
                ERROR_TYPE::AUTH_PERM,
                parsedInput.id,
                mess,
                parsedInput.type_req_str
            ));
    }

    return mess;
}

void WebSocketDS_ns::UserControl::setCommandNameForCheckUser(const string& new_command_name) {
    _command_name_for_user_control = new_command_name;
}

void WebSocketDS_ns::UserControl::setCommandNameForCheckPermission(const string& new_command_name) {
    _command_name_for_check_permission = new_command_name;
}

void WebSocketDS_ns::UserControl::setCommandNameForLog(const string & new_command_name)
{
    _command_name_for_log = new_command_name;
}

void WebSocketDS_ns::UserControl::setTypeOfIdent(const TYPE_OF_IDENT & toi)
{
    _toi = toi;
}

void WebSocketDS_ns::UserControl::setLogActive()
{
    _isLogActive = true;
}

void WebSocketDS_ns::UserControl::check_user(const string& login, const string& password) {

    vector<string> auth_data;
    bool status;

    auth_data.push_back(login);
    auth_data.push_back(password);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << auth_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        argout = authProxy->command_inout(_command_name_for_user_control, argin);
        argout >> status;
        delete authProxy;

        if (!status) {
            string errMess = "Incorrect password for " + login + " or user is not registered";
            throw std::runtime_error(StringProc::exceptionStringOut(
                ERROR_TYPE::AUTH_CHECK,
                errMess,
                "authentification"
            ));
        }
    }
    catch (Tango::DevFailed &e) {
        vector<string> errMess;
        errMess.push_back("Could not connect to auth-device-server " + _authDS);
        for (unsigned int i = 0; i < e.errors.length(); i++) {
            errMess.push_back(string(e.errors[i].desc));
        }
        if (authProxy != nullptr)
            delete authProxy;
        throw std::runtime_error(StringProc::exceptionStringOut(
            ERROR_TYPE::AUTH_SERVER_ERR,
            errMess,
            "authentification"
        ));
    }
}

// TODO: Set log device from property
bool WebSocketDS_ns::UserControl::sendLog(Tango::DeviceProxy *authProxy, const vector<string>& permission_data, const string&  commandJson, bool isGroup, bool status)
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
    std::chrono::seconds unix_timestamp = std::chrono::seconds(time(NULL));
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
    permission_data_for_log.push_back(to_string(status)); // isAuth
    permission_data_for_log.push_back(to_string(isGroup)); // isGroup
    
    Tango::DeviceData argin, argout;

    try {
        argin << permission_data_for_log;
        argout = authProxy->command_inout(_command_name_for_log, argin);
        argout >> isSuccess;
    }
    catch (Tango::DevFailed &e) {
        isSuccess = false;
    }
    return isSuccess;
}

vector<string> WebSocketDS_ns::UserControl::getPermissionData(const ParsedInputJson &parsedInput, const ConnectionData *connData, const string &deviceName)
{
    // Проверка ключей из remoteConf проводится checkKeysFromParsedGet
    vector <string> permission_data;
    TYPE_WS_REQ typeWsReq = parsedInput.type_req;

    if (_toi != TYPE_OF_IDENT::PERMISSION_WWW) {
        permission_data.resize(4);

        permission_data[0] = deviceName; // device
        if (
            typeWsReq == TYPE_WS_REQ::COMMAND
            ) {
            permission_data[1] = parsedInput.otherInpStr.at("command_name"); // commandName
        }
        else if (
            typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE
            ) {
            permission_data[1] = parsedInput.otherInpStr.at("attr_name"); // commandName
        }
            
        // TODO: По опции также проводить авторизацию для всех остальных readonly запросов
        else {
            throw std::runtime_error(
                StringProc::exceptionStringOut(
                    ERROR_TYPE::CHECK_CODE,
                    parsedInput.id,
                    "check getPermissionData",
                    parsedInput.type_req_str
                ));
        }
        permission_data[2] = connData->ip_client;
        permission_data[3] = connData->login;
    }
    else { // Для аутентификации в Егоровом AuthDS в check_permissions_www
        permission_data.resize(5);
        permission_data[0] = connData->login;
        permission_data[1] = connData->password;
        permission_data[2] = deviceName;

        if (
            typeWsReq == TYPE_WS_REQ::COMMAND
            ) {
            permission_data[3] = parsedInput.otherInpStr.at("command_name") + "/write"; // commandName
        }
        else if (
            typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE
            ) {
            permission_data[3] = parsedInput.otherInpStr.at("attr_name") + "/write"; // commandName
        }

        permission_data[4] = connData->ip_client;
    }
    
    return permission_data;
}
