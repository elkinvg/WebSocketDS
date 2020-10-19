#include "WsEvCallBackSer.h"
#include "StringProc.h"
#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    WsEvCallBackSer::WsEvCallBackSer(WSThread *wsThread, const string& precOpt)
        :_wsThread(wsThread), _precOpt(precOpt)
    {
    }

    void WsEvCallBackSer::push_event(Tango::EventData *dt) {
        // Во избежание отправления повторных сообщений об исключениях
        if (_wasException && dt->err) {
            return;
        }

        string message = TangoProcessor::processEvent(dt, _precOpt);
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
