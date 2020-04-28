#ifndef EVENTPROCFORSERVERMODE_H
#define EVENTPROCFORSERVERMODE_H

#include <tango.h>
#include <string>
#include <vector>
#include <array>
#include "DeviceForWs.h"
#include "GroupForWs.h"

namespace WebSocketDS_ns
{
    class WSThread;
    class WsEvCallBackSer;

    class EventProcSer
    {
    public:
        EventProcSer(WSThread* wsThread, DeviceForWs* device, const std::array<std::vector<std::string>, 4> &event_subcr);
        EventProcSer(WSThread* wsThread, GroupForWs* group, const std::array<std::vector<std::string>, 4> &event_subcr);
        ~EventProcSer();

    private:
        log4tango::Logger *get_logger(void);
        std::vector<pair<int, WsEvCallBackSer*>> initSubscr(Tango::DeviceProxy* device, const std::array<std::vector<std::string>, 4>& event_subcr);

    private:
        WSThread* _wsThread;

        DeviceForWs* _device = nullptr;
        GroupForWs* _group = nullptr;

        std::vector<pair<int, WsEvCallBackSer*>> eventSubscrs; // For Device
        std::unordered_map<string, std::vector<pair<int, WsEvCallBackSer*>>> eventSubscrsGr;

        /**
        enum EventType {
            CHANGE_EVENT = 0,         	///< Change event
            QUALITY_EVENT,          	///< Quality change event (deprecated - do not use)
            PERIODIC_EVENT,         	///< Periodic event
            ARCHIVE_EVENT,          	///< Archive event
            USER_EVENT,             	///< User event
            ATTR_CONF_EVENT,        	///< Attribute configuration change event
            DATA_READY_EVENT,       	///< Data ready event
            INTERFACE_CHANGE_EVENT,		///< Device interface change event
            PIPE_EVENT,					///< Device pipe event
            numEventType
        };
        */
        static const Tango::EventType eventTypes[];
    };
}

#endif // EVENTPROCFORSERVERMODE_H
