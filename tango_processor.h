#ifndef attribute_reader_H
#define attribute_reader_H
#include <tango.h>
#include <array>
#include <unordered_map>

namespace WebSocketDS_ns
{
    class tango_processor
    {
    public:
        const string NONE = "\"NONE\"";
    private:
        enum class TYPE_OF_DEVICE_DATA { VOID_D = 0, DATA = 1, ARRAY = 2 };
        unordered_map<string, unsigned short> attrsWithSetPrecision;
        unordered_multimap<string, string> optsForAttributes;
        unordered_multimap<string, string> optsForCommands;

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

    private:
        std::string devAttrToStr(Tango::DeviceAttribute *attr);
        template <typename T>
        std::string attrsToString(/*T& data, */Tango::DeviceAttribute *attr);
        template <typename T>
        void dataFromAttrsToJson(T& data, std::stringstream& ss, string nameOfAttr = "");

        template <typename T>
        Tango::DeviceData parsingJsonForGenerateData(/*T& devData,*/const std::string& jsonData, int typeForDeviceData);


        template <typename T>
        void generateStringJsonFromDevData(Tango::DeviceData& devData, std::stringstream& json);

        template <typename T>
        Tango::DeviceData getDeviceDataFromDataType(const std::string& jsonData);
        template <typename T>
        Tango::DeviceData getDeviceDataFromArrayType(const std::string& jsonData);

        template <typename T>
        void dataArrayFromAttrsToJson(std::vector<T>& vecFromData, std::stringstream& json, string nameOfAttr = "");

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
