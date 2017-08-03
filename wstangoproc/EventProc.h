#ifndef EVENTPROC_H
#define EVENTPROC_H

#include <tango.h>
#include <websocketpp/server.hpp>

#include <unordered_map>

namespace WebSocketDS_ns
{
    class WSThread;
    class ParsedInputJson;
    class ParsingInputJson;
    class EventSubscr;
    class EventProc
    {
    public:
        EventProc(websocketpp::connection_hdl hdl, WSThread* wsThread);        
        void sendRequest(const ParsedInputJson & parsedJson);
        ~EventProc();
    private:
        void subscribeToEvents(string deviceName, string attrName, Tango::EventType eventType);
        void addDevice(const ParsedInputJson & parsedJson);
        void removeDevice(const ParsedInputJson & parsedJson);
        void getDeviceNameFromAlias(string& alias);
    private:
        WSThread* _wsThread;
        websocketpp::connection_hdl _hdl;
        std::unordered_map<string, unique_ptr<EventSubscr>> eventSubscrs;
        std::unordered_map<string, Tango::EventType> eventTypes;
        std::unordered_map<int, string> eventNames;
        std::unordered_map<string, int> eventIt;
        ParsingInputJson* parsing = nullptr;
    };
}

#endif // EVENTPROC_H
