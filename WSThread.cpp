#include <tango.h>
#include <omnithread.h>
#include <log4tango.h>
#include <cmath>
#include "WebSocketDS.h"
#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

namespace WebSocketDS_ns
{

void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    DEBUG_STREAM << " Input message: " << msg->get_payload() << endl;
    string data_from_client = msg->get_payload();

    Tango::DevString input = const_cast<Tango::DevString>(data_from_client.c_str());
    map<string, string> conf = getRemoteConf(hdl);
    Tango::DevString output;
    bool permission = check_permission(conf, input);
    if (permission)
        output = ds->send_command_to_device(input);
    else
        output = CORBA::string_dup("{\"error\":\"Permission denied\"}");

    send(hdl, output);
}

void WSThread::on_open(websocketpp::connection_hdl hdl) {
    DEBUG_STREAM << "New user has been connected!!" << endl;
    websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
    m_connections.insert(hdl);
    send(hdl, cache);
}

void WSThread::on_close(websocketpp::connection_hdl hdl) {
    DEBUG_STREAM << "User has been disconnected!!" << endl;
    websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
    m_connections.erase(hdl);
}


string WSThread::parseOfAddress(string addrFromConn) {
    string out = "";
    string tmpFnd;

    std::size_t found = addrFromConn.find("]");
    if (found == std::string::npos)
        return out;
    
    tmpFnd = addrFromConn.substr(0, found);

    found = tmpFnd.find_last_of(":");
    if (found == std::string::npos)
        return out;

    out = tmpFnd.substr(found+1);

    return out;
}

map<string, string> WSThread::parseOfGetQuery(string query) {
    map<string, string> outMap;
    if (query.size() == 0)
        return outMap;

    vector<string> outSplit;
    outSplit.clear();
    outSplit = split(query,'&');

    for (auto &str : outSplit) {
        vector<string> tmpStr = split(str, '=');
        if (tmpStr.size() != 2)
            continue;
        outMap.insert(std::pair<string, string>(tmpStr[0], tmpStr[1]));
    }

    return outMap;
}


bool WSThread::forValidate(map<string, string> remoteConf) {
    bool isValid = false;

    if (!remoteConf.empty()) {
        isValid = check_user(remoteConf);
    }
    else {
        DEBUG_STREAM << "query from GET is empty " << endl;
    }
    return isValid;
}

bool WSThread::check_user(map<string, string>& parsedGet) {
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

bool WSThread::check_permission(map<string, string>& parsedGet,  Tango::DevString commandJson) {
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

    Tango::DeviceData argin, argout;

    try {
        argin << permission_data;
        Tango::DeviceProxy *authProxy = new Tango::DeviceProxy(ds->authDS);
        argout = authProxy->command_inout("check_permissions", argin);
        argout >> isAuth;
        delete authProxy;
    }
    catch (Tango::DevFailed &e) {
        ERROR_STREAM << "Could not connect to device-server '" << ds->authDS << "'.. Desc: " << e.errors[0].desc.in();
    }
    return isAuth;
}

string WSThread::getCommandName(const string& commandJson) {
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

    out = tmp.substr(found + 1, found2 - (found +1));
    out.erase(std::remove(out.begin(), out.end(), ' '), out.end());
    //out.erase(std::remove_if(out.begin(), out.end(), std::isspace), out.end());
    return out;
}

vector<string> &WSThread::split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


vector<string> WSThread::split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

WSThread::~WSThread() {}

}

