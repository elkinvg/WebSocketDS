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
        GroupForWs(WebSocketDS *dev, string pattern);
        ~GroupForWs();

        virtual string generateJsonForUpdate() override;
        virtual string generateJsonFromPipeComm(const std::map<std::string, std::string> &pipeConf) override;
        virtual Tango::DevString sendCommand(Tango::DevString &argin) override;
        virtual Tango::DevVarCharArray* sendCommandBin(Tango::DevString &argin) override;

    private:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) override;
        // For Group
        Tango::GroupCmdReplyList tangoCommandInoutForGroup(Tango::DevString &argin, const std::map<std::string, std::string> &jsonArgs, string& errorMess);
        Tango::DevString sendCommandToGroup(Tango::DevString &argin,/* const */std::map<std::string, std::string> &jsonArgs);
        // For device
        Tango::DeviceData  tangoCommandInoutForDeviceFromGroup(Tango::DevString &argin,/* const */std::map<std::string, std::string> &jsonArgs, string& errorMess);
        Tango::DevString sendCommandToDevice(Tango::DevString &argin,/* const */std::map<std::string, std::string> &jsonArgs);

        std::vector<Tango::DeviceAttribute>* getAttributeList(const string& device_name_i, vector<string> &attributes);

    private:
        string generateJsonFromPipeCommForGroup(const std::map<std::string, std::string> &pipeConf);
        string generateJsonFromPipeCommForDeviceFromGroup(const std::map<std::string, std::string> &pipeConf);
        Tango::Group *group = nullptr;
        long group_length{ 0 };
        std::vector<std::string> deviceList;
    };
}

#endif
