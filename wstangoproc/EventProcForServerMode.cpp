#include "EventProcForServerMode.h"

#include "WSThread_plain.h"
#include "WSThread_tls.h"

#include "EventSubscr.h"
#include "StringProc.h"

namespace WebSocketDS_ns
{
    EventProcForServerMode::EventProcForServerMode(WSThread *wsThread, std::string deviceName,const std::array<std::vector<std::string>, 4> &event_subcr)
    {
        init(wsThread);
        initSubscr(deviceName, event_subcr);
    }

    EventProcForServerMode::EventProcForServerMode(WSThread* wsThread, const std::vector<std::string> &deviceNames, const std::array<std::vector<std::string>, 4> &event_subcr)
    {
        init(wsThread);

        for (const auto& dev : deviceNames) {
            initSubscr(dev, event_subcr);
        }
    }

    void EventProcForServerMode::init(WSThread* wsThread)
    {
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
        eventTypesArr[0] = Tango::EventType::CHANGE_EVENT;
        eventTypesArr[1] = Tango::EventType::PERIODIC_EVENT;
        eventTypesArr[2] = Tango::EventType::USER_EVENT;
        eventTypesArr[3] = Tango::EventType::ARCHIVE_EVENT;

        _wsThread = wsThread;
    }

    void EventProcForServerMode::initSubscr(std::string deviceName, const std::array<std::vector<std::string>, 4> &event_subcr)
    {
        bool isExcFromSubscr = false;
        vector<pair<string, vector<string>>> exceptions;

        for (unsigned int i = 0; i < event_subcr.size(); i++) {
            for (const auto &attr_name : event_subcr[i]) {
                if (!attr_name.size())
                    continue;
                try {
                    auto evSubscr = std::unique_ptr<EventSubscr>(new EventSubscr(_wsThread, deviceName, attr_name, eventTypesArr[i]));
                    eventSubscrs.push_back(std::move(evSubscr));
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
        }

        try {
            if (isExcFromSubscr)
                _wsThread->send_all(StringProc::exceptionStringOutForEvent(NONE, exceptions, "event_subscr", "exc_from_subscr"));
        }
        catch (...){}        
    }

    EventProcForServerMode::~EventProcForServerMode() {
        eventSubscrs.clear();
    }
}
