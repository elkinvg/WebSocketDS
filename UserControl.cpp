#include "WebSocketDS.h"

bool WebSocketDS_ns::UserControl::check_permission(map<string, string>& parsedGet, Tango::DevString commandJson) {
    bool isAuth = false;
    if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end() || parsedGet.find("ip") == parsedGet.end()) {
        ERROR_STREAM << "login or password or ip not found" << endl;
        return isAuth;
    }
    string commandName = getCommandName(commandJson);
    vector <string> permission_data;
    Tango::DbDevice* dbDev = ds->get_db_device();
    Tango::DbData db_data;
    db_data.push_back(Tango::DbDatum("DeviceServer"));
    dbDev->get_property(db_data);
    string deviceName;
    db_data[0] >> deviceName;

    permission_data.resize(4);
    permission_data[0] = deviceName; // device
    permission_data[1] = commandName;
    permission_data[2] = parsedGet["ip"];
    permission_data[3] = parsedGet["login"];

#ifdef USELOG
    Tango::DevULong tv; // TIMESTAMP
    std::chrono::seconds  unix_timestamp = std::chrono::seconds(std::time(NULL));
    tv = unix_timestamp.count();
    string timestamp_string = to_string(tv);

    vector <string> permission_data_for_log;

    permission_data_for_log.push_back(timestamp_string);
    permission_data_for_log.push_back(parsedGet["login"]);
    permission_data_for_log.push_back(deviceName);
    permission_data_for_log.push_back(parsedGet["ip"]);
    permission_data_for_log.push_back(commandName);
    permission_data_for_log.push_back(commandJson);
#endif

    Tango::DeviceData argin, argout;

    try {
        argin << permission_data;
        Tango::DeviceProxy *authProxy = new Tango::DeviceProxy(ds->authDS);
        argout = authProxy->command_inout("check_permissions", argin);
        argout >> isAuth;
#ifdef USELOG
        string statusBool = to_string(isAuth);
        permission_data_for_log.push_back(statusBool);
        sendLogCommand(permission_data_for_log,authProxy);
#endif
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        ERROR_STREAM << "Could not connect to device-server '" << ds->authDS << "'.. Desc: " << e.errors[0].desc.in();
    }
    return isAuth;
}

bool WebSocketDS_ns::UserControl::check_user(map<string, string>& parsedGet) {
    bool isAuth = false;

    if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end())
        return isAuth;

    vector<string> auth_data;
    auth_data.push_back(parsedGet["login"]);
    auth_data.push_back(parsedGet["password"]);

    Tango::DeviceData argin, argout;

    try {
        argin << auth_data;
        Tango::DeviceProxy *authProxy = new Tango::DeviceProxy(ds->authDS);
        argout = authProxy->command_inout("check_user", argin);
        argout >> isAuth;
        delete authProxy;

        INFO_STREAM << "User " << auth_data[0] << " tried to connect. Connect status is " << std::boolalpha << isAuth << endl;
    }
    catch (Tango::DevFailed &e) {
        ERROR_STREAM << "Could not connect to device-server '" << ds->authDS << "'.. Desc: " << e.errors[0].desc.in();
    }

    return isAuth;
}

#ifdef USELOG
bool WebSocketDS_ns::UserControl::sendLogCommand(vector<string> toLogData, Tango::DeviceProxy *authProxy)
{
    // toLogData[0] = timestamp_string UNIX_TIMESTAMP
    // toLogData[1] = login
    // toLogData[2] = deviceName
    // toLogData[3] = IP
    // toLogData[4] = commandName
    // toLogData[5] = commandJson
    // toLogData[6] = statusBool


    bool isSuccess;
    Tango::DeviceData argin, argout;

    try {
        argin << toLogData;
        argout = authProxy->command_inout("send_log_command_ex", argin);
        argout >> isSuccess;
    }
    catch (Tango::DevFailed &e) {
        ERROR_STREAM << "Could not connect to device-server '" << ds->authDS << "'.. Desc: " << e.errors[0].desc.in();
        isSuccess = false;
    }
    return isSuccess;
}
#endif

string WebSocketDS_ns::UserControl::getCommandName(const string& commandJson) {
    string out = "";

    std::size_t found = commandJson.find("\"command\"");

    if (found == std::string::npos)
        return out;

    string tmp = commandJson.substr(found + 9);
    found = tmp.find(",");
    tmp = tmp.substr(0, found);

    std::size_t found2;
    found = tmp.find_first_of("\"");
    found2 = tmp.find_last_of("\"");

    out = tmp.substr(found + 1, found2 - (found + 1));
    out.erase(std::remove(out.begin(), out.end(), ' '), out.end());
    //out.erase(std::remove_if(out.begin(), out.end(), std::isspace), out.end());
    return out;
}