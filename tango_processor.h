#ifndef attribute_reader_H
#define attribute_reader_H
#include <tango.h>
#include <array>
#include <unordered_map>

#include "common.h"
typedef std::unordered_multimap < std::string, std::string > stringmap;
typedef std::pair<stringmap::iterator, stringmap::iterator> stringmap_iter;

namespace WebSocketDS_ns
{
    class tango_processor
    {
    public:
        const string NONE = "\"NONE\"";
    private:
        enum class TYPE_OF_DEVICE_DATA { VOID_D = 0, DATA = 1, ARRAY = 2 };
        std::unordered_map<string, unsigned short> attrsWithSetPrecision;
        std::unordered_multimap<string, string> optsForAttributes;
        std::unordered_multimap<string, string> optsForCommands;

    public:
        tango_processor();
        bool isMassive(int inType);
        std::string process_attribute_t(Tango::DeviceAttribute& att);
        bool checkCommand(const string &command, const std::map<string, Tango::CommandInfo> &accessibleCommandInfo);
        Tango::DeviceData gettingDevDataFromJsonStr(const std::string& jsonData, int typeForDeviceData);
        std::map<std::string,std::string> getCommandName(const string& jsonInput);

        //std::string gettingJsonStrFromDevData(Tango::DeviceData& devData, const string &command); // std::map<std::string,std::string>
        std::string gettingJsonStrFromDevData(Tango::DeviceData& devData,std::map<std::string,std::string> inputArgs);

        void addOptsForAttribute(string nameAttr, string option);
        void addOptsForCommand(string nameComm, string option);

    private:
        std::string devAttrToStr(Tango::DeviceAttribute *attr);
        template <typename T>
        std::string attrsToString(/*T& data, */Tango::DeviceAttribute *attr);

        template <typename T>
        Tango::DeviceData parsingJsonForGenerateData(/*T& devData,*/const std::string& jsonData, int typeForDeviceData);


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

        template <typename T>
        void addOutToStringStream(T &data, stringstream &ss, stringmap_iter opts/*, stringmap &options, string nameOfAttrOrComm*/);

        //EGOR
    public:
        std::string process_device_attribute_json(Tango::DeviceAttribute& data);
        std::string process_device_data_json(Tango::DeviceData& data);
    private:
        void initQualityNState();
        std::array<string, 5> attrQuality;
        std::array<string, 14> tangoState;
        std::array<TYPE_OF_DEVICE_DATA,28> typeOfData;

    };
}    //    End of namespace

#endif   //    attribute_reader_H
