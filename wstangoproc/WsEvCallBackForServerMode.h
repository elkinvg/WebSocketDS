#ifndef WSEVCALLBACKFORSERVERMODE_H
#define WSEVCALLBACKFORSERVERMODE_H
#include <tango.h>
#include "WSThread.h"

namespace WebSocketDS_ns
{
    class TangoProcessor;
    class WsEvCallBackForServerMode : public Tango::CallBack
    {
    public:
        WsEvCallBackForServerMode(WSThread* wsThread, string &attr_name);
        ~WsEvCallBackForServerMode();
        void push_event(Tango::EventData *dt) override;
    private:
        // ??? !!! временное решение 
        // TODO
        void send_mess_all(const std::string& mess);

        WSThread* _wsThread;
        TangoProcessor *tango_proc;
    };
}

#endif // WSEVCALLBACKFORSERVERMODE_H
