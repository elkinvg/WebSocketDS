#ifndef EVENTSUBSCR_H
#define EVENTSUBSCR_H

#include "WsEvCallBack.h"
#include "WsEvCallBackForServerMode.h"

namespace WebSocketDS_ns
{
    class EventSubscr
    {
        public:
            EventSubscr();
            EventSubscr(websocketpp::connection_hdl hdl, WSThread* wsThread, string deviceName, string attrName, Tango::EventType eventType, int& eventId);
            EventSubscr(WSThread* wsThread, string deviceName, string attrName, Tango::EventType eventType);
            ~EventSubscr();
        private:
            //unique_ptr<WsEvCallBack> wsEvCallBacks = nullptr;
            log4tango::Logger *logger;
            WsEvCallBack* wsEvCallBacks = nullptr;
            //unique_ptr<WsEvCallBackForServerMode> wsEvCallBacksSer = nullptr;
            WsEvCallBackForServerMode* wsEvCallBacksSer = nullptr;
            unique_ptr<Tango::DeviceProxy> eventDev = nullptr;
            string _deviceName;
            string _attrName;
            Tango::EventType _eventType;
            int _eventId;
        };
    }

#endif // EVENTSUBSCR_H
