#include "tango_processor.h"
#include <iomanip>  
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/json_parser.hpp>
//#include <boost/foreach.hpp>
//#include <boost/optional/optional.hpp>
#include <cassert>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
//#include <typeinfo>

namespace WebSocketDS_ns
{
    std::string tango_processor::process_attribute_t(Tango::DeviceAttribute& att) {
        // Генерация JSON из DeviceAttribute
        std::stringstream json;
        json << "";

        Tango::AttrQuality quality = att.get_quality();

        json << "{";
        json << "\"attr\": \"" << att.get_name() << "\", ";
        json << "\"qual\": \"" << SwitchAttrQuality(att.get_quality()) << "\", ";
        json << "\"time\": " << att.time.tv_sec << ", ";

        Tango::AttrDataFormat format = att.get_data_format();
        json << devAttrToStr(&att);
        json << "}";
        return json.str();
    }

    bool tango_processor::checkCommand(const string& command, const std::map<std::string, Tango::CommandInfo>& accessibleCommandInfo)
    {
        auto it = accessibleCommandInfo.find(command);
        if (it != accessibleCommandInfo.end()) return true;
        else return false;
    }

    template <typename T>
    std::string  tango_processor::attrsToString(/*T& data,*/ Tango::DeviceAttribute *attr) {
        Tango::AttrDataFormat format = attr->get_data_format();
        int type = attr->get_type();
        std::vector<T> dataVector;
        T data;
        Tango::DevState stateIn;
        string stateStr;

        std::stringstream ss;

        if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE)
            ss << "\"dimX\": " << attr->dim_x << ", ";
        if (format == Tango::AttrDataFormat::IMAGE)
            ss << "\"dimY\": " << attr->dim_y << ", ";

