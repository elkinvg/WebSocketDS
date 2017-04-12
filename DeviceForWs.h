#ifndef DEVICEFORWS_H
#define DEVICEFORWS_H

#include "GroupOrDeviceForWs.h"

namespace WebSocketDS_ns
{
    class DeviceForWs : public GroupOrDeviceForWs
    {
    public:
        DeviceForWs(string deviceName);
        ~DeviceForWs();

        virtual string generateJsonForUpdate() override;
        virtual string sendPipeCommand(const ParsedInputJson& parsedInput) override;
        virtual string sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) override;

        virtual string sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm) override;

    private:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) override;

    private:
        Tango::DeviceProxy *device = nullptr;
    };
}

#endif
