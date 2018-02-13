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

        if (parsedInputJson.type_req.find("timer") != string::npos ) {
            if (_tc->getMode() == MODE::SERVER) {
                string errorMess = StringProc::exceptionStringOut(parsedInputJson.id, NONE, "Timer is not available in the current mode", parsedInputJson.type_req);
                send(hdl,errorMess);
            }
            else
                timerProc(parsedInputJson, hdl);
            return;
        }
        
        // Действия с событиями
        // eventreq_add_dev - добавление девайсов в подписки
        if (parsedInputJson.type_req.find("eventreq") != string::npos ) {
            if (_tc->getMode() == MODE::SERVER) {
                string errorMess = StringProc::exceptionStringOut(parsedInputJson.id, NONE, "Subscribing to events is not supported in the current mode", parsedInputJson.type_req);
                send(hdl,errorMess);
            }
            else {
                if (m_connections[hdl].eventProc == nullptr)
                    m_connections[hdl].eventProc = unique_ptr<EventProc>(new EventProc(hdl, this));
                m_connections[hdl].eventProc->sendRequest(parsedInputJson);
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
        if (_tc->isServerMode())
            websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);

        m_connections[hdl] = std::move(getConnectionData(hdl));
        m_connections[hdl].sessionId = m_next_sessionid++;

        if (_tc->getTypeOfIdent() == TYPE_OF_IDENT::SIMPLE || _tc->getTypeOfIdent() == TYPE_OF_IDENT::RANDIDENT) {
            _tc->checkUser(m_connections[hdl]);
        }

        _tc->setNumOfConnections(m_connections.size());
        
        DEBUG_STREAM_F << "New user has been connected!! sessionId = " << m_connections[hdl].sessionId << endl;
        DEBUG_STREAM_F << m_connections.size() << " client connected!!" << endl;
    }

    void WSThread::on_close(websocketpp::connection_hdl hdl) {
        DEBUG_STREAM_F << "User has been disconnected!!" << endl;
        if (_tc->isServerMode())
            websocketpp::lib::unique_lock<websocketpp::lib::mutex> con_lock(m_connection_lock);
        m_connections.erase(hdl);
        _tc->setNumOfConnections(m_connections.size());
        DEBUG_STREAM_F << m_connections.size() << " client connected!!" << endl;
    }

    void  WSThread::on_fail(websocketpp::connection_hdl hdl) {
        ERROR_STREAM_F << " Fail from WSThread on_fail " << endl;
    }

    void WSThread::timerProc(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl)
    {
        if (parsedJson.type_req == "timer_start"
                || parsedJson.type_req == "timer_change") {
            if (parsedJson.check_key("msec") != TYPE_OF_VAL::VALUE) {
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Check keys for Timer (msec)", parsedJson.type_req));
                return;
            }
        }

        if (parsedJson.type_req == "timer_stop"
            || parsedJson.type_req == "timer_remove_devs"
            || parsedJson.type_req == "timer_add_devs"
            || parsedJson.type_req == "timer_upd_devs_add"
            || parsedJson.type_req == "timer_upd_devs_rem"
            || parsedJson.type_req == "timer_check"
            || parsedJson.type_req == "timer_change")
        {
            if (m_connections[hdl].timing == nullptr) {
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Timer not yet active", parsedJson.type_req));
                return;
            }
        }

        dev_attr_pipe_map devAttrPipeMap;

        bool hasDevices = (parsedJson.check_key("devices") == TYPE_OF_VAL::OBJECT);
        bool hasGroups = (parsedJson.check_key("group") == TYPE_OF_VAL::OBJECT);

        if (parsedJson.type_req == "timer_start"
                || parsedJson.type_req == "timer_add_devs"
                || parsedJson.type_req == "timer_upd_devs_add"
                || parsedJson.type_req == "timer_upd_devs_rem")
        {
            if (
                !hasDevices
                && !hasGroups
                ) 
            {
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Check keys for Timer (devices or group) ", parsedJson.type_req));
                return;
            }
            
            auto ws_mode = _tc->getMode();
            if (isAliasMode())
            {
                if (parsedJson.check_key("group") == TYPE_OF_VAL::OBJECT)
                {
                    send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "In the current mode only aliases are used", parsedJson.type_req));
                    return;
                }

                for (const auto &devs : parsedJson.otherInpObj.at("devices")) {
                    if (!StringProc::isNameAlias(devs.first)) {
                        send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "In the current mode only aliases are used", parsedJson.type_req));
                        return;
                    }
                }
            }

            if (hasDevices)
                devAttrPipeMap = parsing->getListDevicesAttrPipe(parsedJson.otherInpObj.at("devices"));

            dev_attr_pipe_map group_map;

            if (hasGroups) {
                string errorMessage;
                group_map = GroupForWs::getListDevicesFromGroupForAttrAndPipeProc(parsing->getListDevicesAttrPipe(parsedJson.otherInpObj.at("group")), errorMessage);

                if (errorMessage.size())
                    send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, errorMessage, parsedJson.type_req));
                
                if (!hasDevices && !group_map.size())
                    return;
                
                devAttrPipeMap.insert(group_map.begin(), group_map.end());
            }
        }

        // В режимах    MODE::SERVNCLIENT_ALIAS MODE::CLIENT_ALIAS 
        //              MODE::CLIENT_ALIAS_RO MODE::SERVNCLIENT_ALIAS_RO
        // Могут использоваться только псевдонимы      

        if (parsedJson.type_req == "timer_start")
            return timerStartMeth(parsedJson, hdl, devAttrPipeMap);

        if (parsedJson.type_req == "timer_stop")
            return timerStopMeth(parsedJson, hdl);

        if (parsedJson.type_req == "timer_change")
            return timerChangeMeth(parsedJson, hdl);

        if (parsedJson.type_req == "timer_add_devs")
            return timerAddDevsMeth(parsedJson, hdl, devAttrPipeMap);

        if (parsedJson.type_req == "timer_remove_devs")
            return timerRemDevsMeth(parsedJson, hdl);

        if (parsedJson.type_req == "timer_upd_devs_add")
            return timerUpdDevsAddMeth(parsedJson, hdl, devAttrPipeMap);

        if (parsedJson.type_req == "timer_upd_devs_rem")
            return timerUpdDevsRemMeth(parsedJson, hdl, devAttrPipeMap);

        if (parsedJson.type_req == "timer_check") 
            return timerCheckMeth(parsedJson, hdl);
        
        send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "This request type is not supported", parsedJson.type_req));
    }

    void WSThread::timerStartMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl, dev_attr_pipe_map &devAttrPipeMap)
    {
        if (m_connections[hdl].timing != nullptr)
        {
            string errMess = "Timer already started. The timer period is " + to_string(m_connections[hdl].timing->msec) + " seconds";
            send(hdl, StringProc::exceptionStringOut(errMess, "timer_start"));
            DEBUG_STREAM_F << errMess;
            return;
        }

        if (!devAttrPipeMap.size()) {
            send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "No devices found. Check keys", parsedJson.type_req));
            return;
        }

        try {
            auto msec = stoi(parsedJson.otherInpStr.at("msec"));
            int tmin;
            if (_tc->isTm100ms())
                tmin = 100;
            else
                tmin = 1000;
            if (msec >= tmin) {
                m_connections[hdl].timing = unique_ptr<TimingStruct>(new TimingStruct());
                m_connections[hdl].timing->msec = msec;
            }
            else {
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "The timer period must be greater than or equal to " + to_string(tmin) + " milliseconds.", parsedJson.type_req));
                return;
            }
        }
        catch (...){
            send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "The timer period must be a number", parsedJson.type_req));
            return;
        }

        if (m_connections[hdl].tangoConnForClient == nullptr)
            m_connections[hdl].tangoConnForClient = unique_ptr<TangoConnForClient>(new TangoConnForClient(devAttrPipeMap));
        else
            m_connections[hdl].tangoConnForClient->addDevicesToUpdateList(devAttrPipeMap);

        startTimer(hdl);

        // Если таймер не запустился
        if (m_connections[hdl].timing == nullptr) {
            send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Timer does not start", parsedJson.type_req));
            return;
        }

        // Сообщение об успешном запуске таймера
        send(hdl, StringProc::responseStringOut(parsedJson.id, "Started timer in session", parsedJson.type_req, true));

        m_connections[hdl].timing->isTimerOn = true;
        DEBUG_STREAM_F << "Started timer in session with id " << m_connections[hdl].sessionId;
    }

    void WSThread::timerStopMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl)
    {
        m_connections[hdl].tangoConnForClient->removeAllDevices();
        m_connections[hdl].timing.reset(nullptr);

        // Сообщение об успешной остановке таймера
        send(hdl, StringProc::responseStringOut(parsedJson.id, "Stopped timer in session", parsedJson.type_req, true));
    }

    void WSThread::timerChangeMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl)
    {
        try {
            auto msec = stoi(parsedJson.otherInpStr.at("msec"));
            if (msec > 1000) {
                m_connections[hdl].timing->msec = msec;
                // Сообщение об успешном обновлении периода таймера
                send(hdl, StringProc::responseStringOut(parsedJson.id, "New timer period is " + to_string(msec) + " msec", parsedJson.type_req, true));
            }
            else
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "The timer period must be longer than 1000 milliseconds.", parsedJson.type_req));
        }
        catch (...){
            send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "The timer period must be a number", parsedJson.type_req));
        }
    }

    void WSThread::timerAddDevsMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl, dev_attr_pipe_map &devAttrPipeMap)
    {
        pair<string, string> errors = m_connections[hdl].tangoConnForClient->addDevicesToUpdateList(devAttrPipeMap);
        if (errors.first.size() && errors.second.size()) {
            if (errors.first.size() || errors.second.size()) {
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, errors, parsedJson.type_req));
                return;
            }

            if (errors.first.size())
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, errors.first, parsedJson.type_req));
            if (errors.second.size())
                send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, errors.second, parsedJson.type_req));
        }
        else {
            send(hdl, StringProc::responseStringOut(parsedJson.id, "All devices from the list have been added", parsedJson.type_req));
        }
    }

    void WSThread::timerRemDevsMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl)
    {
        bool hasDev = false;
        bool hasGr = false;
        string keyDev = "devices";
        vector<string> devices_for_rem;

        if (parsedJson.check_key(keyDev) == TYPE_OF_VAL::ARRAY ||
            parsedJson.check_key(keyDev) == TYPE_OF_VAL::VALUE)
        {
            if (parsedJson.check_key(keyDev) == TYPE_OF_VAL::ARRAY)
                devices_for_rem = parsedJson.otherInpVec.at(keyDev);

            if (parsedJson.check_key(keyDev) == TYPE_OF_VAL::VALUE)
                devices_for_rem.push_back(parsedJson.otherInpStr.at(keyDev));
            hasDev = true;
        }
            

        if (parsedJson.check_key("group") == TYPE_OF_VAL::VALUE) 
        {
            string errorMessage;
            auto devsFromGroup = GroupForWs::getArrayOfDevicesFromGroup(parsedJson.otherInpStr.at("group"), errorMessage);
            if (!errorMessage.size() && devsFromGroup.size()) {
                hasGr = true;
                devices_for_rem.insert(devices_for_rem.begin(), devsFromGroup.begin(), devsFromGroup.end());
            }            
        }

        if (!hasDev && !hasGr) {
            send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Check keys for Timer. Key devices or group has an incorrect format. Key devices must be a vector or value. Key group must be value", parsedJson.type_req));
            return;
        }

        int numOfDevsPre = m_connections[hdl].tangoConnForClient->numOfListeningDevices();
        string outMess = m_connections[hdl].tangoConnForClient->removeDevicesFromUpdateList(devices_for_rem);


        if (outMess.size())
            send(hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, outMess, parsedJson.type_req));

        if (!m_connections[hdl].tangoConnForClient->numOfListeningDevices()) 
        {
            send(hdl, StringProc::responseStringOut(parsedJson.id, "All devices from the input list have been removed. Total Number of Listening Devices is 0. The timer will be off", parsedJson.type_req));
            m_connections[hdl].timing.reset(nullptr);
            return;
        }


        if (m_connections[hdl].tangoConnForClient->numOfListeningDevices() < numOfDevsPre)
            send(hdl, StringProc::responseStringOut(parsedJson.id, "All devices from the input list have been removed", parsedJson.type_req));
    }

    void WSThread::timerUpdDevsAddMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl, dev_attr_pipe_map &devAttrPipeMap)
    {
        vector<string> outMessages = m_connections[hdl].tangoConnForClient->addAttrToDevicesFromUpdatelist(devAttrPipeMap);
        send(hdl, StringProc::responseStringOut(parsedJson.id, outMessages, parsedJson.type_req));
    }

    void WSThread::timerUpdDevsRemMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl, dev_attr_pipe_map &devAttrPipeMap)
    {
        vector<string> outMessages = m_connections[hdl].tangoConnForClient->remAttrToDevicesFromUpdatelist(devAttrPipeMap);
        send(hdl, StringProc::responseStringOut(parsedJson.id, outMessages, parsedJson.type_req));
    }

    void WSThread::timerCheckMeth(const ParsedInputJson &parsedJson, websocketpp::connection_hdl hdl) {
        bool timerStatus = false;
        std::stringstream json;
        json << "{\"event\": \"read\", \"type_req\": \"timer_check\", ";
        
        std::string id = parsedJson.id;
        try {
            auto idTmp = stoi(id);
            json << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            if (id == NONE)
                json << "\"id_req\": " << id << ", ";
            else
                json << "\"id_req\": \"" << id << "\", ";
        }

        json << "\"data\":{";
        
        json << "\"timer_started\": ";
        if (m_connections[hdl].timing == nullptr) {
            json << boolalpha << false;
            json << "}}";
            send(hdl, json.str());
            return;
        }
        json << boolalpha << true << ", ";
        json << "\"timer_period\": " << m_connections[hdl].timing->msec;
        json << "}}";
        send(hdl, json.str());
    }

    bool WSThread::forRunTimer(websocketpp::connection_hdl hdl, int timerInd)
    {
        DEBUG_STREAM_F << "timer. SessionId = " << m_connections[hdl].sessionId;

        DEBUG_STREAM_F << " ___ " << m_connections[hdl].timerInd  << " ___ " << timerInd;
        if (m_connections[hdl].timerInd != timerInd){
            DEBUG_STREAM_F << "Timer " << m_connections[hdl].timerInd <<  " was stopped";
            return false;
        }
        timerInd = m_connections[hdl].timerInd;
        bool hasDevice;
        string resp = m_connections[hdl].tangoConnForClient->getJsonForAttribute(hasDevice);
        send(hdl, resp);
        // Этот метод не должен возвращать false в нормальной ситуации, так как
        // наличие девайса проверяется при запуске таймера.

        if (!hasDevice)
            return false;
        if (hdl.expired())
            return false;
        else
            return true;
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

    ConnectionData WSThread::getConnectionData(websocketpp::connection_hdl hdl) {
        ConnectionData conn_data;

        auto parsedGet = getRemoteConf(hdl);

        auto typeOfIdent = _tc->getTypeOfIdent();

        if (typeOfIdent == TYPE_OF_IDENT::SIMPLE || typeOfIdent == TYPE_OF_IDENT::RANDIDENT || typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
            // If login and password not found in GET
            if (!checkKeysFromParsedGet(parsedGet)) {
                if (typeOfIdent == TYPE_OF_IDENT::SIMPLE || typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
                    conn_data.userCheckStatus.first = false;
                    conn_data.userCheckStatus.second = "login or password or ip not found";
                }
                if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
                    conn_data.userCheckStatus.first = false;
                    conn_data.userCheckStatus.second = "login or rand_ident_hash and rand_ident or ip not found";
                }
            }
        }
        conn_data.login = parsedGet["login"];
        conn_data.ip_client = parsedGet["ip"];

        if (typeOfIdent == TYPE_OF_IDENT::SIMPLE || typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW)
        {            
            conn_data.password = parsedGet["password"];
        }
        else if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
            conn_data.forRandIdent.rand_ident_str = parsedGet["rand_ident"];
            conn_data.forRandIdent.rand_ident_hash = parsedGet["rand_ident_hash"];
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