        ss << "\"data\": ";
        if (format == Tango::AttrDataFormat::SCALAR) {
            if (type == Tango::DEV_STATE) {
                (*attr) >> stateIn;
                stateStr = SwitchTangoState(stateIn);
                dataFromAttrsToJson(stateStr, ss);
            }
            else {
                (*attr) >> data;
                dataFromAttrsToJson(data, ss);
            }
        }
        else
            if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
                (*attr) >> dataVector;
                ss << "[";
                bool begin = true;
                for (const auto& fromData : dataVector) {
                    if (!begin) ss << ", ";
                    else begin = false;
                    dataFromAttrsToJson(fromData, ss);
                }
                ss << "]";
            }
        return ss.str();
    }

    template <typename T>
    void tango_processor::dataFromAttrsToJson(T& data, std::stringstream& ss) {
        if (is_floating_point<T>::value) ss << std::setprecision(5) << data;
        else if (std::is_same<T, bool>::value) ss << std::boolalpha << data;
        else if (std::is_same<T, const std::string>::value || std::is_same<T, std::string>::value) ss << "\"" << data << "\"";
        else ss << data;
    }

    std::map<std::string,std::string> tango_processor::getCommandName(const string& jsonInput) {
        boost::property_tree::ptree pt;
        std::stringstream ss;
        std::map<std::string,std::string> output;

        std::string command,id;

        ss << jsonInput;
        boost::property_tree::read_json(ss, pt);

        //pt.get_value_optional
        vector<pair<std::string, boost::optional<std::string>>> boostOpt;

        boostOpt.push_back(std::make_pair("command", pt.get_optional<std::string>("command")));
        boostOpt.push_back(std::make_pair("id", pt.get_optional<std::string>("id")));
        boostOpt.push_back(std::make_pair("argin", pt.get_optional<std::string>("argin")));

        bool isJsonExact = true;
        for (auto& v : boostOpt) {
            if (v.second) {
                output.insert(std::pair<std::string, std::string>(v.first, v.second.get()));
            }
            else {
                output.insert(std::pair<std::string, std::string>(v.first, "NONE"));
            }
        }

        //command = pt.get<std::string>("command");
        //id = pt.get<std::string>("id");
        //output.insert(std::pair<std::string,std::string>("command",command));
        //output.insert(std::pair<std::string,std::string>("id",id));
        return output;
        //return command;
    }

    string tango_processor::gettingJsonStrFromDevData(Tango::DeviceData &devData,std::map<std::string,std::string> inputArgs)
    {
        int type;
        try {
            type = devData.get_type();
        }
//        catch (Tango::DevFailed &e) {
        catch (Tango::WrongData &e) {
            type = Tango::DEV_VOID;
        }

        std::stringstream json;
        // ??? check, if inputArgs['arg'] is empty
        json << "{";
        json << "\"command\": " << "\"" << inputArgs["command"] << "\",";
        json << "\"id\": "  << inputArgs["id"] << ",";

        switch (type)
        {
        case Tango::DEV_VOID:
            json << "\"agrout\": \"OK\"";
            break;
        case Tango::DEV_BOOLEAN: // ??? not boolean?
        {
            //Tango::DevBoolean parsed;
            //generateStringJsonFromDevData<Tango::DevBoolean>(devData, json);
            Tango::DevBoolean bl;
            devData >> bl;
            json << "\"agrout\": ";
            dataFromAttrsToJson(bl, json);
        }
        break;
        case Tango::DEV_SHORT:
        {
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevShort>(devData, json);
        }
        break;
        case Tango::DEV_LONG:
        {
            //Tango::DevLong parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevLong>(devData, json);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            //Tango::DevFloat parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevFloat>(devData, json);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            //Tango::DevDouble parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevDouble>(devData, json);
        }
        break;
        case Tango::DEV_USHORT:
        {
            //Tango::DevUShort parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevUShort>(devData, json);
        }
        break;
        case Tango::DEV_ULONG:
        {
            //Tango::DevULong parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevULong>(devData, json);
        }
        break;
        case Tango::DEV_STRING:
        {
            //std::string parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<std::string>(devData, json);
        }
        break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
                    //{
                    //    Tango::DevUChar parsed;
                    //    deviceData = generateStringJsonFromDevData<Tango::DevUChar>(jsonData,typeForDeviceData);
                    //}
            break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            //Tango::DevShort parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevShort>(devData, json);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            //Tango::DevLong parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevLong>(devData, json);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            //Tango::DevFloat parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevFloat>(devData, json);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            //Tango::DevDouble parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevDouble>(devData, json);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            //Tango::DevUShort parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevUShort>(devData, json);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            //Tango::DevULong parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevULong>(devData, json);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            //std::string parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<std::string>(devData, json);
        }
        break;
        case Tango::DEVVAR_LONGSTRINGARRAY:
            //        {
            //            Tango::DevLong parsed;
            //            deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
            break;
            //        case Tango::DEV_STATE:
            //            json << devStateToStr(&data);
            //            break;
            //        case Tango::CONST_DEV_STRING:
            //            json << devConstStringToStr(&data);
            //            break;
        case Tango::DEVVAR_BOOLEANARRAY:
            //        {
            //            Tango::DevBoolean parsed;
            //            deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEV_UCHAR:
            //        {
            //            Tango::DevUChar parsed;
            //            deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEV_LONG64:
        {
            //Tango::DevLong64 parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevLong64>(devData, json);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            //Tango::DevULong64 parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevULong64>(devData, json);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            //Tango::DevLong64 parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevLong64>(devData, json);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            //Tango::DevULong64 parsed;
            /*fromDeviceData = */generateStringJsonFromDevData<Tango::DevULong64>(devData, json);
        }
        break;
        //case Tango::DEV_INT:
        //{
        //    int parsed;
        //    deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
        //}
        //            break;
        ////        case Tango::DEV_ENCODED:
        ////            json << devEncodedToStr(&data);
        ////            break;
        default:
            break;
        }
        json << "}";
        return json.str();
    }

    template <typename T>
    void tango_processor::generateStringJsonFromDevData(Tango::DeviceData &devData, std::stringstream& json)
    {
        TYPE_OF_DEVICE_DATA type = getTypeOfData(devData.get_type());
        std::vector<T> vecFromData;
        T data;

        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            devData >> data;
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            devData >> vecFromData;
        } else if (type == TYPE_OF_DEVICE_DATA::VOID_D) {
            json << "\"agrout\": \"OK\"";
        }

//        std::stringstream json;
//        json << "{";
//        json << "\"command\": \"test\" ,";
        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            json << "\"agrout\": ";
            dataFromAttrsToJson(data,json);
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            json << "\"agrout\": [";
            bool begin = true;
            for (const auto& fromData : vecFromData) {
                if (!begin) json << ", ";
                else begin = false;
                dataFromAttrsToJson(fromData, json);
            }
            json << " ]";
        }
