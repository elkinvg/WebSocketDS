#include "EventProc.h"

#include "WSThread_plain.h"
#include "WSThread_tls.h"

////#include "WSThread.h"
//#include "tango.h"
#include "ParsingInputJson.h"
#include "EventSubscr.h"
#include "StringProc.h"

namespace WebSocketDS_ns
{
    EventProc::EventProc(websocketpp::connection_hdl hdl, WSThread *wsThread)
    {
        _wsThread = wsThread;
        _hdl = hdl;
        parsing = new ParsingInputJson();

        //enum EventType {
        //    CHANGE_EVENT = 0,         	///< Change event
        //    QUALITY_EVENT,          	///< Quality change event (deprecated - do not use)
        //    PERIODIC_EVENT,         	///< Periodic event
        //    ARCHIVE_EVENT,          	///< Archive event
        //    USER_EVENT,             	///< User event
        //    ATTR_CONF_EVENT,        	///< Attribute configuration change event
        //    DATA_READY_EVENT,       	///< Data ready event
        //    INTERFACE_CHANGE_EVENT,		///< Device interface change event
        //    PIPE_EVENT,					///< Device pipe event
        //    numEventType
        //};

        eventTypes["change"] = Tango::EventType::CHANGE_EVENT;
        eventTypes["periodic"] = Tango::EventType::PERIODIC_EVENT;
        eventTypes["user"] = Tango::EventType::USER_EVENT;

        eventTypes["archive"] = Tango::EventType::ARCHIVE_EVENT;
    }

