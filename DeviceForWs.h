#ifndef DEVICEFORWS_H
#define DEVICEFORWS_H

#include "GroupOrDeviceForWs.h"

namespace WebSocketDS_ns
{
    class DeviceForWs : public GroupOrDeviceForWs
    {
    public:
        DeviceForWs(WebSocketDS *dev, string deviceName);
        ~DeviceForWs();

        virtual string generateJsonForUpdate() override;
        virtual string generateJsonFromPipeComm(const std::map<std::string, std::string> &pipeConf) override;
        virtual Tango::DevString sendCommand(Tango::DevString &argin) override;
        virtual Tango::DevVarCharArray* sendCommandBin(Tango::DevString &argin) override;

    private:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) override;
        Tango::DeviceData tangoCommandInout(Tango::DevString &argin, const std::map<std::string, std::string> &jsonArgs, string& errorMess);

    private:
        Tango::DeviceProxy *device = nullptr;
    };
}

#endif
