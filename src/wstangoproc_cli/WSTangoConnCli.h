#ifndef WSTANGOCONN_CLI_H
#define WSTANGOCONN_CLI_H

#ifdef _WIN32
#define WINVER 0x0A00
#endif

#include "WSTangoConn.h"
#include <vector>

#include <websocketpp/server.hpp>

namespace WebSocketDS_ns
{
    class WebSocketDS_cli;

    class ConnectionData;
    class EventProcCli;
    class DeviceForWs;
    class GroupForWs;

    class WSTangoConnCli: public WSTangoConn
    {
    public:
        WSTangoConnCli(WebSocketDS_cli * dev);
        virtual string sendRequest(const ParsedInputJson& parsedInput, ConnectionData *connData) override;
        virtual log4tango::Logger *get_logger(void) override;

        virtual vector<pair<long, TaskInfo>> sendRequestAsync(const ParsedInputJson& parsedInput, ConnectionData *connData, vector<string>& errorsFromGroupReq) override;
        virtual string checkResponse(const std::pair<long, TaskInfo>& idInfo) override;

        virtual void setNumOfConnections(unsigned long num) override;
        virtual void setFalsedConnectionStatus() override;

        string sendRequest_Event(websocketpp::connection_hdl hdl, const ParsedInputJson& parsedInput);

        void clientDisconnected(websocketpp::connection_hdl hdl);

       ~WSTangoConnCli();

    private:
        virtual vector<string> getDevOptions() override;
        string getDeviceName(const ParsedInputJson & inputReq);

        Tango::DeviceProxy* _genDeviceForWs(string deviceName, const ParsedInputJson& parsedInput);

        GroupForWs* _genGroupForWs(string devicePatt, const ParsedInputJson& parsedInput);

        vector<pair<long, TaskInfo>> _sendRequestAsyncDevOrGr(const ParsedInputJson & parsedInput, const string& deviceName, const bool& isGroupReq, vector<string>& errorsFromGroupReq);

        string sendRequest_PipeComm(const ParsedInputJson& parsedInput, ConnectionData* connData);
        
    private:
        WebSocketDS_cli *_wsds;
        WSThread *wsThread;
        log4tango::Logger *logger;
        EventProcCli* eventProc = nullptr;

    };
}
#endif
