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
        // Во избежание отправления повторных сообщений об исключениях
        if (_wasException && dt->err) {
            return;
        }

        try
        {
            _evProc->sendMessage(
                _deviceName
                , _attrName
                , dt
            );
        }
        catch (...) {}

        if (!_wasException && dt->err) {
            _wasException = true;
        }
        if (_wasException && !dt->err) {
            _wasException = false;
        }
    }
}
