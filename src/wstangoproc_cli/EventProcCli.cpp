#include "EventProcCli.h"
#include "ParsingInputJson.h"
#include "StringProc.h"
#include "WsEvCallBackCli.h"
#include "TangoProcessor.h"
#include "WSThread.h"

#include "ResponseFromEvent.h"
#include "EnumConverter.h"

namespace WebSocketDS_ns {
    EventProcCli::EventProcCli(WSThread* wsThread, bool isOldVersionOfJson)
        :_wsThread(wsThread), _isOldVersionOfJson(isOldVersionOfJson)
    {
    }

    EventProcCli::~EventProcCli()
    {
    }

    string EventProcCli::request(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl)
    {
        if (parsedInput.type_req == TYPE_WS_REQ::EVENT_REQ_ADD_DEV) {
            return _addDevReq(parsedInput, hdl);
        }

        if (parsedInput.type_req == TYPE_WS_REQ::EVENT_REQ_OFF) {
            unsubscribeAllDev(hdl);
            return StringProc::successRespOut(parsedInput);
        }

        if (parsedInput.type_req == TYPE_WS_REQ::EVENT_REQ_CHECK_DEV) {
            return _checkEventReq(parsedInput, hdl);
        }

        if (parsedInput.type_req == TYPE_WS_REQ::EVENT_REQ_REM_DEV) {
            return _remDevReq(parsedInput, hdl);
        }

        return StringProc::exceptionStringOut(ERROR_TYPE::CHECK_CODE, parsedInput.id, "Check code EventProcCli::request()", parsedInput.type_req_str);
    }

    void EventProcCli::sendMessage(const string & deviceName, const string & attrName, Tango::EventData * dt)
    {
        std::unique_lock<std::mutex> con_lock(m_eventproc_lock, std::defer_lock);

        if (con_lock.try_lock()) {
            map < websocketpp::connection_hdl, string, std::owner_less < websocketpp::connection_hdl>> connAddrList = eventSubs[deviceName][attrName].connList;

            vector<websocketpp::connection_hdl> _del_conn;

            for (pair<websocketpp::connection_hdl, string> hdlnopt : connAddrList) {
                string precOpt = hdlnopt.second;
                string message = TangoProcessor::processEvent(dt, precOpt);
                try {
                    _wsThread->send(hdlnopt.first, message);
                }
                catch (...) {
                    _del_conn.push_back(hdlnopt.first);
                }
            }

            // Если выкинуто исключение, закрыть соединение со стороны сервера
            if (_del_conn.size()) {
                _wsThread->closeConnections(_del_conn);
            }
        }
        // TODO: Пока просто игнорируется
    }

