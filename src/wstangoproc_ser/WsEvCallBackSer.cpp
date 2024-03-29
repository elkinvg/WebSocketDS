#include "WsEvCallBackSer.h"
#include "StringProc.h"
#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    WsEvCallBackSer::WsEvCallBackSer(WSThread *wsThread, const UserOptions& userOpt)
        :_wsThread(wsThread), _userOpt(userOpt)
    {
    }

    void WsEvCallBackSer::push_event(Tango::EventData *dt) {
        // Во избежание отправления повторных сообщений об исключениях
        if (_wasException && dt->err) {
            return;
        }

        MyEventData eventData(dt);
        eventData.err = dt->err;
        eventData.eventType = dt->event;
        eventData.errors = dt->errors;
        eventData.tv_sec = dt->get_date().tv_sec;
        eventData.deviceName = dt->device->dev_name();

        string message = TangoProcessor::processEvent(eventData, _userOpt);
        send_mess(message);

        if (!_wasException && dt->err) {
            _wasException = true;
        }
        if (_wasException && !dt->err) {
            _wasException = false;
        }
    }

    WsEvCallBackSer::~WsEvCallBackSer()
    {
    }

    void WsEvCallBackSer::send_mess(const std::string& mess)
    {
        try {
            _wsThread->send_all(mess);
        }
        catch (...) {}
    }
}
