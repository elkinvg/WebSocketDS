#ifndef DEVICEFORWS_H
#define DEVICEFORWS_H

#include "GroupOrDeviceForWs.h"

namespace WebSocketDS_ns
{
    class DeviceForWs : public GroupOrDeviceForWs
    {
    public:
        DeviceForWs(string deviceName);
        DeviceForWs(string deviceName, std::pair<vector<string>, vector<string>>& attr_pipes);
        DeviceForWs(string deviceName, array<vector<string>, 3>& attrCommPipe);
        DeviceForWs(string deviceName, const string& commandOrAttrName, TYPE_WS_REQ type_req);
        ~DeviceForWs();

        virtual string generateJsonForUpdate() override;
        virtual void generateJsonForUpdate(std::stringstream& json) override;

        virtual void generateJsonForAttrReadCl(const ParsedInputJson& parsedInput, std::stringstream& json) override;

        virtual string sendPipeCommand(const ParsedInputJson& parsedInput) override;
        virtual string sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) override;

        virtual string sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm) override;

        virtual string sendAttrWr(const ParsedInputJson& parsedInput, bool& statusComm) override;

        bool pingDevice(string& errorMess);

    private:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) override;
        virtual bool checkIsAttributeWriteble(const string& attr_name) override;

        virtual bool initAllAttrs() override;
        void forGenerateJsonForUpdate(stringstream &json);
        void getDeviceNameFromAlias(string& alias);

    private:
        Tango::DeviceProxy *device = nullptr;
    };
}

#endif
