#include "WsEvCallBackCli.h"
#include "EventProcCli.h"

namespace WebSocketDS_ns
{
    WsEvCallBackCli::WsEvCallBackCli(EventProcCli* evProc, const string & deviceName, const string & attrName)
        :_evProc(evProc)
        ,_deviceName(deviceName)
        , _attrName(attrName)
    {
    }

    WsEvCallBackCli::~WsEvCallBackCli()
    {
    }

    void WsEvCallBackCli::push_event(Tango::EventData * dt)
    {
        try
        {
            _evProc->sendMessage(
                _deviceName
                , _attrName
                , dt
            );
        }
        catch (...) {}
    }
}
