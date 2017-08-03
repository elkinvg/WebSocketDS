#include "EventSubscr.h"

namespace WebSocketDS_ns
{
    EventSubscr::EventSubscr(websocketpp::connection_hdl hdl, WSThread* wsThread, string deviceName, string attrName, Tango::EventType eventType, int& eventId)
    {
        wsEvCallBacks = new WsEvCallBack(hdl, wsThread, attrName);
        eventDev = new Tango::DeviceProxy(deviceName);
        _deviceName = deviceName;
        _attrName = attrName;
        _eventType = eventType;
        _eventId = eventDev->subscribe_event(_attrName, _eventType, wsEvCallBacks);
        eventId = _eventId;
    }
    
    EventSubscr::~EventSubscr()
    {
        eventDev->unsubscribe_event(_eventId);
    }
    
    EventSubscr::EventSubscr(){}
    
}
