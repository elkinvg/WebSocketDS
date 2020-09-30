#ifndef WSTANGOCONN_SER_H
#define WSTANGOCONN_SER_H

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include "WSTangoConn.h"
#include <vector>
#include <array>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "TaskInfo.h"

namespace WebSocketDS_ns
{
    class WebSocketDS;
    class GroupForWs;
    class DeviceForWs;
    class ConnectionData;
    class EventProcSer;

    class WSTangoConnSer: public WSTangoConn
    {
    public:
        WSTangoConnSer(WebSocketDS * dev);

        virtual string sendRequest(const ParsedInputJson& parsedInput, ConnectionData *connData) override;
        virtual vector<pair<long, TaskInfo>> sendRequestAsync(const ParsedInputJson& parsedInput, ConnectionData *connData, vector<string>& errorsFromGroupReq) override;
        virtual log4tango::Logger *get_logger(void) override;

        virtual string checkResponse(const std::pair<long, TaskInfo>& idInfo) override;

        virtual void setNumOfConnections(unsigned long num) override;

        void update();


        ~WSTangoConnSer();
    private:
        // TODO: no virtual
        virtual vector<string> getDevOptions() override;
        void updateData();
        void for_update_data();
        void initDeviceServer();

        string sendRequest_PipeComm(const ParsedInputJson& parsedInput, ConnectionData* connData);

        void initEventSubscr();
        void deleteEventSubscr();
    private:
        WebSocketDS *_wsds;
        WSThread *wsThread;
        log4tango::Logger *logger = nullptr;
        std::thread *updTh = nullptr;
        GroupForWs* groupForWs = nullptr;
        DeviceForWs* deviceForWs = nullptr;
        
        EventProcSer* eventSubscr = nullptr;
        bool _hasEventSubscr{ false };
        array<vector<string>, 4> list_of_event_subcr;

        bool _isInitDs{ false };
        bool _hasAttrsForUpdate{ false };
        string _errorMessage;

        bool do_update { true };
        bool local_th_exit{ false };

        std::condition_variable _upd_cond;
        std::mutex _upd_lock;

    };
}
#endif
