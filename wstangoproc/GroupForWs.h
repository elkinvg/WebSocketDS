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
        GroupForWs(string pattern, std::pair<vector<string>, vector<string>>& attr_pipes);
        GroupForWs(string pattern, array<vector<string>, 3>& attrCommPipe);
        GroupForWs(string pattern, vector<string> &commands);
        ~GroupForWs();

        static dev_attr_pipe_map getListDevicesFromGroupForAttrAndPipeProc(const dev_attr_pipe_map &group_of_devs, string& errorMessage);
        static vector<string> getArrayOfDevicesFromGroup(const string pattern, string& errorMessage);

        virtual string generateJsonForUpdate() override;
        virtual void generateJsonForUpdate(std::stringstream& json) override;

        virtual void generateJsonForAttrRead(const ParsedInputJson& parsedInput, std::stringstream& json) override;

        virtual string sendPipeCommand(const ParsedInputJson& parsedInput) override;
        virtual string sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) override;
        virtual string sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm) override;

        virtual string sendAttrWr(const ParsedInputJson& parsedInput, bool& statusComm) override;
        virtual string sendAttrRead(const ParsedInputJson& parsedInput) override;

        virtual vector<string> getListOfDevicesNames() override;

    private:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) override;
        virtual bool checkIsAttributeWriteble(const string& attr_name) override;
        virtual bool initAllAttrs() override;
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
