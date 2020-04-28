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
        string message = TangoProcessor::processEvent(dt, _precOpt);
        send_mess(message);
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
