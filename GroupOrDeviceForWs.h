#ifndef GROUPORDEVICEFORWS_H
#define GROUPORDEVICEFORWS_H

#include <tango.h>

#include "common.h"

using std::unordered_map;
using std::pair;

namespace WebSocketDS_ns
{
    class WebSocketDS;
    class TangoProcessor;

    class GroupOrDeviceForWs : public Tango::LogAdapter
    {
    public:
        GroupOrDeviceForWs(WebSocketDS *dev);
        virtual ~GroupOrDeviceForWs();

        void initAttrComm(vector<string> &attributes, vector<string> &commands);
        OUTPUT_DATA_TYPE checkDataType(string commandName);

        virtual string generateJsonForUpdate() = 0;
        virtual Tango::DevString sendCommand(Tango::DevString &argin) = 0;
        virtual Tango::DevVarCharArray* sendCommandBin(Tango::DevString &argin) = 0;

    protected:
        virtual Tango::CommandInfo getCommandInfo(const string& command_name) = 0;

        void generateAttrJson(std::stringstream& json, std::vector<Tango::DeviceAttribute> *attrList);

        Tango::DeviceData tangoCommandInoutForDevice(Tango::DeviceProxy *deviceProxy, Tango::DevString &argin, string &commandName, const string &arginStr, const string idStr, string &errorMess);

        Tango::DevVarCharArray* sendCommandBinForDevice(Tango::DeviceProxy *deviceProxy, Tango::DevString &argin, const std::map<std::string, std::string> &jsonArgs);

        Tango::DevVarCharArray* errorMessageToCharArray(const string&);

        void fromException(Tango::DevFailed &e, string func)
        {
            auto lnh = e.errors.length();
            for (unsigned int i = 0; i<lnh; i++) {
                ERROR_STREAM << " From " + func + ": " << e.errors[i].desc << endl;
            }
        }

    private:
        void gettingAttrOrCommUserConf(string &inp, TYPE_WS_REQ type_req);

        void initAttr(vector<string> &attributes);
        void initComm(vector<string> &commands);

    protected:
        std::vector<std::pair<unsigned short, unsigned short>> nIters;
        unsigned long long iterator{ 0 };
        vector<string> _attributes;
        std::vector<bool>  isJsonAttribute;
        int nAttributes{0};
        std::map<std::string, Tango::CommandInfo> accessibleCommandInfo;
        std::unique_ptr<TangoProcessor> processor;
        WebSocketDS *ds;
    
    private:
//        WebSocketDS *ds;
    };
}

#endif // GROUPORDEVICEFORWS_H
