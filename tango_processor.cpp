#include "tango_processor.h"
#include <iomanip>  
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
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
    tango_processor::tango_processor() {
        initQualityNState();
    }

    bool tango_processor::isMassive(int inType) {
        TYPE_OF_DEVICE_DATA td = typeOfData[inType];
        if (td == TYPE_OF_DEVICE_DATA::ARRAY) return true;
        else return false;
    }

    std::string tango_processor::process_attribute_t(Tango::DeviceAttribute& att) {
        // Генерация JSON из DeviceAttribute
        std::stringstream json;
        json << "";

        std::string quality = attrQuality[att.get_quality()]; //SwitchAttrQuality(att.get_quality());
        json << "{";
        json << "\"attr\": \"" << att.get_name() << "\", ";
        json << "\"qual\": \"" << quality << "\", ";
        json << "\"time\": " << att.time.tv_sec << ", ";

        Tango::AttrDataFormat format = att.get_data_format();
        if (quality == "INVALID")
            json << "\"data\": \"NONE\"";
        else
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
                stateStr = tangoState[stateIn];// SwitchTangoState(stateIn);
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
                dataArrayFromAttrsToJson(dataVector,ss);
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
        // Проверка входных парамеров JSON.
        // Если параметры найдены в строке будет значение параметра, иначе пустая строка
        // Для argin, если входной параметр массив передаёт "Array", иначе просто значение
        boost::property_tree::ptree pt;
        std::stringstream ss;
        std::map<std::string,std::string> output;

        std::string command,id;

        ss << jsonInput;
        try {
            boost::property_tree::read_json(ss, pt);

            //pt.get_value_optional
            // Взят boost::optional. В случае, если параметр JSON не найден присваивает этому параметру пустую строку

            // Если входная строка не JSON, возвращяет строку с ошибкой.
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
                    output.insert(std::pair<std::string, std::string>(v.first, NONE));
                }
            }

            // Здесь, если output["argin"].size() == 0, значит параметр argin найден, но формат данных не простое значение.
            // Прводится проверка, является ли он массивом

            if (output["argin"].size() == 0) {
                int it = 0;
                try {
                    for (boost::property_tree::ptree::value_type &v : pt.get_child("argin")) {
                        it++;
                    }
                }
                catch (boost::property_tree::ptree_bad_data) { it = 0; }
                if (it) output["argin"] = "Array";
            }

        }
        catch (boost::property_tree::json_parser::json_parser_error &je)
        {
            std::string err = "Json parsed error. Message from Boost: " + je.message();
            output.insert(std::make_pair("error",err));
        }

        return output;
    }

    string tango_processor::gettingJsonStrFromDevData(Tango::DeviceData &devData,std::map<std::string,std::string> inputArgs)
    {
        int type;
        try {
            type = devData.get_type();
        }
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
            json << "\"argout\": \"OK\"";
            break;
        case Tango::DEV_BOOLEAN: // ??? not boolean?
        {
            Tango::DevBoolean bl;
            devData >> bl;
            json << "\"argout\": ";
            dataFromAttrsToJson(bl, json);
        }
        break;
        case Tango::DEV_SHORT:
        {
            generateStringJsonFromDevData<Tango::DevShort>(devData, json);
        }
        break;
        case Tango::DEV_LONG:
        {
            generateStringJsonFromDevData<Tango::DevLong>(devData, json);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            generateStringJsonFromDevData<Tango::DevFloat>(devData, json);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            generateStringJsonFromDevData<Tango::DevDouble>(devData, json);
        }
        break;
        case Tango::DEV_USHORT:
        {
            generateStringJsonFromDevData<Tango::DevUShort>(devData, json);
        }
        break;
        case Tango::DEV_ULONG:
        {
            generateStringJsonFromDevData<Tango::DevULong>(devData, json);
        }
        break;
        case Tango::DEV_STRING:
        {
            generateStringJsonFromDevData<std::string>(devData, json);
        }
        break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
        {
            std::vector<unsigned char> vecFromData;
            devData >> vecFromData;
            json << "\"argout\": [";
            bool begin = true;
            for (const auto& fromData : vecFromData) {
                unsigned short int tmp;
                if (!begin) json << ", ";                
                else begin = false;
                tmp = (unsigned short int)fromData;
                dataFromAttrsToJson(tmp, json);
            }
            json << " ]";

        }
                    //{
                    //    Tango::DevUChar parsed;
                    //    deviceData = generateStringJsonFromDevData<Tango::DevUChar>(jsonData,typeForDeviceData);
                    //}
            break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            generateStringJsonFromDevData<Tango::DevShort>(devData, json);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            generateStringJsonFromDevData<Tango::DevLong>(devData, json);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            generateStringJsonFromDevData<Tango::DevFloat>(devData, json);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            generateStringJsonFromDevData<Tango::DevDouble>(devData, json);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            generateStringJsonFromDevData<Tango::DevUShort>(devData, json);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            generateStringJsonFromDevData<Tango::DevULong>(devData, json);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            generateStringJsonFromDevData<std::string>(devData, json);
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
            generateStringJsonFromDevData<Tango::DevLong64>(devData, json);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            generateStringJsonFromDevData<Tango::DevULong64>(devData, json);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            generateStringJsonFromDevData<Tango::DevLong64>(devData, json);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            generateStringJsonFromDevData<Tango::DevULong64>(devData, json);
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
        TYPE_OF_DEVICE_DATA type = typeOfData[devData.get_type()];

        std::vector<T> vecFromData;
        T data;

        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            devData >> data;
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            devData >> vecFromData;
        } else if (type == TYPE_OF_DEVICE_DATA::VOID_D) {
            json << "\"argout\": \"OK\"";
        }

        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            json << "\"argout\": ";
            dataFromAttrsToJson(data,json);
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            json << "\"argout\": [";
            dataArrayFromAttrsToJson(vecFromData,json);
            json << " ]";
        }
    }

    template <typename T>
    void tango_processor::dataArrayFromAttrsToJson(std::vector<T>& vecFromData,  std::stringstream& json) {
        bool begin = true;

        for (T fromData : vecFromData) {
            if (!begin) json << ", ";
            else begin = false;
            dataFromAttrsToJson(fromData, json);
        }
    }


    template <typename T>
    Tango::DeviceData tango_processor::getDeviceDataFromDataType(const std::string& jsonData) {
        boost::property_tree::ptree pt;
        std::stringstream ss;
        ss << jsonData;
        boost::property_tree::read_json(ss, pt);
        T devData;
        Tango::DeviceData dOut;
        try {
            devData = pt.get<T>("argin");
            dOut << devData;
        }
        catch (boost::property_tree::ptree_bad_data) {
            //cout << "Unknown type of data" << endl;
        }

        return dOut;
    }

    template <typename T>
    Tango::DeviceData tango_processor::getDeviceDataFromArrayType(const std::string& jsonData) {
        boost::property_tree::ptree pt;
        std::stringstream ss;
        ss << jsonData;
        boost::property_tree::read_json(ss, pt);
        T devData;
        vector<T> devDataVector;
        Tango::DeviceData dOut;

        try {
            for (boost::property_tree::ptree::value_type &v : pt.get_child("argin")) {
                devDataVector.push_back(v.second.get_value<T>());
            }
        }
        catch (boost::property_tree::ptree_bad_data) {
            //cout << "Unknown type of data" << endl;
        }
        dOut << devDataVector;
        return dOut;
    }

    template <typename T>
    Tango::DeviceData tango_processor::parsingJsonForGenerateData(/*T& devData,*/ const std::string& jsonData, int typeForDeviceData) {

        Tango::DeviceData dOut;

        TYPE_OF_DEVICE_DATA type = typeOfData[typeForDeviceData];

        if (type == TYPE_OF_DEVICE_DATA::DATA) 
            dOut = getDeviceDataFromDataType<T>(jsonData);
        else if (type == TYPE_OF_DEVICE_DATA::ARRAY)
            dOut = getDeviceDataFromArrayType<T>(jsonData);

        return dOut;
    }

    std::string tango_processor::devAttrToStr(Tango::DeviceAttribute *attr) {
        int type = attr->get_type();
        std::string out;
        switch (type) {
        case Tango::DEV_BOOLEAN:
        {
            out = attrsToString<Tango::DevBoolean>(attr);
        }
        break;
        case Tango::DEV_SHORT:
        {
            out = attrsToString<Tango::DevShort>(attr);
        }
        break;
        case Tango::DEV_LONG:
        {
            out = attrsToString<Tango::DevLong>(attr);
        }
        break;
        case Tango::DEV_LONG64:
        {
            out = attrsToString<Tango::DevLong64>(attr);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            out = attrsToString<Tango::DevFloat>(attr);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            out = attrsToString<Tango::DevDouble>(attr);
        }
        break;
        case Tango::DEV_UCHAR:
        {
            std::stringstream ss;
            Tango::AttrDataFormat format = attr->get_data_format();
            if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE)
                ss << "\"dimX\": " << attr->dim_x << ", ";
            if (format == Tango::AttrDataFormat::IMAGE)
                ss << "\"dimY\": " << attr->dim_y << ", ";

            ss << "\"data\": ";
            Tango::DevUChar data;
            unsigned short tmp;
            vector<unsigned char> dataVector;
            vector<unsigned short> tmpVec;
            if (format == Tango::AttrDataFormat::SCALAR) {
                (*attr) >> data;
                tmp = (unsigned short)data;                
                dataFromAttrsToJson(tmp, ss);
            }
            else
                if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
                    (*attr) >> dataVector;
                    for (auto& i : dataVector) {
                        tmpVec.push_back((unsigned short)i);
                    }
                    ss << "[";
                    dataArrayFromAttrsToJson(tmpVec, ss);
                    ss << "]";
                }
            out = ss.str();
        }
        break;
        case Tango::DEV_USHORT:
        {
            out = attrsToString<Tango::DevUShort>(attr);
        }
        break;
        case Tango::DEV_ULONG:
        {
            out = attrsToString<Tango::DevULong>(attr);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            out = attrsToString<Tango::DevULong64>(attr);
        }
        break;
        case Tango::DEV_STRING:
        {
            out = attrsToString<std::string>(attr);
        }
        break;
        case Tango::DEV_STATE:
        {
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
        {
            deviceData = getDeviceDataFromDataType<bool>(jsonData);
        }
        break;
        case Tango::DEV_SHORT:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_LONG:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevLong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevFloat>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevDouble>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_USHORT:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevUShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_ULONG:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevULong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_STRING:
        {
            deviceData = parsingJsonForGenerateData<std::string>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_CHARARRAY:
        {
            deviceData = getDeviceDataFromArrayType<unsigned char>(jsonData);
        }
            break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevLong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevFloat>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevDouble>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevUShort>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevULong>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
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
        {
            //deviceData = getDeviceDataFromDataType<Tango::DevUChar>(jsonData);
            //deviceData = getDeviceDataFromDataType<unsigned char>(jsonData);
        }
            //        {
            //            Tango::DevUChar parsed;
            //            deviceData = parsingJsonForGenerateData(jsonData,typeForDeviceData);
            //        }
            break;
        case Tango::DEV_LONG64:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevLong64>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevULong64>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            deviceData = parsingJsonForGenerateData<Tango::DevLong64>(jsonData, typeForDeviceData);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
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


    void tango_processor::initQualityNState() {
        //init array for AttrQuality
        attrQuality[Tango::AttrQuality::ATTR_VALID] = "VALID";
        attrQuality[Tango::AttrQuality::ATTR_INVALID] = "INVALID";
        attrQuality[Tango::AttrQuality::ATTR_ALARM] = "ALARM";
        attrQuality[Tango::AttrQuality::ATTR_CHANGING] = "CHANGING";
        attrQuality[Tango::AttrQuality::ATTR_WARNING] = "WARNING";

        // ini array for State
        tangoState[Tango::ON] = "ON";
        tangoState[Tango::OFF] = "OFF";
        tangoState[Tango::CLOSE] = "CLOSE";
        tangoState[Tango::OPEN] = "OPEN";
        tangoState[Tango::INSERT] = "INSERT";
        tangoState[Tango::EXTRACT] = "EXTRACT";
        tangoState[Tango::MOVING] = "MOVING";
        tangoState[Tango::STANDBY] = "STANDBY";
        tangoState[Tango::FAULT] = "FAULT";
        tangoState[Tango::INIT] = "INIT";
        tangoState[Tango::RUNNING] = "RUNNING";
        tangoState[Tango::ALARM] = "ALARM";
        tangoState[Tango::DISABLE] = "DISABLE";

        // init array for type (data or massive)
        typeOfData[Tango::DEV_VOID] = TYPE_OF_DEVICE_DATA::VOID_D;

        typeOfData[Tango::DEV_BOOLEAN] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_SHORT] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_LONG] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_FLOAT] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_DOUBLE] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_USHORT] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_ULONG] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_STRING] = TYPE_OF_DEVICE_DATA::DATA;

        typeOfData[Tango::DEVVAR_CHARARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_SHORTARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_LONGARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_FLOATARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_DOUBLEARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_USHORTARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_ULONGARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_STRINGARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_LONGSTRINGARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_DOUBLESTRINGARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        
        //typeOfData[Tango::DEV_STATE] = TYPE_OF_DEVICE_DATA::ARRAY;
        //typeOfData[Tango::CONST_DEV_STRING] = TYPE_OF_DEVICE_DATA::ARRAY;

        typeOfData[Tango::DEVVAR_BOOLEANARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;

        typeOfData[Tango::DEV_UCHAR] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_LONG64] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEV_ULONG64] = TYPE_OF_DEVICE_DATA::DATA;
        typeOfData[Tango::DEVVAR_LONG64ARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEVVAR_ULONG64ARRAY] = TYPE_OF_DEVICE_DATA::ARRAY;
        typeOfData[Tango::DEV_INT] = TYPE_OF_DEVICE_DATA::DATA;
        //typeOfData[Tango::DEV_ENCODED] = TYPE_OF_DEVICE_DATA::DATA;
    }

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
}
