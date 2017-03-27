#include <tango.h>
#include <omnithread.h>
#include <log4tango.h>
#include <cmath>
#include "WebSocketDS.h"
#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "UserControl.h"
#include "WSThread.h"

#include "StringProc.h"

namespace WebSocketDS_ns
{
    WSThread::WSThread(WebSocketDS *dev/*, std::string hostName*/, int portNumber) : omni_thread(), Tango::LogAdapter(dev)
    {
        //host = hostName;
        port = portNumber;
        ds = dev;
        uc = unique_ptr<UserControl>(new UserControl(dev));
    }

void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    string data_from_client = msg->get_payload();
    INFO_STREAM << " Input message: " << msg->get_payload() << endl;
    
    map<string, string> conf = getRemoteConf(hdl);


    auto parsedJson = StringProc::parseJsonFromCommand(data_from_client, ds->isGroup());

    string commandName;

    if (parsedJson.find("error") != parsedJson.end()) {
        auto errorMess = StringProc::exceptionStringOut(NONE, "The input message (command) must be in json format", parsedJson["error"],"unknown");
        send(hdl, errorMess);
        return;
    }

    bool isPipe = false;

    if (ds->isGroup()) {
        if (parsedJson["read_pipe_gr"] != NONE || parsedJson["read_pipe_dev"] != NONE )
            isPipe = true;
        else {
            commandName = parsedJson["command_device"];
            if (!commandName.size() || commandName == NONE)
                commandName = parsedJson["command_group"];
            }
    }
    else {
        if (parsedJson["read_pipe"] != NONE)
            isPipe = true;
        else
            commandName = parsedJson["command"];
    }

    if (isPipe) {
        getDataFromTangoPipe(hdl,parsedJson);
    }
    else {
        bool permission = uc->check_permission(conf, data_from_client, commandName);
        if (permission)
            sendCommandToTango(hdl, commandName, data_from_client);
        else
        {
            string output = StringProc::exceptionStringOut(parsedJson["id"], commandName,"Permission denied","command");
            send(hdl, output);
        }
    }
}

void WSThread::on_open(websocketpp::connection_hdl hdl) {
    DEBUG_STREAM << "New user has been connected!!" << endl;
    websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
    m_connections.insert(hdl);
    ds->attr_NumberOfConnections_read[0] = m_connections.size();
    send(hdl, cache);
}

void WSThread::on_close(websocketpp::connection_hdl hdl) {
    DEBUG_STREAM << "User has been disconnected!!" << endl;
    websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
    m_connections.erase(hdl);
    ds->attr_NumberOfConnections_read[0] = m_connections.size();
}

void  WSThread::on_fail(websocketpp::connection_hdl hdl) {
    //ERROR_STREAM << " Fail from WSThread on_fail " << endl;
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

void WSThread::sendCommandToTango(websocketpp::connection_hdl hdl, string commandName, string dataFromClient)
{
    string output;
    OUTPUT_DATA_TYPE odt = ds->check_type_of_data(commandName);
    Tango::DevString input = const_cast<Tango::DevString>(dataFromClient.c_str());
    if (odt == OUTPUT_DATA_TYPE::BINARY) {

        Tango::DevVarCharArray* bindata = ds->send_command_bin(input);
        unsigned long len = bindata->length();
        if (len > 0) {
            for (int i = 0; i < len; i++) {
                output.push_back((*bindata)[i]);
            }
            send(hdl, output.c_str(), len);
        }
        else {
            output = "err" + StringProc::exceptionStringOut(NONE, commandName, "No data", "command");
            send(hdl, output.c_str(), output.size());
        }
    }
    else {
        output = (string)ds->send_command(input);
        removeSymbolsForString(output);
        send(hdl, output);
    }
}

void WSThread::getDataFromTangoPipe(websocketpp::connection_hdl hdl, std::map<string, string> parsedPipeConf)
{
    string output = ds->readPipeFromDeviceOrGroup(parsedPipeConf);
    removeSymbolsForString(output);
    send(hdl, output);
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

void WSThread::removeSymbolsForString(string &str) {
    //if (str.find('\0') != string::npos)
    //    str.erase(remove(str.begin(), str.end(), '\0'), str.end());
    if (str.find('\r') != string::npos)
        str.erase(remove(str.begin(), str.end(), '\r'), str.end());
    if (str.find('\n') != string::npos)
        std::replace(str.begin(), str.end(), '\n', ' ');
}

WSThread::~WSThread() {}

}

