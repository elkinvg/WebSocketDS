#include "UserControl.h"
#include <chrono>

WebSocketDS_ns::UserControl::UserControl(string authDS, TYPE_OF_IDENT toi, bool isLogActive)
{
    _authDS = authDS;
    _toi = toi;
    _isLogActive = isLogActive;
}

bool WebSocketDS_ns::UserControl::check_permission(const ParsedInputJson& parsedInputJson, const unordered_map<string, string> &remoteConf, string deviceName, bool isGroup, string &mess, TYPE_WS_REQ typeWsReq) {
    bool isAuth = false;

    vector <string> permission_data = getPermissionData(parsedInputJson, remoteConf, deviceName, typeWsReq);

    Tango::DeviceData argin, argout;
    Tango::DeviceProxy *authProxy = nullptr;

    try {
        argin << permission_data;
        authProxy = new Tango::DeviceProxy(_authDS);
        argout = authProxy->command_inout("check_permissions", argin);
        argout >> isAuth;
        std::stringstream ss;
        ss << "User " << permission_data[3] << " tried to run the command " << permission_data[1] << ". Access status is " << std::boolalpha << isAuth;
        mess = ss.str();

        // Если включён режим отправления данных в журналы.
        // Если задан uselog в Property "Options". Подробнее в README.md
        // Отправляется только если isAuth == false
        if (_isLogActive && !isAuth)
            sendLog(authProxy, permission_data, parsedInputJson.inputJson, make_pair(isAuth, isGroup));
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        std::stringstream ss;
        ss << "Could not connect to auth-device-server " << _authDS << " .. Desc:";
        for (unsigned int i = 0; i < e.errors.length(); i++)
            ss << " " << e.errors[0].reason << ".";
        mess = ss.str();
        if (authProxy!=nullptr)
            delete authProxy;
    }
    return isAuth;
}

pair<bool, string> WebSocketDS_ns::UserControl::getInformationFromCheckingUser(const ConnectionData &connectionData)
{
    bool isAuth = false;
    string mess;

    auto parsedGet = connectionData.remoteConf;

    if (_toi == TYPE_OF_IDENT::SIMPLE || _toi == TYPE_OF_IDENT::RANDIDENT) {
        if (!checkKeysFromParsedGet(parsedGet)) {
            if (_toi == TYPE_OF_IDENT::SIMPLE)
                mess = "login or password or ip not found";
            if (_toi == TYPE_OF_IDENT::RANDIDENT)
                mess = "login or rand_ident_hash and rand_ident or ip not found";
                
            return make_pair(isAuth, mess);
        }
    }    

    if (_toi == TYPE_OF_IDENT::SIMPLE)
    {
        isAuth = check_user(parsedGet, mess);

        if (!isAuth) {
            if (!mess.size())
                mess = "incorrect login or password";
        }
    }
    else if (_toi == TYPE_OF_IDENT::RANDIDENT || _toi == TYPE_OF_IDENT::RANDIDENT2 || _toi == TYPE_OF_IDENT::RANDIDENT3) {
        string login, rand_ident, rand_ident_hash;

        if (_toi == TYPE_OF_IDENT::RANDIDENT) {
            login = connectionData.remoteConf.at("login");
            rand_ident = connectionData.remoteConf.at("rand_ident");
            rand_ident_hash = connectionData.remoteConf.at("rand_ident_hash");
        }
        if (_toi == TYPE_OF_IDENT::RANDIDENT2 || _toi == TYPE_OF_IDENT::RANDIDENT3) {
            login = connectionData.forRandIdent2.login;
            rand_ident_hash = connectionData.forRandIdent2.rand_ident_hash;
        }

        if (_toi == TYPE_OF_IDENT::RANDIDENT3)
            rand_ident = connectionData.forRandIdent2.rand_ident_str;
        if (_toi == TYPE_OF_IDENT::RANDIDENT2)
            rand_ident = std::to_string(connectionData.forRandIdent2.rand_ident);

        isAuth = check_user_rident(login, rand_ident, rand_ident_hash, mess);

        if (!isAuth) {
            if (!mess.size())
                mess = "Incorrect login or rand_ident or rand_ident_hash. ";
        }

    }
    return make_pair(isAuth, mess);
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
        for (unsigned int i = 0; i < e.errors.length(); i++)
            ss << " " << i << ": " << e.errors[0].desc << ".";
        errMess = ss.str();
        if (authProxy != nullptr)
            delete authProxy;
    }

    return isAuth;
}

bool WebSocketDS_ns::UserControl::check_user_rident(string login, string rand_ident, string rand_ident_hash, string& errMess)
{
    bool isAuth = false;
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
        argout >> isAuth;
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        std::stringstream ss;
        ss << "Could not connect to auth-device-server " << _authDS << " .. Desc:";
        for (unsigned int i = 0; i < e.errors.length(); i++)
            ss << " " << i << ": " << e.errors[0].desc << ".";
        errMess = ss.str();
        if (authProxy != nullptr)
            delete authProxy;
    }

    return isAuth;
}

bool WebSocketDS_ns::UserControl::sendLogCommand(const WebSocketDS_ns::ParsedInputJson &parsedInputJson, const std::unordered_map<std::string, std::string> &remoteConf, string deviceName, bool isGroup, bool status, TYPE_WS_REQ typeWsReq)
{
    if (!_isLogActive)
        return false;

    vector <string> permission_data = getPermissionData(parsedInputJson, remoteConf, deviceName, typeWsReq);
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

vector<string> WebSocketDS_ns::UserControl::getPermissionData(const ParsedInputJson &parsedInputJson, const std::unordered_map<std::string, std::string> &remoteConf, const string &deviceName, TYPE_WS_REQ typeWsReq)
{
    // Проверка ключей из remoteConf проводится checkKeysFromParsedGet
    vector <string> permission_data;

    permission_data.resize(4);

    permission_data[0] = deviceName; // device
    if (typeWsReq == TYPE_WS_REQ::COMMAND || typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT)
        permission_data[1] = parsedInputJson.otherInpStr.at("command_name"); // commandName
    else if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE)
        permission_data[1] = parsedInputJson.otherInpStr.at("attr_name"); // commandName
    else
        throw exception("check getPermissionData");
    permission_data[2] = remoteConf.at("ip");
    permission_data[3] = remoteConf.at("login");
    
    return permission_data;
}

bool WebSocketDS_ns::UserControl::checkKeysFromParsedGet(const unordered_map<string, string>& parsedGet)
{
    if (_toi == TYPE_OF_IDENT::RANDIDENT)
    {
        if (parsedGet.find("login") == parsedGet.end() /*|| parsedGet.find("id_ri") == parsedGet.end()*/)
            return false;

        if (parsedGet.find("ip") == parsedGet.end())
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
