#ifndef WSEVCALLBACKSER_H
#define WSEVCALLBACKSER_H
#include <tango.h>
#include "WSThread.h"
#include "UserOptions.h"

namespace WebSocketDS_ns
{
    class WsEvCallBackSer : public Tango::CallBack
    {
    public:
        WsEvCallBackSer(WSThread* wsThread, const UserOptions& userOpt);
        virtual ~WsEvCallBackSer();
        void push_event(Tango::EventData *dt) override;
    private:
        void send_mess(const std::string& mess);

    private:
        bool _wasException{ false };
        WSThread* _wsThread;
        UserOptions _userOpt;
    };
}

#endif // WSEVCALLBACKSER_H
