#ifndef TANGO_PROCESSOR_H
#define TANGO_PROCESSOR_H


#include <tango.h>
#include <array>
#include <unordered_map>

#include "common.h"
using Tango::DevicePipe;

namespace WebSocketDS_ns
{
    class TangoProcessor
    {
    public:
        TangoProcessor();
        bool isMassive(int inType);

        string processPipe(DevicePipe& pipe, TYPE_WS_REQ pipeType);

        std::string process_attribute_t(Tango::DeviceAttribute& att, bool isShortAttr);
        Tango::DeviceData gettingDevDataFromJsonStr(const std::string& jsonData, int typeForDeviceData);

        std::string gettingJsonStrFromDevData(Tango::DeviceData& devData, std::map<std::string,std::string>& inputArgs, bool isFromGroup = false);
        std::string gettingJsonStrFromGroupCmdReplyList(Tango::GroupCmdReplyList& replyList,std::map<std::string,std::string>& inputArgs);

        pair<bool, string> checkOption(string nameOfAttrOrComm, string option, TYPE_WS_REQ type_req);
        void initOptionsForAttrOrComm(string nameAttrOrComm, const std::vector<string> &options, TYPE_WS_REQ type_req);


    private:
        void initQualityNState();
        stringmap getOpts(string nameOfAttrOrComm, TYPE_WS_REQ type_req);
        std::string devAttrToStr(Tango::DeviceAttribute *attr);

        // argout
        void generateArgoutForJson(Tango::DeviceData& devData, std::stringstream& json,const string& command_name);

        template <typename T>
        std::string attrsToString(Tango::DeviceAttribute *attr);

        template <typename T>
        Tango::DeviceData parsingJsonForGenerateData(const std::string& jsonData, int typeForDeviceData);


        template <typename T>
        void generateStringJsonFromDevData(Tango::DeviceData& devData, std::stringstream& json, string command_name);

        template <typename T>
        Tango::DeviceData getDeviceDataFromDataType(const std::string& jsonData);
        template <typename T>
        Tango::DeviceData getDeviceDataFromArrayType(const std::string& jsonData);

        template <typename T>
        void dataFromAttrsOrCommToJson(T& data, std::stringstream& ss, TYPE_WS_REQ type_req , string nameOfAttrOrCom);
        template <typename T>
        void dataArrayFromAttrOrCommToJson(std::vector<T>& vecFromData, std::stringstream& json,  TYPE_WS_REQ type_req, string nameOfAttrOrComm);

        // getting ios options for floating type
        template <typename T>
        void outForFloat(T &data, stringstream &ss, TYPE_IOS_OPT ios_opt, std::streamsize precIn = 0);

        // FOR PIPE
        void extractFromPipe(DevicePipe& pipe, std::stringstream& json , int dataType, std::pair<string,TYPE_WS_REQ> nameOfAttrAndTypeWsReq);

        template <typename T>
        void forExtractingFromPipe(DevicePipe &pipe, stringstream &json, std::pair<string, TYPE_WS_REQ> &nameOfAttrAndTypeWsReq, bool isArray);

        //EGOR
    public:
        std::string process_device_attribute_json(Tango::DeviceAttribute& data);
        std::string process_device_data_json(Tango::DeviceData& data);
        // Attributes
    private:
        enum class TYPE_OF_DEVICE_DATA { VOID_D = 0, DATA = 1, ARRAY = 2 };
        //std::unordered_map<string, unsigned short> attrsWithSetPrecision;
        stringunmap optsForAttributes;
        stringunmap optsForCommands;
        stringunmap optsForPipe;
        stringunmap optsForPipeComm;
        std::array<string, 5> attrQuality;
        std::array<string, 14> tangoState;
        std::array<TYPE_OF_DEVICE_DATA,28> typeOfData;

    };
}    //    End of namespace

#endif   //    TANGO_PROCESSOR_H
