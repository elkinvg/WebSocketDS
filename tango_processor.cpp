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

#include <algorithm>
//#include <unordered_set>


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
        json << "\"data_type\": " << att.get_type() << ", ";
        

        Tango::AttrDataFormat format = att.get_data_format();
        json << "\"data_format\": " << format << ", ";

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

        string nameAttr = attr->get_name();

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
                dataFromAttrsOrCommToJson(stateStr, ss,TYPE_WS_REQ::ATTRIBUTE, nameAttr);
            }
            else {
                (*attr) >> data;
                dataFromAttrsOrCommToJson(data, ss, TYPE_WS_REQ::ATTRIBUTE, nameAttr);
            }
        }
        else
            if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
                (*attr) >> dataVector;
                ss << "[";
                dataArrayFromAttrOrCommToJson(dataVector,ss,TYPE_WS_REQ::ATTRIBUTE,nameAttr);
                ss << "]";
            }
        return ss.str();
    }

    template <typename T>
    void tango_processor::dataFromAttrsOrCommToJson(T& data, std::stringstream& ss, TYPE_WS_REQ type_req, string nameOfAttrOrComm) {
        auto gettedOpts = getOpts(nameOfAttrOrComm, type_req);
        if (is_floating_point<T>::value) {

            // Лямбда-функция для получения числа для std::setprecision
            auto get_srsz = [](string fromOptStr) {
                std::streamsize tmpsz = 0;
                if (fromOptStr != "") {
                    try {
                        tmpsz = (std::streamsize)stoi(fromOptStr);
                    }
                    catch (...) {
                        tmpsz = 0;
                    }
                }
                return tmpsz;
            };

            bool hasIosOpt = false;
            TYPE_IOS_OPT ios_opt;

            // default streamsize. Was 5. From get_srsz return 0 if optStr = ""
            std::streamsize srsz = 0;

            string optStr = ""; // string from getted opt

            if (gettedOpts.find("prec") != gettedOpts.end()) {
                hasIosOpt = true;
                ios_opt = TYPE_IOS_OPT::PREC;
                optStr = gettedOpts["prec"];
            }
            else
                if (gettedOpts.find("precf") != gettedOpts.end()) {
                    hasIosOpt = true;
                    ios_opt = TYPE_IOS_OPT::PRECF;
                    optStr = gettedOpts["precf"];
                }
                else
                    if (gettedOpts.find("precs") != gettedOpts.end()) {
                        hasIosOpt = true;
                        ios_opt = TYPE_IOS_OPT::PRECS;
                        optStr = gettedOpts["precs"];
                    }

            if (!hasIosOpt) {
                ios_opt = TYPE_IOS_OPT::PREC;
            }
            else {
                srsz = get_srsz(optStr);
            }

            outForFloat(data, ss, ios_opt, srsz);
            return;
        }
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
            // Взят boost::optional. В случае, если параметр JSON не найден, присваивает этому параметру пустую строку

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

        json << "{\"event\": \"read\", \"type_req\": \"command\", \"data\": ";
        json << "{";
        json << "\"command_name\": " << "\"" << inputArgs["command"] << "\",";
        json << "\"id_req\": "  << inputArgs["id"] << ",";
        json << "\"data_type\": " << type << ",";

        string command_name = inputArgs["command"];

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
            dataFromAttrsOrCommToJson(bl, json,TYPE_WS_REQ::COMMAND,command_name);
        }
        break;
        case Tango::DEV_SHORT:
        {
            generateStringJsonFromDevData<Tango::DevShort>(devData, json,command_name);
        }
        break;
        case Tango::DEV_LONG:
        {
            generateStringJsonFromDevData<Tango::DevLong>(devData, json,command_name);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            generateStringJsonFromDevData<Tango::DevFloat>(devData, json,command_name);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            generateStringJsonFromDevData<Tango::DevDouble>(devData, json,command_name);
        }
        break;
        case Tango::DEV_USHORT:
        {
            generateStringJsonFromDevData<Tango::DevUShort>(devData, json,command_name);
        }
        break;
        case Tango::DEV_ULONG:
        {
            generateStringJsonFromDevData<Tango::DevULong>(devData, json,command_name);
        }
        break;
        case Tango::DEV_STRING:
        {
            generateStringJsonFromDevData<std::string>(devData, json,command_name);
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
                dataFromAttrsOrCommToJson(tmp, json,TYPE_WS_REQ::COMMAND,command_name);
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
            generateStringJsonFromDevData<Tango::DevShort>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            generateStringJsonFromDevData<Tango::DevLong>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            generateStringJsonFromDevData<Tango::DevFloat>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            generateStringJsonFromDevData<Tango::DevDouble>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            generateStringJsonFromDevData<Tango::DevUShort>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            generateStringJsonFromDevData<Tango::DevULong>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            generateStringJsonFromDevData<std::string>(devData, json,command_name);
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
            generateStringJsonFromDevData<Tango::DevLong64>(devData, json,command_name);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            generateStringJsonFromDevData<Tango::DevULong64>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            generateStringJsonFromDevData<Tango::DevLong64>(devData, json,command_name);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            generateStringJsonFromDevData<Tango::DevULong64>(devData, json,command_name);
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
        json << "} ";
        json << "}";
        return json.str();
    }

    void  tango_processor::addOptsForAttribute(string nameAttr, string option) {
        optsForAttributes.insert(make_pair(nameAttr, option));
    }

    void tango_processor::addOptsForCommand(string nameComm, string option)
    {
        optsForCommands.insert(make_pair(nameComm, option));
    }


    template <typename T>
    void tango_processor::generateStringJsonFromDevData(Tango::DeviceData &devData, std::stringstream& json, string command_name)
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
            dataFromAttrsOrCommToJson(data,json, TYPE_WS_REQ::COMMAND, command_name);
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            json << "\"argout\": [";
            dataArrayFromAttrOrCommToJson(vecFromData,json, TYPE_WS_REQ::COMMAND, command_name);
            json << " ]";
        }
    }

    template <typename T>
    void tango_processor::dataArrayFromAttrOrCommToJson(std::vector<T>& vecFromData, std::stringstream& json,  TYPE_WS_REQ type_req, string nameOfAttrOrComm) {
        bool begin = true;

        for (T fromData : vecFromData) {
            if (!begin) json << ", ";
            else begin = false;
            dataFromAttrsOrCommToJson(fromData, json, type_req, nameOfAttrOrComm);
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
            string nameAttr = attr->get_name();
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
                dataFromAttrsOrCommToJson(tmp, ss,TYPE_WS_REQ::ATTRIBUTE,nameAttr);
            }
            else
                if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
                    (*attr) >> dataVector;
                    for (auto& i : dataVector) {
                        tmpVec.push_back((unsigned short)i);
                    }
                    ss << "[";
                    dataArrayFromAttrOrCommToJson(tmpVec, ss,TYPE_WS_REQ::ATTRIBUTE,nameAttr);
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
        tangoState[Tango::UNKNOWN] = "UNKNOWN";
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

    //--------------------------------------------------------
    /**
    *	Method      : tango_processor::getOpts()
    *	Description : Getting option from name of attribute or command
    */
    //--------------------------------------------------------

    std::unordered_map<string, string> tango_processor::getOpts(string nameOfAttrOrComm, TYPE_WS_REQ type_req) {
        stringmap_iter opts;

        /**
        Получение дополнительных опций из прописанных команд или атрибутов.
        Формат для дополнения опций
        Attrname;opt1=10;opt2=12;opt3
        Здесь AttrDevDouble - имя атрибута
        Остальное опции. Либо имя опции, либо опция со значением через "="
        */

        // лямбда-функция для получения map опций. 
        // Для opt1=10 map["opt1"]="10", для opt3 map["opt3"]=""

        // typedef in "tango_processor.h"
        // typedef std::unordered_multimap < std::string, std::string > stringmap;
        // typedef std::pair<stringmap::iterator, stringmap::iterator> stringmap_iter;
        auto optsMap = [](stringmap_iter opts_in) {
            std::unordered_map<string, string> outMap;

            for_each(
                opts_in.first,
                opts_in.second,
                [&](stringmap::value_type& x){
                size_t pos = 0;
                string delimiter = "=";

                string tmpPars = x.second;

                if ((pos = tmpPars.find(delimiter)) != std::string::npos) {
                    string keyM = tmpPars.substr(0, pos);
                    tmpPars.erase(0, pos + delimiter.length());
                    outMap.insert(std::make_pair(keyM, tmpPars));
                }
                else {
                    outMap.insert(std::make_pair(tmpPars, ""));
                }
            }
            );

            return outMap;
        };

        if (type_req == TYPE_WS_REQ::COMMAND) {
            opts = optsForCommands.equal_range(nameOfAttrOrComm);
        }
        if (type_req == TYPE_WS_REQ::ATTRIBUTE) {
            opts = optsForAttributes.equal_range(nameOfAttrOrComm);
        }

        //std::unordered_map<string, string> forRet = optsMap(opts);
        return optsMap(opts);
    }

    template <typename T>
    void tango_processor::outForFloat(T &data, stringstream &ss, TYPE_IOS_OPT ios_opt, std::streamsize precIn) {

        if (precIn > 20 || precIn < 0)
            precIn = 0;

        if (ios_opt == TYPE_IOS_OPT::PREC)
            ss << std::setprecision(precIn) << data;
        if (ios_opt == TYPE_IOS_OPT::PRECF)
            ss << std::fixed << std::setprecision(precIn) << data;
        if (ios_opt == TYPE_IOS_OPT::PRECS)
            ss << std::scientific << std::setprecision(precIn) << data;
    }
}
