#include "EventSubscr.h"

namespace WebSocketDS_ns
{
    EventSubscr::EventSubscr(websocketpp::connection_hdl hdl, WSThread* wsThread, string deviceName, string attrName, Tango::EventType eventType, int& eventId)
    {
        logger = wsThread->logger;
        //wsEvCallBacks = std::unique_ptr<WsEvCallBack>(new WsEvCallBack(hdl, wsThread, attrName));
        wsEvCallBacks = new WsEvCallBack(hdl, wsThread, attrName);
        eventDev = std::unique_ptr<Tango::DeviceProxy>(new Tango::DeviceProxy(deviceName));
        _deviceName = deviceName;
        _attrName = attrName;
        _eventType = eventType;
        _eventId = eventDev->subscribe_event(_attrName, _eventType, wsEvCallBacks);
        eventId = _eventId;
        DEBUG_STREAM_F << "subscribe_event with id :" << _eventId << endl;
    }

    EventSubscr::EventSubscr(WSThread* wsThread, string deviceName, string attrName, Tango::EventType eventType)
    {
        logger = wsThread->logger;
        //wsEvCallBacksSer = std::unique_ptr<WsEvCallBackForServerMode>( new WsEvCallBackForServerMode(wsThread, attrName));
        wsEvCallBacksSer = new WsEvCallBackForServerMode(wsThread, attrName);
        eventDev = std::unique_ptr<Tango::DeviceProxy>(new Tango::DeviceProxy(deviceName));
        _deviceName = deviceName;
        _attrName = attrName;
        _eventType = eventType;
        _eventId = eventDev->subscribe_event(_attrName, _eventType, wsEvCallBacksSer);
        DEBUG_STREAM_F << "subscribe_event with id :" << _eventId << endl;
    }
    
    EventSubscr::~EventSubscr()
    {
        DEBUG_STREAM_F << "unsubscribe_event with id :" << _eventId << endl;
        if (wsEvCallBacksSer != nullptr)
            delete wsEvCallBacksSer;
        if (wsEvCallBacks != nullptr)
            delete wsEvCallBacks;
        eventDev->unsubscribe_event(_eventId);
    }
    
    EventSubscr::EventSubscr(){}
    
}
