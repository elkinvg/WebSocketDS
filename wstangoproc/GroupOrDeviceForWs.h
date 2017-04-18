#ifndef GROUPORDEVICEFORWS_H
#define GROUPORDEVICEFORWS_H

#include <tango.h>
#include <array>
#include "common.h"

using std::unordered_map;
using std::pair;
using std::array;

namespace WebSocketDS_ns
{
    class TangoProcessor;

    class GroupOrDeviceForWs
    {
    public:
        GroupOrDeviceForWs();
        virtual ~GroupOrDeviceForWs();
        
        void useNotShortAttrOut() { _isShortAttr = false; };

        void initAttrCommPipe(array<vector<string>, 3>& attrCommPipe);
        OUTPUT_DATA_TYPE checkDataType(string commandName);

        virtual string generateJsonForUpdate() = 0;
        virtual string sendPipeCommand(const ParsedInputJson& parsedInput) = 0;
        virtual string sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) = 0;
        virtual string sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm) = 0;

    protected:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) = 0;

        void generateAttrJson(std::stringstream& json, std::vector<Tango::DeviceAttribute> *attrList);

        Tango::DeviceData tangoCommandInoutForDevice(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& dataFromJson, string& errorMessInJson);

        void generateJsonHeadForPipeComm(const ParsedInputJson& parsedInput, stringstream &json);

        string sendCommandBinForDevice(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& parsedInput, bool& statusComm);

    private:

        void initAttrAndPipe(vector<string> &attributes, vector<string>&pipeName);
        void initComm(vector<string> &commands);

    protected:
        std::vector<std::pair<unsigned short, unsigned short>> nIters;
        unsigned long long iterator{ 0 };
        vector<string> _attributes;
        string _pipeAttr;
        std::vector<bool>  isJsonAttribute;
        int nAttributes{0};
        std::unordered_map<std::string, Tango::CommandInfo> accessibleCommandInfo;
        std::unique_ptr<TangoProcessor> processor;
        const string ERR_PRED = "err";
    
    private:
        bool _isShortAttr{ true };
    };
}

#endif // GROUPORDEVICEFORWS_H