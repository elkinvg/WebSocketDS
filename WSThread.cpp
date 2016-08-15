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
    //DEBUG_STREAM << " Input message: " << msg->get_payload() << endl;
    string data_from_client = msg->get_payload();
    INFO_STREAM << " Input message: " << msg->get_payload() << endl;
    Tango::DevString input = const_cast<Tango::DevString>(data_from_client.c_str());
    map<string, string> conf = getRemoteConf(hdl);
    Tango::DevString output;
    bool permission = uc->check_permission(conf, input);
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
        isValid = uc->check_user(remoteConf);
    }
    else {
        DEBUG_STREAM << "query from GET is empty " << endl;
    }
    return isValid;
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

