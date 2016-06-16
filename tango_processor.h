#ifndef attribute_reader_H
#define attribute_reader_H
#include <tango.h>

namespace WebSocketDS_ns
{
    class tango_processor
    {
    public:
        // elkin begin
        std::string process_attribute_t(Tango::DeviceAttribute& att);
        bool checkCommand(const string &command, const std::map<string, Tango::CommandInfo> &accessibleCommandInfo);
        Tango::DeviceData gettingDevDataFromJsonStr(const std::string& jsonData, int typeForDeviceData);
        std::map<std::string,std::string> getCommandName(const string& jsonInput);

        //std::string gettingJsonStrFromDevData(Tango::DeviceData& devData, const string &command); // std::map<std::string,std::string>
        std::string gettingJsonStrFromDevData(Tango::DeviceData& devData,std::map<std::string,std::string> inputArgs);

    private:

        std::string devAttrToStr(Tango::DeviceAttribute *attr);
        template <typename T>
        std::string attrsToString(/*T& data, */Tango::DeviceAttribute *attr);
        template <typename T>
        void dataFromAttrsToJson(T& data, std::stringstream& ss);

        template <typename T>
        Tango::DeviceData parsingJsonForGenerateData(/*T& devData,*/const std::string& jsonData, int typeForDeviceData);

        //TMP BEGIN
        //std::string process_device_data_t(Tango::DeviceData &deviceData);
        enum class TYPE_OF_DEVICE_DATA {VOID_D=0, DATA=1 ,ARRAY=2};
        TYPE_OF_DEVICE_DATA getTypeOfData(int tangoType);

        template <typename T>
        void generateStringJsonFromDevData(Tango::DeviceData& devData, std::stringstream& json);

//        std::string devDataToString(Tango::DeviceData* deviceData);

//        template <typename T>
//        std::string dataToString(T& data, Tango::DeviceData *devData);


        //TMP END
        // elkin end

        //EGOR
    public:
        std::string process_device_attribute_json(Tango::DeviceAttribute& data);
        std::string process_device_data_json(Tango::DeviceData& data);
    private:
        std::string SwitchAttrQuality(Tango::AttrQuality quality);
        std::string SwitchTangoState(Tango::DevState state);

    };
}    //    End of namespace

#endif   //    attribute_reader_H