    void EventProc::sendRequest(const ParsedInputJson & parsedJson)
    {
        if (parsedJson.type_req == "eventreq_add_dev")
            addDevice(parsedJson);
        if (parsedJson.type_req == "eventreq_off") {
            eventSubscrs.clear();
            eventNames.clear();
            eventIt.clear();
            _wsThread->send(_hdl, StringProc::responseStringOut(parsedJson.id, "All devices from the subscription have been deleted", parsedJson.type_req, true));
            return;
        }
        if (parsedJson.type_req == "eventreq_rem_dev")
            removeDevice(parsedJson);
        if (parsedJson.type_req == "eventreq_check_dev") {
            if (parsedJson.check_key("event_type") != TYPE_OF_VAL::VALUE
                || parsedJson.check_key("device") != TYPE_OF_VAL::VALUE
                || parsedJson.check_key("attribute") != TYPE_OF_VAL::VALUE) {
                _wsThread->send(_hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Key event_type or device or attribute not found. Check input JSON", parsedJson.type_req));
                return;
            }
            string tmpAttrName = parsedJson.otherInpStr.at("attribute");
            string deviceName = parsedJson.otherInpStr.at("device");
            string eventType = parsedJson.otherInpStr.at("event_type");
            StringProc::parseInputString(tmpAttrName, ";");
            string key = tmpAttrName + "|" + deviceName + "|" + eventType;
            
            int gettedEventIt = -1;
            if (eventIt.find(key) != eventIt.end())
                gettedEventIt = eventIt.at(key);

            string resp = "{\"device\": \"" + deviceName + "\", \"attribute\": \"" + tmpAttrName + "\", \"event_type\": \"" + eventType + "\", \"event_id\": " + to_string(gettedEventIt) + "}";

            _wsThread->send(_hdl, StringProc::responseStringOut(parsedJson.id, resp, parsedJson.type_req, false));
        }
    }

    void EventProc::addDevice(const ParsedInputJson & parsedJson)
    {
        vec_event_inf eventDevInp;

        if (parsedJson.check_key("periodic") == TYPE_OF_VAL::OBJECT)
            parsing->getEventDevInp(parsedJson.otherInpObj.at("periodic"), eventDevInp, "periodic");
        if (parsedJson.check_key("change") == TYPE_OF_VAL::OBJECT)
            parsing->getEventDevInp(parsedJson.otherInpObj.at("change"), eventDevInp, "change");
        if (parsedJson.check_key("user") == TYPE_OF_VAL::OBJECT)
            parsing->getEventDevInp(parsedJson.otherInpObj.at("user"), eventDevInp, "user");
        // ??? !!! not verified
        if (parsedJson.check_key("archive") == TYPE_OF_VAL::OBJECT)
            parsing->getEventDevInp(parsedJson.otherInpObj.at("archive"), eventDevInp, "archive");

        if (!eventDevInp.size()) {
            _wsThread->send(_hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Device list not found. Check input JSON", parsedJson.type_req));
            return;
        }

       
        bool isNotAlias = false;
        vector<string> notAliasDevs;

        bool isAlreadyPushed = false;
        vector<string> alreadyPushedDevice;

        bool isExcFromSubscr = false;
        vector<pair<string, vector<string>>> exceptions;

        bool isOk = false;
        vector<pair<int, tuple<string, string, string>*>> pushedEv;
        
        for (auto &ev : eventDevInp) {
            auto deviceName = std::get<0>(ev);
            auto attributeName = std::get<1>(ev);
            string tmpAttrName = attributeName;
            auto eventType = std::get<2>(ev);
            bool isNameAlias = StringProc::isNameAlias(deviceName);

            if (_wsThread->isAliasMode() && !isNameAlias) {
                isNotAlias = true;
                notAliasDevs.push_back(deviceName);
                continue;
            }
            if (isNameAlias)
                getDeviceNameFromAlias(deviceName);
            // tmpAttrName введён для того, чтобы поместить в key имя атрибута без опций
            StringProc::parseInputString(tmpAttrName, ";");
            string key = tmpAttrName + "|" + deviceName + "|" + eventType;
            if (eventSubscrs.find(key) != eventSubscrs.end()) {
                isAlreadyPushed = true;
                alreadyPushedDevice.push_back(deviceName + "/" + attributeName);
                continue;
            }
            try {
                int eventId;
                eventSubscrs[key] = unique_ptr<EventSubscr>(new EventSubscr(_hdl, _wsThread, deviceName, attributeName, eventTypes[eventType], eventId));
                isOk = true;
                pushedEv.push_back(make_pair(eventId, &ev));
                eventNames[eventId] = key;
                eventIt[key] = eventId;
            }
            catch (Tango::DevFailed &e) {
                vector<string> errs;
                for (int i = 0; i < e.errors.length(); i++) {
                    errs.push_back((string)e.errors[i].desc);
                }
                isExcFromSubscr = true;
                exceptions.push_back(make_pair(deviceName, errs));
            }
        }
        if (isNotAlias)
            _wsThread->send(_hdl, StringProc::exceptionStringOutForEvent(parsedJson.id, notAliasDevs, parsedJson.type_req, "not_aliases"));

        if (isAlreadyPushed)
            _wsThread->send(_hdl, StringProc::exceptionStringOutForEvent(parsedJson.id, alreadyPushedDevice, parsedJson.type_req, "already_pushed"));

        if (isExcFromSubscr)
            _wsThread->send(_hdl, StringProc::exceptionStringOutForEvent(parsedJson.id, exceptions, parsedJson.type_req, "exc_from_subscr"));
        
        if (isOk) {
            stringstream ss;
            ss << "[";

            bool nfst = false;
            for (auto& listev : pushedEv) {
                if (nfst)
                    ss << ", ";
                else
                    nfst = true;
                ss << "{";
                ss << "\"device\": \"" << std::get<0>(*listev.second) << "\", ";
                ss << "\"attribute\": \"" << std::get<1>(*listev.second) << "\", ";
                ss << "\"event_type\": \"" << std::get<2>(*listev.second) << "\", ";
                ss << "\"event_id\": " << listev.first;
                ss << "}";
            }
            ss << "]";
            _wsThread->send(_hdl, StringProc::responseStringOut(parsedJson.id, ss.str(), parsedJson.type_req, false));
        }        
    }

    void EventProc::removeDevice(const ParsedInputJson & parsedJson) {
        if (parsedJson.check_key("event_id") != TYPE_OF_VAL::VALUE) {
            _wsThread->send(_hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Key event_id not found or it isn't string|int", parsedJson.type_req));
            return;
        }
        int event_id;
        try {
            event_id = std::stoi(parsedJson.otherInpStr.at("event_id"));
        }
        catch (...) {
            _wsThread->send(_hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "The key event_id must be an integer type", parsedJson.type_req));
            return;
        }
        if (eventNames.find(event_id) == eventNames.end()) {
            _wsThread->send(_hdl, StringProc::exceptionStringOut(parsedJson.id, NONE, "Event subscriber with event_id " + to_string(event_id) + " not found", parsedJson.type_req));
            return;
        }
        eventSubscrs.erase(eventNames[event_id]);
        _wsThread->send(_hdl, StringProc::responseStringOut(parsedJson.id, "Event subscriber with event_id " + to_string(event_id) + " deleted", parsedJson.type_req, true));
        eventIt.erase(eventNames[event_id]);
        eventNames.erase(event_id);
    }

    void EventProc::getDeviceNameFromAlias(string& alias) {
        string device_name_from_alias;
        try {
            Tango::Database *db = Tango::Util::instance()->get_database();
            db->get_device_alias(alias, device_name_from_alias);
            alias = device_name_from_alias;
        }
        catch (Tango::DevFailed& e) {}
    }
    
    EventProc::~EventProc()
    {
        delete parsing;
    }
}