#ifndef WSEVCALLBACKSER_H
#define WSEVCALLBACKSER_H
#include <tango.h>
#include "WSThread.h"

namespace WebSocketDS_ns
{
    class WsEvCallBackSer : public Tango::CallBack
    {
    public:
        WsEvCallBackSer(WSThread* wsThread, const string& precOpt);
        virtual ~WsEvCallBackSer();
        void push_event(Tango::EventData *dt) override;
    private:
        void send_mess(const std::string& mess);

    private:
        bool _wasException{ false };
        WSThread* _wsThread;
        string _precOpt;
    };
}

#endif // WSEVCALLBACKSER_H
