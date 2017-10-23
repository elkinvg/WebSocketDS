#ifndef EVENTPROCFORSERVERMODE_H
#define EVENTPROCFORSERVERMODE_H

#include <tango.h>
#include <string>
#include <vector>
#include <array>

namespace WebSocketDS_ns
{
    class WSThread;
    class EventSubscr;
    
    class EventProcForServerMode
    {
    public:
        EventProcForServerMode(WSThread* wsThread, std::string deviceName,const std::array<std::vector<std::string>, 4> &event_subcr);
        EventProcForServerMode(WSThread* wsThread, const std::vector<std::string> &deviceNames, const std::array<std::vector<std::string>, 4> &event_subcr);
        ~EventProcForServerMode();
    private:
        void init(WSThread* wsThread);
        void initSubscr(std::string deviceName, const std::array<std::vector<std::string>, 4> &event_subcr);

    private:
        WSThread* _wsThread;
        std::array<Tango::EventType, 4> eventTypesArr;
        std::vector <unique_ptr<EventSubscr>> eventSubscrs;
};
}

#endif // EVENTPROCFORSERVERMODE_H
