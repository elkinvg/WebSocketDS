#ifndef GROUPFORWS_H
#define GROUPFORWS_H

#include "GroupOrDeviceForWs.h"
#include <exception>

namespace WebSocketDS_ns
{
    class GroupForWsException : public exception
    {
        virtual const char* what() const throw()
        {
            return "Exception from getCommandInfoForGroup";
        }
    };

    class GroupForWs : public GroupOrDeviceForWs
    {
    public:
        GroupForWs(string pattern);
        ~GroupForWs();

        virtual string generateJsonForUpdate() override;
        virtual string sendPipeCommand(const ParsedInputJson& parsedInput) override;
        virtual string sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) override;
        virtual string sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm) override;

    private:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) override;
        // For Group
        Tango::GroupCmdReplyList tangoCommandInoutForGroup(const ParsedInputJson& dataFromJson, string& errorMessInJson);
        // For device
        std::vector<Tango::DeviceAttribute>* getAttributeList(const string& device_name_i, vector<string> &attributes);

    private:
        string generateJsonFromPipeCommForGroup(const ParsedInputJson& parsedInput);
        string generateJsonFromPipeCommForDeviceFromGroup(const ParsedInputJson& parsedInput);
        Tango::Group *group = nullptr;
        long group_length{ 0 };
        std::vector<std::string> deviceList;
    };
}

#endif
