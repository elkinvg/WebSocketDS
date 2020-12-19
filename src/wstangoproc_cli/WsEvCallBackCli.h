#ifndef WSEVCALLBACKCLI_H
#define WSEVCALLBACKCLI_H
#include <tango.h>

namespace WebSocketDS_ns
{
    class EventProcCli;
    class WsEvCallBackCli : public Tango::CallBack
    {
    public:
        WsEvCallBackCli(EventProcCli* evProc, const string &deviceName, const string &attrName);
        virtual ~WsEvCallBackCli();
        virtual void push_event(Tango::EventData *dt) override;

    private:
        bool _wasException{ false };
        EventProcCli* _evProc;
        string _deviceName;
        string _attrName;
    };
}

#endif // WSEVCALLBACKSER_H
