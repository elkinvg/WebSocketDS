#ifndef EVENTPROCFORCLIENTMODE_H
#define EVENTPROCFORCLIENTMODE_H

#include <map>
#include <unordered_map>
#include <tango.h>
#include <websocketpp/server.hpp>
#include <mutex>

using std::unordered_map;
using std::map;
using std::mutex;

namespace WebSocketDS_ns
{
    struct ParsedInputJson;
    class WsEvCallBackCli;
    class WSThread;
    struct ResponseFromEventReq;

    struct TangoAttrEventType {
        string eventTypeString;
        string deviceName;
        string attrName;
    };

    
    struct UsedEventSubscr
    {
        map < websocketpp::connection_hdl, string, std::owner_less < websocketpp::connection_hdl>> connList;
        int eventSubId;
        TangoAttrEventType eventAttrCh;
        WsEvCallBackCli* evCallback = nullptr;
    };

    typedef unordered_map <string,  // device name
        unordered_map < string,     // attribute name 
        unordered_map < string,     // event type
        UsedEventSubscr
        >>> event_sub_list;

    typedef map< websocketpp::connection_hdl,
        unordered_map <string,  // device name
        unordered_map <string,  // attribute name 
        Tango::EventType>>,     // type of event
        std::owner_less<websocketpp::connection_hdl > > event_type_list;

    class EventProcCli
    {
    public:
        EventProcCli(WSThread* wsThread, bool isOldVersionOfJson);
        ~EventProcCli();
        
        string request(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl);
        void sendMessage(const string& deviceName, const string & attrName, Tango::EventData * dt);
        void unsubscribeAllDev(websocketpp::connection_hdl hdl);

    private:
        string _addDevReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl);
        string _checkEventReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl);
        string _remDevReq(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl);

        int _getIdOfEventSubscription(websocketpp::connection_hdl hdl, const string& deviceName, const string& attribute, const string& eventType);

        bool _delSubHdl(websocketpp::connection_hdl hdl, TangoAttrEventType& evInfo);
        void _clearSubscrInfoMaps(const TangoAttrEventType& evInfo);

        bool _checkDeviceExist(string deviceName);
        ResponseFromEventReq _addCallback(websocketpp::connection_hdl hdl, const string& deviceName, const string & attrName, const string& precOpt, Tango::EventType eventType);
        void _checkEventReqList(const ParsedInputJson & parsedInput, websocketpp::connection_hdl hdl, const unordered_map<string, vector<string>>& evSubList, Tango::EventType eventType, vector<ResponseFromEventReq>& successResponses, vector<ResponseFromEventReq>& errorResponses);


    private:
        unordered_map<string, Tango::DeviceProxy*> usedDevices;
        event_sub_list eventSubs;
        event_type_list eventTypes;
        unordered_map<int, TangoAttrEventType> listOfId;

        mutex m_eventproc_lock;
        WSThread* _wsThread;
        bool _isOldVersionOfJson{ false };
    };
}
#endif
