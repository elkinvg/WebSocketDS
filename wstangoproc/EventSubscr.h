#ifndef EVENTSUBSCR_H
#define EVENTSUBSCR_H

#include "WsEvCallBack.h"
namespace WebSocketDS_ns
{
    class EventSubscr
    {
        public:
            EventSubscr();
            EventSubscr(websocketpp::connection_hdl hdl, WSThread* wsThread, string deviceName, string attrName, Tango::EventType eventType, int& eventId);
            ~EventSubscr();
        private:
            WsEvCallBack* wsEvCallBacks = nullptr;
            Tango::DeviceProxy* eventDev = nullptr;
            string _deviceName;
            string _attrName;
            Tango::EventType _eventType;
            int _eventId;
        };
    }

#endif // EVENTSUBSCR_H
