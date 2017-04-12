#include "WSTangoConn.h"
#include "WSThread.h"

#include <tango.h>
#include <omnithread.h>
//#include <log4tango.h>
#include <cmath>

#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>


#include "StringProc.h"

namespace WebSocketDS_ns
{
    WSThread::WSThread(WSTangoConn *tc, int portNumber) : omni_thread()
    {
        port = portNumber;
        _tc = tc;
        logger = _tc->get_logger();
    }

    void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {

        string data_from_client = msg->get_payload();
        INFO_STREAM_F <<  " Input message: " << data_from_client << endl;

        ParsedInputJson parsedInputJson = StringProc::parseInputJson(data_from_client);

        if (!parsedInputJson.isOk) {
            string errorMess;
            if (parsedInputJson.errMess.size())
                errorMess = StringProc::exceptionStringOut(NONE, NONE, parsedInputJson.errMess, "unknown");
            else
                errorMess = StringProc::exceptionStringOut(NONE, NONE, "Unknown message from parsing", "unknown");
            send(hdl, errorMess);
            return;
        }

        if (!parsedInputJson.type_req.size()) {
            // Для старых клиентов отправляющих без type_req
            if (parsedInputJson.check_key("command") == TYPE_OF_VAL::VALUE) {
                parsedInputJson.type_req = "command";
                parsedInputJson.otherInpStr["command_name"] = parsedInputJson.otherInpStr["command"];
            }
            else {
                string errorMess = StringProc::exceptionStringOut(parsedInputJson.id, NONE, "Input json must contain key type_req", "unknown");
                send(hdl, errorMess);
                return;
            }
        }

        parsedInputJson.inputJson = data_from_client;
        parsedInputJson.remoteConf =  getRemoteConf(hdl);

        // Проверка типа запроса происходит в WSTangoConn::getTypeWsReq
        // Если вводимый type_req не соответствует ни одному из перечисленных,
        // высылается сообщение об ошибке
        bool isBinary;
        string resp = _tc->sendRequest(parsedInputJson, isBinary);
        if (isBinary)
            send(hdl, resp.c_str(), resp.size());
        else
            send(hdl, resp);
        return;
    }

    void WSThread::on_open(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM_F << "New user has been connected!!" << endl;
        websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        m_connections.insert(hdl);
        _tc->setNumOfConnections(m_connections.size());
        DEBUG_STREAM_F << m_connections.size() << " client connected!!" << endl;
        send(hdl, cache);
    }

    void WSThread::on_close(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM_F << "User has been disconnected!!" << endl;
        websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        m_connections.erase(hdl);
        _tc->setNumOfConnections(m_connections.size());
        DEBUG_STREAM_F << m_connections.size() << " client connected!!" << endl;
    }

    void  WSThread::on_fail(websocketpp::connection_hdl hdl) {
        ERROR_STREAM_F << " Fail from WSThread on_fail " << endl;
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

        out = tmpFnd.substr(found + 1);

        return out;
    }

    unordered_map<string, string> WSThread::parseOfGetQuery(string query) {
        // For parsing of get Query
        //
        // ws://address?arg1=val&arg2=val&arg3=val
        // For check_permission method Query mustcontain arguments "login" and "password"
        // If defined #USERANDIDENT method Query must contain 
        // arguments "login", "id_ri","rand_ident_hash" and "rand_ident"


        unordered_map<string, string> outMap;
        if (query.size() == 0)
            return outMap;

        vector<string> outSplit;
        outSplit.clear();
        outSplit = split(query, '&');

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
            // ??? !!! USERCHECK isValid = uc->check_user(remoteConf);
        }
        else {
            //DEBUG_STREAM << "query from GET is empty " << endl;
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

