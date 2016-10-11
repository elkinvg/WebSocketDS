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
    //Tango::DevString output;
    string output;
    bool permission = uc->check_permission(conf, input);
    if (permission)
        output = (string)ds->send_command_to_device(input);
    else
        output = "{\"event\": \"error\", \"data\": [{\"error\":\"Permission denied\", \"type_req\": \"command\"}] }";
        //output = "{\"error\":\"Permission denied\"}";

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

void  WSThread::on_fail(websocketpp::connection_hdl hdl) {
    ERROR_STREAM << " Fail from WSThread on_fail " << endl;
#ifdef TESTFAIL
    std::fstream fs;
    fs.open ("/tmp/tango_log/web_socket/test_log.out", std::fstream::in | std::fstream::out | std::fstream::app);
    Tango::DevULong cTime;
    std::chrono::seconds  timeFromUpdateData= std::chrono::seconds(std::time(NULL));
    cTime = timeFromUpdateData.count();
    fs << cTime << " : Fail from WSThread" << endl;

    fs.close();
#endif
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
    // For parsing of get Query
    //
    // ws://address?arg1=val&arg2=val&arg3=val
    // For check_permission method Query mustcontain arguments "login" and "password"
    // If defined #USERANDIDENT method Query must contain 
    // arguments "login", "id_ri","rand_ident_hash" and "rand_ident"


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

