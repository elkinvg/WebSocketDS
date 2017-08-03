#ifndef WSEVCALLBACK_H
#define WSEVCALLBACK_H

#include <tango.h>
#include "WSThread.h"

namespace WebSocketDS_ns
{
    class TangoProcessor;
    class WsEvCallBack : public Tango::CallBack
    {
    public:
        WsEvCallBack(websocketpp::connection_hdl hdl, WSThread* wsThread, string &attr_name);
        ~WsEvCallBack();
        
        void push_event(Tango::EventData *dt) override;
    private:
        websocketpp::connection_hdl _hdl;
        WSThread* _wsThread;
        TangoProcessor *tango_proc;
    };
}

#endif // WSEVCALLBACK_H
