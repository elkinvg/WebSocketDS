#ifndef attribute_reader_H
#define attribute_reader_H
#include <tango.h>

namespace WebSocketDS_ns
{
    class tango_processor
    {
    public:
        // elkin begin
        std::string process_attribute_t(Tango::DeviceAttribute att);
        std::string devAttrToStr(Tango::DeviceAttribute *attr);
        template <typename T>
        std::string attrsToString(T& data, Tango::DeviceAttribute *attr);
        template <typename T>
        void dataFromAttrsToJson(T& data, std::stringstream& ss);

        bool checkCommand(const string &jsonInput, const std::map<string, Tango::CommandInfo> &accessibleCommandInfo);
        std::string process_device_data_json_t(std::string jsonInput, int typeForDeviceData);
        Tango::DeviceData gettingDevDataFromJsonStr(std::string jsonData, int typeForDeviceData);

        template <typename T>
        Tango::DeviceData parsingJsonForGenerateData(T& devData, std::string jsonData, int typeForDeviceData);


        //TMP BEGIN
        //std::string process_device_data_t(Tango::DeviceData &deviceData);
        enum class TYPE_OF_DEVICE_DATA {VOID_D=0, DATA=1 ,ARRAY=2};
        TYPE_OF_DEVICE_DATA getTypeOfData(int tangoType);

//        std::string devDataToString(Tango::DeviceData* deviceData);

//        template <typename T>
//        std::string dataToString(T& data, Tango::DeviceData *devData);


        //TMP END
        // elkin end

        std::string process_device_data(Tango::DeviceData data);
        std::string process_device_data_json(Tango::DeviceData data);
        std::string process_device_attribute_json(Tango::DeviceAttribute data);
        Tango::DeviceData process_json(std::string source, int type);

    private:
        std::string SwitchTangoState(Tango::DevState state);
        std::string SwitchAttrQuality(Tango::AttrQuality quality);

        // Encoded
        std::string devEncodedToStr(Tango::DeviceData *data);
        //std::string devEncodedScalarToStr(Tango::DeviceAttribute *attr);
        std::string devEncodedSpectrumToStr(Tango::DeviceAttribute *attr);
        std::string devEncodedImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevEncodedData(std::string data);

        // Void
        std::string devVoidToStr(Tango::DeviceData *data);
        Tango::DeviceData strToDevVoidData(std::string data);

        // State
        std::string devStateToStr(Tango::DeviceData *data);

        Tango::DeviceData strToDevStateData(std::string data);

        // Char
        std::string devCharArrayToStr(Tango::DeviceData *data);

        // Char array
        Tango::DeviceData strToDevCharArrayData(std::string data);

        // UChar
        //std::string devUCharScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devUCharSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devUCharImageToStr(Tango::DeviceAttribute *attr);

        // Boolean
        std::string devBooleanToStr(Tango::DeviceData *data);
        std::string devBooleanArrayToStr(Tango::DeviceData *data);
        //std::string devBooleanScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devBooleanSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devBooleanImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevBooleanData(std::string data);
        Tango::DeviceData strToDevBooleanArrayData(std::string data);
        Tango::DeviceAttribute strToDevBooleanScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevBooleanSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevBooleanImageAttr(std::string data);

        // String
        std::string devStringToStr(Tango::DeviceData *data);
        std::string devStringArrayToStr(Tango::DeviceData *data);
        //std::string devStringScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devStringSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devStringImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevStringData(std::string data);
        Tango::DeviceData strToDevStringArrayData(std::string data);
        Tango::DeviceAttribute strToDevStringScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevStringSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevStringImageAttr(std::string data);

        // Short
        std::string devShortToStr(Tango::DeviceData *data);
        std::string devShortArrayToStr(Tango::DeviceData *data);
        //std::string devShortScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devShortSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devShortImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevShortData(std::string data);
        Tango::DeviceData strToDevShortArrayData(std::string data);
        Tango::DeviceAttribute strToDevShortScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevShortSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevShortImageAttr(std::string data);

