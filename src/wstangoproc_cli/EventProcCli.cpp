#include "EventProcCli.h"
#include "ParsingInputJson.h"
#include "StringProc.h"
#include "WsEvCallBackCli.h"
#include "TangoProcessor.h"
#include "WSThread.h"

#include "ResponseFromEvent.h"
#include "EnumConverter.h"

#include "ErrorInfo.h"
#include "EventReqException.h"
#include "MyEventData.h"

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

        ErrorInfo err;
        err.typeofError = ERROR_TYPE::CHECK_CODE;
        err.errorMessage = "Check code. EventProcCli::request()";
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;

        // В обычном случае не возвращается никогда
        return StringProc::exceptionStringOut(err);
    }

    void EventProcCli::sendMessage(const string & deviceName, const string & attrName, Tango::EventData * dt)
    {
        std::unique_lock<std::mutex> con_lock(m_eventproc_lock, std::defer_lock);

        if (con_lock.try_lock()) {
            map < websocketpp::connection_hdl, string, std::owner_less < websocketpp::connection_hdl>> connAddrList = eventSubs[deviceName][attrName][dt->event].connList;

            vector<websocketpp::connection_hdl> _del_conn;

            for (pair<websocketpp::connection_hdl, string> hdlnopt : connAddrList) {
                string precOpt = hdlnopt.second;

                // DONE: ДЛя каждого соединения делается копия Tango::DeviceAttribute
                // TODO: Не делать копию самого Tango::DeviceAttribute, а лишь необходимых данных?
                MyEventData eventData;
                eventData.attr_value.deep_copy(*(dt->attr_value));
                eventData.err = dt->err;
                eventData.eventType = dt->event;
                eventData.errors = dt->errors;
                eventData.tv_sec = dt->get_date().tv_sec;
                eventData.attrName = dt->attr_name;
                eventData.deviceName = dt->device->dev_name();

                string message = TangoProcessor::processEvent(eventData, precOpt);
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

        string messFromErrorResponses, messFromSuccResponses;

        if (successResponses.size()) {
            messFromSuccResponses = StringProc::responseStringOutForEventSub(
                parsedInput.id
                , parsedInput.type_req_str
                , successResponses
            );
        }

        if (errorResponses.size()) {
            ErrorInfo err;
            err.typeofReq = parsedInput.type_req_str;
            err.typeofError = ERROR_TYPE::FROM_EVENT_SUBSCR;
            err.id = parsedInput.id;
            err.errorResponses = errorResponses;
            messFromErrorResponses = StringProc::exceptionStringOut(err);

            throw EventReqException(messFromErrorResponses, messFromSuccResponses);
        }

        //if (errorResponses.size() && !successResponses.size()) {
        //    return StringProc::exceptionStringOutForEvent(
        //        ERROR_TYPE::FROM_EVENT_SUBSCR
        //        , errorResponses
        //        , parsedInput.id
        //        , parsedInput.type_req_str
        //    );
        //}

        return messFromSuccResponses;
    }

    string EventProcCli::_checkEventReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl)
    {
        //"event_type", "device", "attribute"
        const string& event_type_str = parsedInput.otherInpStr.at("event_type");
        if (
            event_type_str != "change"
            && event_type_str != "periodic"
            && event_type_str != "user"
            && event_type_str != "archive"
            )
        {
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::IS_NOT_VALID;
            err.errorMessage = "Check type of event in request";
            err.typeofReq = parsedInput.type_req_str;
            err.id = parsedInput.id;

            return StringProc::exceptionStringOut(err);
        }


        const string& deviceName = parsedInput.otherInpStr.at("device");
        const string& attribute = parsedInput.otherInpStr.at("attribute");

        std::unique_lock<std::mutex> con_lock(m_eventproc_lock);
        
        int eventId;
        
        try {
            eventId = _getIdOfEventSubscription(hdl, deviceName, attribute, event_type_str);
        }
        catch (const std::out_of_range& oor) {
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::SUBSCR_NOT_FOUND;
            err.errorMessage = "No subscription found with this id";
            err.typeofReq = parsedInput.type_req_str;
            err.device_name = deviceName;
            err.id = parsedInput.id;

            return StringProc::exceptionStringOut(err);
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
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::SUBSCR_NOT_FOUND;
            err.errorMessage = "Key event_sub_id must be a number";
            err.typeofReq = parsedInput.type_req_str;
            err.id = parsedInput.id;

            return StringProc::exceptionStringOut(err);
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
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::SUBSCR_NOT_FOUND;
            err.errorMessage = "Event subscriber with this identifier not found";
            err.typeofReq = parsedInput.type_req_str;
            err.id = parsedInput.id;

            return StringProc::exceptionStringOut(err);
        }

        return StringProc::successRespOut(parsedInput);
    }

    int EventProcCli::_getIdOfEventSubscription(websocketpp::connection_hdl hdl, const string & deviceName, const string & attribute, const string& eventType)
    {
        int eventId;

        auto &bydev = eventSubs.at(deviceName);
        auto &byattr = bydev.at(attribute);
        auto &bytype = byattr.at(eventType);

        bytype.connList.at(hdl);
        eventId = bytype.eventSubId;

        return eventId;
    }

    bool EventProcCli::_delSubHdl(websocketpp::connection_hdl hdl, TangoAttrEventType& evInfo)
    {
        UsedEventSubscr& usedEvSub = eventSubs[evInfo.deviceName][evInfo.attrName][evInfo.eventTypeString];
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
        const string& eventType = evInfo.eventTypeString;
        eventSubs[deviceName][attrName].erase(eventType);

        if (!eventSubs[deviceName][attrName].size()) {
            eventSubs[deviceName].erase(attrName);
        }

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
                eventAttrCh.eventTypeString = EnumConverter::eventTypeToString(evType);

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

    ResponseFromEventReq EventProcCli::_addCallback(websocketpp::connection_hdl hdl, const string & deviceName, const string & attrName, const string & precOpt, Tango::EventType eventType)
    {
        ResponseFromEventReq resp;
        resp.attrName = attrName;
        resp.deviceName = deviceName;
        resp.eventType = eventType;

        string eventTypeString = EnumConverter::eventTypeToString(eventType);
        resp.eventTypeStr = eventTypeString;

        UsedEventSubscr& evSubInfo = eventSubs[deviceName][attrName][eventTypeString];

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
                evInfoDev.eventTypeString = eventTypeString;

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
                    eventSubs[deviceName][attrName].erase(EnumConverter::eventTypeToString(eventType));
                    if (!eventSubs[deviceName][attrName].size()) {
                        eventSubs[deviceName].erase(attrName);
                    }
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