//        json << "}";
//        return json.str();
    }

    //string tango_processor::process_device_data_json_t(string jsonInput, int typeForDeviceData)
    //{
    //    Tango::DeviceData deviceData = gettingDevDataFromJsonStr(jsonInput,typeForDeviceData);
    //    return "";
    //}


    //    std::string tango_processor::process_device_data_t(Tango::DeviceData& deviceData) {
    //        std::stringstream json;
    //        json << "{";
    //        json << "\"argout\": ";
    //        json << "}";
    //        return json.str();
    //    }

    //template <typename T>
    //Tango::DeviceData tango_processor::parsingJsonForGenerateData_tt(std::string jsonData, int typeForDeviceData) {

    //    boost::property_tree::ptree pt;
    //    std::stringstream ss;
    //    ss << jsonData;
    //    boost::property_tree::read_json(ss, pt);
    //    vector<T> devDataVector;
    //    Tango::DeviceData dOut;
    //    T devData;

    //    TYPE_OF_DEVICE_DATA type = getTypeOfData(typeForDeviceData);
    //    if (type == TYPE_OF_DEVICE_DATA::DATA) {
    //        devData = pt.get<T>("argin");
    //        dOut << devData;
    //    }
    //    else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
    //        for (boost::property_tree::ptree::value_type &v : pt.get_child("argin")) {
    //            devDataVector.push_back(v.second.get_value<T>());
    //        }
    //        dOut << devDataVector;
    //    }
    //    return dOut;
    //}

    boost::property_tree::ptree tango_processor::getPTree(const std::string& jsonData) {
        boost::property_tree::ptree pt;
        std::stringstream ss;
        ss << jsonData;
        boost::property_tree::read_json(ss, pt);
        return pt;
    }

    template <typename T>
    Tango::DeviceData tango_processor::parsingJsonForGenerateData(/*T& devData,*/ const std::string& jsonData, int typeForDeviceData) {

        boost::property_tree::ptree pt;
        pt = getPTree(jsonData);
        //std::stringstream ss;
        //ss << jsonData;
        //boost::property_tree::read_json(ss, pt);
        vector<T> devDataVector;
        Tango::DeviceData dOut;
        T devData;

        TYPE_OF_DEVICE_DATA type = getTypeOfData(typeForDeviceData);
        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            devData = pt.get<T>("argin");
            dOut << devData;
        }
        else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            for (boost::property_tree::ptree::value_type &v : pt.get_child("argin")) {
                devDataVector.push_back(v.second.get_value<T>());
            }
            dOut << devDataVector;
        }
        return dOut;
    }

    std::string tango_processor::devAttrToStr(Tango::DeviceAttribute *attr) {
        int type = attr->get_type();
        //Tango::AttrDataFormat format = attr->get_data_format();
        std::string out;
        switch (type) {
        case Tango::DEV_BOOLEAN:
        {
            //Tango::DevBoolean inp;
            out = attrsToString<Tango::DevBoolean>(attr);
        }
        break;
        case Tango::DEV_SHORT:
        {
            //Tango::DevShort inp;
            out = attrsToString<Tango::DevShort>(attr);
        }
        break;
        case Tango::DEV_LONG:
        {
            //Tango::DevLong inp;
            out = attrsToString<Tango::DevLong>(attr);
        }
        break;
        case Tango::DEV_LONG64:
        {
            //Tango::DevLong64 inp;
            out = attrsToString<Tango::DevLong64>(attr);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            //Tango::DevFloat inp;
            out = attrsToString<Tango::DevFloat>(attr);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            //Tango::DevDouble inp;
            out = attrsToString<Tango::DevDouble>(attr);
        }
        break;
        case Tango::DEV_UCHAR:
        {
            //Tango::DevUChar inp;
            out = attrsToString<Tango::DevUChar>(attr);
        }
        break;
        case Tango::DEV_USHORT:
        {
            //Tango::DevUShort inp;
            out = attrsToString<Tango::DevUShort>(attr);
        }
        break;
        case Tango::DEV_ULONG:
        {
            //Tango::DevULong inp;
            out = attrsToString<Tango::DevULong>(attr);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            //Tango::DevULong64 inp;
            out = attrsToString<Tango::DevULong64>(attr);
        }
        break;
        case Tango::DEV_STRING:
        {
            //Tango::DevString inp;
            //std::string inp;
            out = attrsToString<std::string>(attr);
        }
        break;
        case Tango::DEV_STATE:
        {
            //Tango::DevState inp;
            out = attrsToString<Tango::DevState>(attr);
        }
        break;
        case Tango::DEV_ENCODED:
            // посмотреть
            break;
        default:
            break;
        }
        return out;
    }

    Tango::DeviceData tango_processor::gettingDevDataFromJsonStr(const string &jsonData, int typeForDeviceData)
    {
        Tango::DeviceData deviceData;

        switch (typeForDeviceData)
        {
        case Tango::DEV_VOID:
            break;
        case Tango::DEV_BOOLEAN: // ??? not boolean?
            //{
            //    //Tango::DevBoolean parsed;
            //    deviceData = parsingJsonForGenerateData<bool>(jsonData, typeForDeviceData);
            //}
        {
            Tango::DevBoolean bl;
            boost::property_tree::ptree pt;
            pt = getPTree(jsonData);
            bl = pt.get<bool>("argin");
            deviceData << bl;
            //devData >> bl;
            //json << "\"agrout\": ";
            //dataFromAttrsToJson(bl, json);
        }
        break;
        case Tango::DEV_SHORT:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_LONG:
        {
            //Tango::DevLong parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevLong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            //Tango::DevFloat parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevFloat>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            //Tango::DevDouble parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevDouble>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_USHORT:
        {
            //Tango::DevUShort parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevUShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_ULONG:
        {
            //Tango::DevULong parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevULong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_STRING:
        {
            //std::string parsed;
            deviceData = parsingJsonForGenerateData<std::string>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
                    //{
                    //    Tango::DevUChar parsed;
                    //    deviceData = parsingJsonForGenerateData<Tango::DevUChar>(jsonData,typeForDeviceData);
                    //}
            break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            //Tango::DevShort parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            //Tango::DevLong parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevLong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            //Tango::DevFloat parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevFloat>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            //Tango::DevDouble parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevDouble>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            //Tango::DevUShort parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevUShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            //Tango::DevULong parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevULong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            //std::string parsed;
            deviceData = parsingJsonForGenerateData<std::string>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_LONGSTRINGARRAY:
            //        {
            //            Tango::DevLong parsed;
            //            deviceData = parsingJsonForGenerateData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
            break;
            //        case Tango::DEV_STATE:
            //            json << devStateToStr(&data);
            //            break;
            //        case Tango::CONST_DEV_STRING:
            //            json << devConstStringToStr(&data);
            //            break;
        case Tango::DEVVAR_BOOLEANARRAY:
            //        {
            //            Tango::DevBoolean parsed;
            //            deviceData = parsingJsonForGenerateData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEV_UCHAR:
            //        {
            //            Tango::DevUChar parsed;
            //            deviceData = parsingJsonForGenerateData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEV_LONG64:
        {
            //Tango::DevLong64 parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevLong64>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            //Tango::DevULong64 parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevULong64>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            //Tango::DevLong64 parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevLong64>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            //Tango::DevULong64 parsed;
            deviceData = parsingJsonForGenerateData<Tango::DevULong64>(jsonData, typeForDeviceData);
        }
        break;
        //case Tango::DEV_INT:
        //{
        //    int parsed;
        //    deviceData = parsingJsonForGenerateData(jsonData,typeForDeviceData);
        //}
        //            break;
        ////        case Tango::DEV_ENCODED:
        ////            json << devEncodedToStr(&data);
        ////            break;
        default:
            break;
        }
        return deviceData;
    }

    //    std::string tango_processor::devDataToString(Tango::DeviceData* deviceData) {
    //        int type = deviceData->get_type();
    //        std::string out;

    //        switch (type) {
    //        case Tango::DEV_VOID:
    //            break;
    //        case Tango::DEV_BOOLEAN:
    //            break;
    //         case Tango::DEV_SHORT:
    //            Tango::DevShort inp;
    //            out = dataToString(inp,deviceData);
    //            break;
    //        default:
    //            break;
    //        }
    //        return out;
    //    }

    tango_processor::TYPE_OF_DEVICE_DATA tango_processor::getTypeOfData(int tangoType)
    {
        switch (tangoType)
        {
        case Tango::DEV_VOID:
            return TYPE_OF_DEVICE_DATA::VOID_D;
        case Tango::DEV_BOOLEAN:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_SHORT:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_LONG:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_FLOAT:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_DOUBLE:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_USHORT:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_ULONG:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_STRING:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEVVAR_CHARARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_SHORTARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_LONGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_FLOATARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_DOUBLEARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_USHORTARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_ULONGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_STRINGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_LONGSTRINGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
            //        case Tango::DEV_STATE:
            //            json << devStateToStr(&data);
            //            break;
            //        case Tango::CONST_DEV_STRING:
            //            json << devConstStringToStr(&data);
            //            break;
        case Tango::DEVVAR_BOOLEANARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEV_UCHAR:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_LONG64:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEV_ULONG64:
            return TYPE_OF_DEVICE_DATA::DATA;
        case Tango::DEVVAR_LONG64ARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEVVAR_ULONG64ARRAY:
            return TYPE_OF_DEVICE_DATA::ARRAY;
        case Tango::DEV_INT:
            return TYPE_OF_DEVICE_DATA::DATA;
            //        case Tango::DEV_ENCODED:
            //            json << devEncodedToStr(&data);
            //            break;
        default:
            break;
        }
    }


    //    template <typename T>
    //    std::string  tango_processor::dataToString(T& data, Tango::DeviceData *devData) {
    //        int type = devData->get_type();
    //        std::vector<T> dataVector;

    //        std::stringstream ss;
    //        ss << "\"argout\": ";
    //        if (getTypeOfData(type)==TYPE_OF_DEVICE_DATA::DATA) {
    //            (*devData) >> data;
    //            dataFromAttrsToJson(data,ss);
    //        }
    //        if (getTypeOfData(type)==TYPE_OF_DEVICE_DATA::ARRAY) {
    //            (*devData) >> dataVector;
    //            ss << "[";
    //            bool begin = true;
    //            for (const auto& fromData : dataVector) {
    //                if (!begin) ss << ", ";
    //                else begin = false;
    //                dataFromAttrsToJson(fromData, ss);
    //            }
    //            ss << "]";
    //        }
    //        return ss.str();
    //    }

    // EGOR
    std::string tango_processor::process_device_attribute_json(Tango::DeviceAttribute& data)
    {
        std::string argout;
        (data) >> argout;
        return argout;
    }

    std::string tango_processor::process_device_data_json(Tango::DeviceData& data)
    {
        std::string argout;
        (data) >> argout;
        return argout;
    }

    std::string tango_processor::SwitchAttrQuality(Tango::AttrQuality quality)
    {
        //ATTR_VALID, ATTR_INVALID, ATTR_ALARM, ATTR_CHANGING, ATTR_WARNING
        switch (quality)
        {
        case Tango::AttrQuality::ATTR_VALID:
            return std::string("VALID");
            break;
        case Tango::AttrQuality::ATTR_INVALID:
            return std::string("INVALID");
            break;
        case Tango::AttrQuality::ATTR_ALARM:
            return std::string("ALARM");
            break;
        case Tango::AttrQuality::ATTR_CHANGING:
            return std::string("CHANGING");
            break;
        case Tango::AttrQuality::ATTR_WARNING:
            return std::string("WARNING");
            break;
        default:
            return std::string("INVALID");
        }
    }

    std::string tango_processor::SwitchTangoState(Tango::DevState state)
    {

        switch (state)
        {
        case Tango::ON:
            return std::string("ON");
            break;
        case Tango::OFF:
            return std::string("OFF");
            break;
        case Tango::CLOSE:
            return std::string("CLOSE");
            break;
        case Tango::OPEN:
            return std::string("OPEN");
            break;
        case Tango::INSERT:
            return std::string("INSERT");
            break;
        case Tango::EXTRACT:
            return std::string("EXTRACT");
            break;
        case Tango::MOVING:
            return std::string("MOVING");
            break;
        case Tango::STANDBY:
            return std::string("STANDBY");
            break;
        case Tango::FAULT:
            return std::string("FAULT");
            break;
        case Tango::INIT:
            return std::string("INIT");
            break;
        case Tango::RUNNING:
            return std::string("RUNNING");
            break;
        case Tango::ALARM:
            return std::string("ALARM");
            break;
        case Tango::DISABLE:
            return std::string("DISABLE");
            break;
        default:
            return std::string("UNKNOWN");
        }
    }
}