    string EventProcCli::_addDevReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl)
    {
        vector<ResponseFromEventReq> errorResponses;
        vector<ResponseFromEventReq> successResponses;

        if (parsedInput.periodicEvSubList.size()) {
            _checkEventReqList(
                parsedInput
                , hdl
                , parsedInput.periodicEvSubList
                , Tango::EventType::PERIODIC_EVENT
                , successResponses
                , errorResponses
            );
        }

        if (parsedInput.changeEvSubList.size()) {
            _checkEventReqList(
                parsedInput
                , hdl
                , parsedInput.changeEvSubList
                , Tango::EventType::CHANGE_EVENT
                , successResponses
                , errorResponses
            );
        }

        if (parsedInput.archiveEvSubList.size()) {
            _checkEventReqList(
                parsedInput
                , hdl
                , parsedInput.archiveEvSubList
                , Tango::EventType::ARCHIVE_EVENT
                , successResponses
                , errorResponses
            );
        }

        if (parsedInput.userEvSubList.size()) {
            _checkEventReqList(
                parsedInput
                , hdl
                , parsedInput.userEvSubList
                , Tango::EventType::USER_EVENT
                , successResponses
                , errorResponses
            );
        }

        if (errorResponses.size() && !successResponses.size()) {
            return StringProc::exceptionStringOutForEvent(
                ERROR_TYPE::FROM_EVENT_SUBSCR
                , errorResponses
                , parsedInput.id
                , parsedInput.type_req_str
            );
        }

        return StringProc::responseStringOutForEventSub(
            parsedInput.id
            , parsedInput.type_req_str
            , successResponses
            , errorResponses
        );
    }

    string EventProcCli::_checkEventReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl)
    {
        //"event_type", "device", "attribute"
        const string& event_type_str = parsedInput.otherInpStr.at("event_type");
        Tango::EventType eventType;

        if (event_type_str == "change") {
            eventType = Tango::EventType::CHANGE_EVENT;
        } 
        else if (event_type_str == "periodic") {
            eventType = Tango::EventType::PERIODIC_EVENT;
        }
        else if (event_type_str == "user") {
            eventType = Tango::EventType::USER_EVENT;
        }
        else if (event_type_str == "archive") {
            eventType = Tango::EventType::ARCHIVE_EVENT;
        }
        else {
            return StringProc::exceptionStringOut(ERROR_TYPE::IS_NOT_VALID, parsedInput.id, "Check type of event in request", parsedInput.type_req_str);
        }

        const string& deviceName = parsedInput.otherInpStr.at("device");
        const string& attribute = parsedInput.otherInpStr.at("attribute");

        std::unique_lock<std::mutex> con_lock(m_eventproc_lock);
        
        int eventId;
        
        try {
            eventId = _getIdOfEventSubscription(hdl, deviceName, attribute, eventType);
        }
        catch (const std::out_of_range& oor) {
            return StringProc::exceptionStringOut(ERROR_TYPE::SUBSCR_NOT_FOUND, parsedInput.id, "No subscription found with this id", parsedInput.type_req_str);
        }

        string resp;

        std::stringstream ss;
        // TODO: REPLACE TO STRING PROC?
        ss << "{";
        ss << "\"event\":\"read\", ";
        ss << "\"type_req\": \"" << parsedInput.type_req_str << "\", ";
        // ID MUST BE VALUE
        try {
            auto idTmp = stoi(parsedInput.id);
            ss << " \"id_req\": " << idTmp << ",";
        }
        catch (...) {}
        ss << "\"data\": {";
        // TODO: Надо ли вводить short version
        // {data: eventId}
        //if (_isOldVersionOfJson) {

        //}
        //else {

        //}
        ss << "\"device\": \"" << deviceName << "\", ";
        ss << "\"attribute\": \"" << attribute << "\", ";
        ss << "\"event_type\": \"" << event_type_str << "\", ";
        ss << "\"event_sub_id\": \"" << eventId << "\"";
        ss << "}}";

        return ss.str();
    }

    string EventProcCli::_remDevReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl)
    {
        int eventId;

        try {
            eventId = stoi(parsedInput.otherInpStr.at("event_sub_id"));
        }
        catch (...) {
            return StringProc::exceptionStringOut(ERROR_TYPE::SUBSCR_NOT_FOUND, parsedInput.id, "Key event_sub_id must be a number", parsedInput.type_req_str);
        }

        std::unique_lock<std::mutex> con_lock(m_eventproc_lock);
        try
        {
            // TODO: CHECK/ ERROR CAN BE ONLY IN listOfId
            TangoAttrEventType info = listOfId.at(eventId);
            bool deleted = _delSubHdl(hdl, info);

            if (deleted) {
                _clearSubscrInfoMaps(info);
            }

            eventTypes[hdl][info.deviceName].erase(info.attrName);

            if (!eventTypes[hdl][info.deviceName].size()) {
                eventTypes[hdl].erase(info.deviceName);
            }

            if (!eventTypes[hdl].size()) {
                eventTypes.erase(hdl);
            }
        }
        catch (...)
        {
            return StringProc::exceptionStringOut(ERROR_TYPE::SUBSCR_NOT_FOUND, parsedInput.id, "Event subscriber with this identifier not found", parsedInput.type_req_str);
        }

        return StringProc::successRespOut(parsedInput);
    }

    int EventProcCli::_getIdOfEventSubscription(websocketpp::connection_hdl hdl, const string & deviceName, const string & attribute, const Tango::EventType & eventType)
    {
        int eventId;

        auto &bydev = eventSubs.at(deviceName);
        auto &byattr = bydev.at(attribute);

        byattr.connList.at(hdl);
        eventId = byattr.eventSubId;

        return eventId;
    }

    bool EventProcCli::_delSubHdl(websocketpp::connection_hdl hdl, TangoAttrEventType& evInfo)
    {
        UsedEventSubscr& usedEvSub = eventSubs[evInfo.deviceName][evInfo.attrName];
        usedEvSub.connList.erase(hdl);
        // Удаление, если не осталось подписчиков
        if (!usedEvSub.connList.size()) {
            try {
                usedDevices[usedEvSub.eventAttrCh.deviceName]->unsubscribe_event(usedEvSub.eventSubId);
            }
            catch (...) {}

            // Удаление из списка по ID
            listOfId.erase(usedEvSub.eventSubId);

            delete usedEvSub.evCallback;
            return true;
        }
        return false;
    }

    void EventProcCli::_clearSubscrInfoMaps(const TangoAttrEventType & evInfo)
    {
        const string& deviceName = evInfo.deviceName;
        const string& attrName = evInfo.attrName;
        eventSubs[deviceName].erase(attrName);

        if (!eventSubs[deviceName].size()) {
            eventSubs.erase(deviceName);
            delete usedDevices[deviceName];
            usedDevices.erase(deviceName);
        }
    }

    void EventProcCli::unsubscribeAllDev(websocketpp::connection_hdl hdl)
    {
        std::unique_lock<std::mutex> con_lock(m_eventproc_lock);
        auto list = eventTypes[hdl];

        if (!list.size()) {
            return;
        }

        vector<TangoAttrEventType> forDel;

        for (auto &devs : list) {
            string dev = devs.first;
            for (auto &attrs : devs.second) {
                string attr = attrs.first;
                Tango::EventType evType = attrs.second;

                TangoAttrEventType eventAttrCh;
                eventAttrCh.attrName = attr;
                eventAttrCh.deviceName = dev;
                eventAttrCh.eventType = evType;

                bool deleted = _delSubHdl(hdl, eventAttrCh);
                if (deleted) {
                    forDel.push_back(eventAttrCh);
                }
            }
        }

        // Удаление, если нет слушателей для конкретного типа события
        vector<string> rDevs;

        for (const auto &fd : forDel) {
            _clearSubscrInfoMaps(fd);
        }

        eventTypes.erase(hdl);
    }

    bool EventProcCli::_checkDeviceExist(string deviceName)
    {
        if (usedDevices.find(deviceName) == usedDevices.end()) {
            Tango::DeviceProxy* dp = new Tango::DeviceProxy(deviceName);
            usedDevices[deviceName] = dp;
            return false;
        }
        return true;
    }

    // TODO: Может давать в аргументах по ссылке вместо 
    //      const string & deviceName, const string & attrName,
    //      const string & precOpt, Tango::EventType eventType
    ResponseFromEventReq EventProcCli::_addCallback(websocketpp::connection_hdl hdl, const string & deviceName, const string & attrName, const string & precOpt, Tango::EventType eventType)
    {
        ResponseFromEventReq resp;
        resp.attrName = attrName;
        resp.deviceName = deviceName;
        resp.eventType = eventType;
        resp.eventTypeStr = EnumConverter::eventTypeToString(eventType);

        UsedEventSubscr& evSubInfo = eventSubs[deviceName][attrName];

        // Если нет Callback, значит используется впервые
        if (evSubInfo.evCallback == nullptr) {
            bool wasFailed = false;
            try
            {
                // TODO: Правильно удалить
                evSubInfo.evCallback = new WsEvCallBackCli(
                    this
                    , deviceName
                    , attrName
                );

                int eventSubId = usedDevices[deviceName]->subscribe_event(
                    attrName
                    , eventType
                    , evSubInfo.evCallback
                );

                TangoAttrEventType evInfoDev;
                evInfoDev.attrName = attrName;
                evInfoDev.deviceName = deviceName;
                evInfoDev.eventType = eventType;

                evSubInfo.eventAttrCh = evInfoDev;
                evSubInfo.eventSubId = eventSubId;

                listOfId[eventSubId] = evInfoDev;
                resp.eventSubId = eventSubId;
            }
            catch (Tango::DevFailed &e) {
                wasFailed = true;
                for (unsigned int i = 0; i < e.errors.length(); i++) {
                    resp.errorMessages.push_back((string)e.errors[i].desc);
                }
            }
            catch (...)
            {
                wasFailed = true;
                // TODO: ERROR
                resp.errorMessages = {"Unknown error"};
            }

            if (wasFailed) {
                resp.respType = RESPONSE_TYPE::ERROR_M;

                if (evSubInfo.evCallback != nullptr) {
                    delete evSubInfo.evCallback;
                }
                return resp;
            }
        }
        // Иначе проверка на существование подписки для данного hdl,
        //    на данный тип и с данными precOpt
        else {
            if (evSubInfo.connList.find(hdl) != evSubInfo.connList.end()) {
                // TODO: ERROR
                resp.respType = RESPONSE_TYPE::ADDED_EARLY;
                resp.eventSubId = evSubInfo.eventSubId;
                return resp;
            }
        }
        evSubInfo.connList[hdl] = precOpt;
        // TODO: CHECK правильное удаление
        eventTypes[hdl][deviceName][attrName] = eventType;
        resp.respType = RESPONSE_TYPE::SUCCESS;
        return resp;
    }

    void EventProcCli::_checkEventReqList(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl, const unordered_map<string, vector<string>>& evSubList, Tango::EventType eventType, vector<ResponseFromEventReq>& successResponses, vector<ResponseFromEventReq>& errorResponses)
    {
        std::unique_lock<std::mutex> con_lock(m_eventproc_lock);

        for (const auto& devNattr : evSubList) {
            ResponseFromEventReq resp;

            string deviceName = devNattr.first;
            bool wasEx = true;
            try {
                wasEx = _checkDeviceExist(deviceName);
            }
            catch (Tango::DevFailed &e)
            {
                resp.respType = RESPONSE_TYPE::ERROR_M;
                resp.deviceName = deviceName;
                
                for (unsigned int i = 0; i < e.errors.length(); i++) {
                    resp.errorMessages.push_back((string)e.errors[i].desc);
                }
                
                errorResponses.push_back(resp);

                continue;
            }
            auto attrList = devNattr.second;
            // Проверка и Удаление девайса если ни на один из атрибутов не подписался
            bool isAnythingSub = false;
            for (auto& attrName : attrList) {
                string opt = StringProc::checkPrecisionOptions(attrName, parsedInput);
                resp = _addCallback(
                    hdl
                    , deviceName
                    , attrName
                    , opt
                    , eventType
                );
                // IF FALSE - callback was not exist
                if (resp.respType == RESPONSE_TYPE::ERROR_M) {
                    eventSubs[deviceName].erase(attrName);
                    errorResponses.push_back(resp);
                }
                if (resp.respType == RESPONSE_TYPE::SUCCESS || resp.respType == RESPONSE_TYPE::ADDED_EARLY) {
                    successResponses.push_back(resp);
                }

                isAnythingSub = isAnythingSub || (resp.respType != RESPONSE_TYPE::ERROR_M);
            }

            if (!isAnythingSub && !wasEx) {
                delete usedDevices[deviceName];
                usedDevices.erase(deviceName);
            }
        }
    }
}