        // UShort
        std::string devUShortToStr(Tango::DeviceData *data);
        std::string devUShortArrayToStr(Tango::DeviceData *data);
        //std::string devUShortScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devUShortSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devUShortImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevUShortData(std::string data);
        Tango::DeviceData strToDevUShortArrayData(std::string data);
        Tango::DeviceAttribute strToDevUShortScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevUShortSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevUShortImageAttr(std::string data);

        // Long
        std::string devLongToStr(Tango::DeviceData *data);
        std::string devLongArrayToStr(Tango::DeviceData *data);
        //std::string devLongScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devLongSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devLongImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevLongData(std::string data);
        Tango::DeviceData strToDevLongArrayData(std::string data);
        Tango::DeviceAttribute strToDevLongScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevLongSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevLongImageAttr(std::string data);

        // ULong
        std::string devULongToStr(Tango::DeviceData *data);
        std::string devULongArrayToStr(Tango::DeviceData *data);
        //std::string devULongScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devULongSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devULongImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevULongData(std::string data);
        Tango::DeviceData strToDevULongArrayData(std::string data);
        Tango::DeviceAttribute strToDevULongScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevULongSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevULongImageAttr(std::string data);

        // Long64
        std::string devLong64ToStr(Tango::DeviceData *data);
        std::string devLong64ArrayToStr(Tango::DeviceData *data);
        //std::string devLong64ScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devLong64SpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devLong64ImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevLong64Data(std::string data);
        Tango::DeviceData strToDevLong64ArrayData(std::string data);
        Tango::DeviceAttribute strToDevLong64ScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevLong64SpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevLong64ImageAttr(std::string data);

        // ULong64
        std::string devULong64ToStr(Tango::DeviceData *data);
        std::string devULong64ArrayToStr(Tango::DeviceData *data);
        //std::string devULong64ScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devULong64SpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devULong64ImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevULong64Data(std::string data);
        Tango::DeviceData strToDevULong64ArrayData(std::string data);
        Tango::DeviceAttribute strToDevULong64ScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevULong64SpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevULong64ImageAttr(std::string data);

        // Float
        std::string devFloatToStr(Tango::DeviceData *data);
        std::string devFloatArrayToStr(Tango::DeviceData *data);
        //std::string devFloatScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devFloatSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devFloatImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevFloatData(std::string data);
        Tango::DeviceData strToDevFloatArrayData(std::string data);
        Tango::DeviceAttribute strToDevFloatScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevFloatSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevFloatImageAttr(std::string data);

        // Double
        std::string devDoubleToStr(Tango::DeviceData *data);
        std::string devDoubleArrayToStr(Tango::DeviceData *data);
        //std::string devDoubleScalarToStr(Tango::DeviceAttribute *attr);
        //std::string devDoubleSpectrumToStr(Tango::DeviceAttribute *attr);
        //std::string devDoubleImageToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceData strToDevDoubleData(std::string data);
        Tango::DeviceData strToDevDoubleArrayData(std::string data);
        Tango::DeviceAttribute strToDevDoubleScalarAttr(std::string data);
        Tango::DeviceAttribute strToDevDoubleSpectrumAttr(std::string data);
        Tango::DeviceAttribute strToDevDoubleImageAttr(std::string data);

        // Long string
        std::string devLongStringArrayToStr(Tango::DeviceData *data);
        Tango::DeviceData strToDevLongStringArrayData(std::string data);

        // Double string
        std::string devDoubleStringArrayToStr(Tango::DeviceData *data);
        Tango::DeviceData strToDevDoubleStringArrayData(std::string data);

        // Const string
        std::string devConstStringToStr(Tango::DeviceData *data);
        Tango::DeviceData strToConstStringData(std::string data);

        // UChar
        std::string devUCharToStr(Tango::DeviceData *data);
        Tango::DeviceData strToDevUCharData(std::string data);

        // Int
        std::string devIntToStr(Tango::DeviceData *data);
        Tango::DeviceData strToDevIntData(std::string data);

    };
}    //    End of namespace

#endif   //    attribute_reader_H
