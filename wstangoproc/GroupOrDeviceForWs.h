#ifndef GROUPORDEVICEFORWS_H
#define GROUPORDEVICEFORWS_H

#include <tango.h>
#include <array>
#include <unordered_set>

#include "ParsingInputJson.h"

using std::unordered_map;
using std::pair;
using std::array;
using std::vector;

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
        virtual void generateJsonForUpdate(std::stringstream& json) = 0;

        virtual void generateJsonForAttrReadCl(const ParsedInputJson& parsedInput, std::stringstream& json) = 0;

        virtual string sendPipeCommand(const ParsedInputJson& parsedInput) = 0;
        virtual string sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) = 0;
        virtual string sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm) = 0;

        virtual string sendAttrWr(const ParsedInputJson& parsedInput, bool& statusComm) = 0;

        virtual vector<string> getListOfDevicesNames() = 0;

        string insertAttrToList(vector<string> &attrNames);
        vector<string> eraseAttrFromList(vector<string> &attrNames, const string& pipeName, const string& deviceName);

    protected:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) = 0;

        virtual bool checkIsAttributeWriteble(const string& attr_name) = 0;

        void generateAttrJson(std::stringstream& json, vector<Tango::DeviceAttribute> *attrList);

        Tango::DeviceData tangoCommandInoutForDevice(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& dataFromJson, string& errorMessInJson);
        Tango::DeviceData tangoCommandInoutForDeviceCl(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& dataFromJson, string& errorMessInJson);

        void generateJsonHeadForPipeComm(const ParsedInputJson& parsedInput, stringstream &json);

        string sendCommandBinForDevice(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& parsedInput, bool& statusComm);

        virtual bool initAllAttrs() = 0;

    //private:

        //void initAttrAndPipe(vector<string> &attributes, vector<string>&pipeName);
        void initAttr(vector<string> &attributes);
        void initPipe(vector<string> &pipeName);
        void initComm(vector<string> &commands);

    private:
        void forNiterOpt(string attrName);
        void _tangoCommandInoutForDevice(Tango::DeviceData &outDeviceData, Tango::DeviceProxy *deviceProxy, const ParsedInputJson& dataFromJson, string& commandName, string& errorMessInJson, int type);

    protected:
        //std::vector<std::pair<unsigned short, unsigned short>> nIters;
        std::unordered_map<std::string, std::pair<unsigned short, unsigned short>> nIters;
        unsigned long long iterator{ 0 };
        vector<string> _attributes;
        string _pipeAttr;
        std::unordered_set<std::string> isJsonAttribute;
        //std::vector<bool>  isJsonAttribute;
        int nAttributes{0};
        std::unordered_map<std::string, Tango::CommandInfo> accessibleCommandInfo;
        std::unique_ptr<TangoProcessor> processor;
        //const string ERR_PRED = "err"; // .insert(0, ERR_PRED)
        std::unordered_set<std::string> isWrtAttribute;
    
    private:
        bool _isShortAttr{ true };
    };
}

#endif // GROUPORDEVICEFORWS_H
