#include "WSTangoConn.h"
#include "WSThread.h"

#include <tango.h>
#include <omnithread.h>
//#include <log4tango.h>
#include <cmath>

#include <locale.h>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>

#include "ConnectionData.h"
#include "EventProc.h"

#include "StringProc.h"
#include "ParsingInputJson.h"

#include "GroupForWs.h"

namespace WebSocketDS_ns
{
    WSThread::WSThread(WSTangoConn *tc, int portNumber) 
        : omni_thread(), m_next_sessionid(1)
    {
        port = portNumber;
        _tc = tc;
        logger = _tc->get_logger();
        parsing = new ParsingInputJson();
    }

    bool WSThread::isAliasMode()
    {
        auto ws_mode = _tc->getMode();
        if (
            ws_mode == MODE::SERVNCLIENT_ALIAS
            || ws_mode == MODE::CLIENT_ALIAS
            || ws_mode == MODE::CLIENT_ALIAS_RO
            || ws_mode == MODE::SERVNCLIENT_ALIAS_RO
            )
            return true;
        else
            return false;
    }

    void WSThread::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
        string data_from_client = msg->get_payload();
        INFO_STREAM_F << " Input message: " << data_from_client << endl;

        ParsedInputJson parsedInputJson = parsing->parseInputJson(data_from_client);

        if (!parsedInputJson.isOk) {
            string errorMess;
            if (parsedInputJson.errMess.size())
                errorMess = StringProc::exceptionStringOut(NONE, NONE, parsedInputJson.errMess, "unknown_req");
            else
                errorMess = StringProc::exceptionStringOut(NONE, NONE, "Unknown message from parsing", "unknown_req");
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
                string errorMess = StringProc::exceptionStringOut(parsedInputJson.id, NONE, "Input json must contain key type_req", "unknown_req");
                send(hdl, errorMess);
                return;
            }
        }
        
        // Действия с событиями
        // eventreq_add_dev - добавление девайсов в подписки
        if (parsedInputJson.type_req.find("eventreq") != string::npos ) {
            if (_tc->getMode() == MODE::SERVER) {
                string errorMess = StringProc::exceptionStringOut(parsedInputJson.id, NONE, "Subscribing to events is not supported in the current mode", parsedInputJson.type_req);
                send(hdl,errorMess);
            }
            else {
                if (m_connections[hdl]->eventProc == nullptr)
                    m_connections[hdl]->eventProc = unique_ptr<EventProc>(new EventProc(hdl, this));
                m_connections[hdl]->eventProc->sendRequest(parsedInputJson);
            }
            return;
        }

        parsedInputJson.inputJson = data_from_client;

        // Проверка типа запроса происходит в WSTangoConn::getTypeWsReq
        // Если вводимый type_req не соответствует ни одному из перечисленных,
        // высылается сообщение об ошибке
        bool isBinary;
        string resp;
        _tc->sendRequest(parsedInputJson, isBinary, m_connections[hdl], resp);
        if (isBinary)
            send(hdl, resp.c_str(), resp.size());
        else
            send(hdl, resp);
        return;
    }

    void WSThread::on_open(websocketpp::connection_hdl hdl) {

        auto open_action = [&]() {
            m_connections[hdl] = getConnectionData(hdl);
            m_connections[hdl]->sessionId = m_next_sessionid++;

            if (_tc->getTypeOfIdent() == TYPE_OF_IDENT::SIMPLE || _tc->getTypeOfIdent() == TYPE_OF_IDENT::RANDIDENT) {
                _tc->checkUser(m_connections[hdl]);
            }

            _tc->setNumOfConnections(m_connections.size());

            DEBUG_STREAM_F << "New user has been connected!! sessionId = " << m_connections[hdl]->sessionId << endl;
            DEBUG_STREAM_F << m_connections.size() << " client connected!!" << endl;
        };

        if (_tc->isServerMode()) {
            std::unique_lock<std::mutex> con_lock(m_connection_lock);
            open_action();
        }
        else {
            open_action();
        }        
    }

    void WSThread::on_close(websocketpp::connection_hdl hdl) {

        auto close_action = [&]() {
            DEBUG_STREAM_F << "User has been disconnected!!" << endl;
            delete m_connections[hdl];
            m_connections.erase(hdl);
            _tc->setNumOfConnections(m_connections.size());
            DEBUG_STREAM_F << m_connections.size() << " client connected!!" << endl;
        };

        if (_tc->isServerMode()) {
            std::unique_lock<std::mutex> con_lock(m_connection_lock);
            close_action();
        }
        else {
            close_action();
        }
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

    WSThread::~WSThread() {
        if (parsing != nullptr)
            delete parsing;
    }

    ConnectionData* WSThread::getConnectionData(websocketpp::connection_hdl hdl) {
        ConnectionData* conn_data = new ConnectionData();
        auto parsedGet = getRemoteConf(hdl);

        auto typeOfIdent = _tc->getTypeOfIdent();

        if (typeOfIdent == TYPE_OF_IDENT::SIMPLE || typeOfIdent == TYPE_OF_IDENT::RANDIDENT || typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
            // If login and password not found in GET
            if (!checkKeysFromParsedGet(parsedGet)) {
                if (typeOfIdent == TYPE_OF_IDENT::SIMPLE || typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
                    conn_data->userCheckStatus.first = false;
                    conn_data->userCheckStatus.second = "login or password or ip not found";
                }
                if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
                    conn_data->userCheckStatus.first = false;
                    conn_data->userCheckStatus.second = "login or rand_ident_hash and rand_ident or ip not found";
                }
            }
        }
        conn_data->login = parsedGet["login"];
        conn_data->ip_client = parsedGet["ip"];

        if (typeOfIdent == TYPE_OF_IDENT::SIMPLE || typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW)
        {            
            conn_data->password = parsedGet["password"];

            if (typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
                // Для аутентификации в Егоровом AuthDS в check_permissions_www
                // При подключении не проводится check_user. 
                conn_data->userCheckStatus.first = true;
            }
        }
        else if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
            conn_data->forRandIdent.rand_ident_str = parsedGet["rand_ident"];
            conn_data->forRandIdent.rand_ident_hash = parsedGet["rand_ident_hash"];
        }

        return conn_data;
    }

    bool WSThread::checkKeysFromParsedGet(const unordered_map<string, string>& parsedGet)
    {
        if (_tc->getTypeOfIdent() == TYPE_OF_IDENT::RANDIDENT)
        {
            if (parsedGet.find("login") == parsedGet.end())
                return false;

            if (parsedGet.find("ip") == parsedGet.end())
                return false;

            if (parsedGet.find("rand_ident_hash") == parsedGet.end() || parsedGet.find("rand_ident") == parsedGet.end())
                return false;
        }
        else {
            if (parsedGet.find("login") == parsedGet.end() || parsedGet.find("password") == parsedGet.end() || parsedGet.find("ip") == parsedGet.end()) {
                DEBUG_STREAM_F << "login or password or ip not found" << endl;
                return false;
            }
        }
        return true;
    }

}

