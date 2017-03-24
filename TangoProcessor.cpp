#include "TangoProcessor.h"
#include <iomanip>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <sstream>
#include <string>
#include <type_traits>

#include <algorithm>

namespace WebSocketDS_ns
{
    TangoProcessor::TangoProcessor() {
        initQualityNState();
    }

    bool TangoProcessor::isMassive(int inType) {
        TYPE_OF_DEVICE_DATA td = typeOfData[inType];
        if (td == TYPE_OF_DEVICE_DATA::ARRAY) return true;
        else return false;
    }

    string TangoProcessor::processPipe(DevicePipe &pipe)
    {
        /** Генерация JSON для pipe
        {
            "nameOfAttr" : value,
            "nameOfAttr2" : [value1, value2]
        }
        **/

        std::stringstream json;
        size_t numElements = pipe.get_data_elt_nb();

        json << "{";
        for (size_t i=0; i<numElements; i++) {
            string nameOfAttr = pipe.get_data_elt_name(i);
            int typeD = pipe.get_data_elt_type(i);
            if (i)
                json << ", ";
            json << "\"" << nameOfAttr << "\": ";
            extractFromPipe(pipe,json,typeD, nameOfAttr);
        }
        json << "}";
        return json.str();
    }



    std::string TangoProcessor::process_attribute_t(Tango::DeviceAttribute& att) {
        // Генерация JSON из DeviceAttribute
        std::stringstream json;
        json << "";

        std::string quality = attrQuality[att.get_quality()]; //SwitchAttrQuality(att.get_quality());
        json << "{";
        json << "\"attr\": \"" << att.get_name() << "\", ";
        json << "\"qual\": \"" << quality << "\", ";
        json << "\"time\": " << att.time.tv_sec << ", ";

        if (quality == "INVALID")
            json << "\"data\": " << NONE;
        else
            json << devAttrToStr(&att);
          
        json << "}";
        return json.str();
    }


    template <typename T>
    std::string  TangoProcessor::attrsToString(/*T& data,*/ Tango::DeviceAttribute *attr) {
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
    void TangoProcessor::dataFromAttrsOrCommToJson(T& data, std::stringstream& ss, TYPE_WS_REQ type_req, string nameOfAttrOrComm) {
        auto gettedOpts = getOpts(nameOfAttrOrComm, type_req);
        if (is_floating_point<T>::value) {
            // default streamsize.
            std::streamsize srsz =ss.precision();

            // Лямбда-функция для получения числа для std::setprecision
            auto get_srsz = [=](string fromOptStr) {
                std::streamsize tmpsz = srsz;
                if (fromOptStr != "") {
                    try {
                        tmpsz = (std::streamsize)stoi(fromOptStr);
                    }
                    catch (...) {
                        tmpsz = srsz;
                    }
                }
                return tmpsz;
            };

            bool hasIosOpt = false;
            TYPE_IOS_OPT ios_opt;


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

    string TangoProcessor::gettingJsonStrFromDevData(Tango::DeviceData &devData, std::map<string, string> &inputArgs, bool isFromGroup)
    {
        std::stringstream json;

        // Ответ в успешном случае будет таким:
        //    {
        //      "event": "read",
        //      "type_req": "command",
        //      "data": {
        //        "command_name": "имя команды",
        //        "id_req": "id запроса запуска команды",
        //        "argout": "Данные. Единственное значение
        //        или массив в зависимости возвращаемых данных. В случае
        //        массива [...]"
        //      }
        //    }
        if (isFromGroup)
            json << "{\"event\": \"read\", \"type_req\": \"command_device\", \"data\": ";
        else
            json << "{\"event\": \"read\", \"type_req\": \"command\", \"data\": ";

        string command_name;
        if (isFromGroup)
            command_name = inputArgs["command_device"];
        else
            command_name = inputArgs["command"];

        json << "{";
        json << "\"command_name\": " << "\"" << command_name << "\",";
        if (isFromGroup)
            json << "\"device_name\": " << "\"" << inputArgs["device_name"] << "\",";
        try {
            auto idTmp = stoi(inputArgs["id"]);
            json << "\"id_req\": "  << idTmp << ",";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (inputArgs["id"] == NONE)
                json << "\"id_req\": "  << inputArgs["id"] << ",";
            else
                json << "\"id_req\": \""  << inputArgs["id"] << "\",";
        }
        //json << "\"data_type\": " << type << ",";
        json << "\"argout\":";
        generateArgoutForJson(devData,json,command_name);

        json << "} ";
        json << "}";
        return json.str();
    }

    string TangoProcessor::gettingJsonStrFromGroupCmdReplyList(Tango::GroupCmdReplyList &replyList, std::map<string, string> &inputArgs)
    {
        std::stringstream json;
        string command_name = inputArgs["command_group"];

        json << "{\"event\": \"read\", \"type_req\": \"command_group\", ";
        json << "\"data\":";
        json << "{";
        json << "\"command_name\": " << "\"" << command_name << "\", ";
        try {
            auto idTmp = stoi(inputArgs["id"]);
            json << "\"id_req\": "  << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (inputArgs["id"] == NONE)
                json << "\"id_req\": "  << inputArgs["id"] << ", ";
            else
                json << "\"id_req\": \""  << inputArgs["id"] << "\", ";
        }

        json << "\"argout\":";
        json << " {";

        bool notFirst = false;
        for (auto& reply: replyList) {
            //string object_name = reply.obj_name();

            // Get device name.
            // Returns the device name for the group element
            string device_name = reply.dev_name();

            // Getting status. Returns a boolean set to true if the command executed
            // on the group element has failed.
            bool isFailed = reply.has_failed();
            
            if (notFirst)
                json << ", ";
            else
                notFirst = !notFirst;
            json << "\"" << device_name << "\": ";
            if (!isFailed) {
                Tango::DeviceData devData = reply.get_data();
                generateArgoutForJson(devData, json, command_name);
            }
            else {
                const Tango::DevErrorList& el = reply.get_err_stack();
                json << " {";
                json << "\"errors\": [";
                bool tmpf = false;
                for (unsigned int i=0; i<el.length(); i++) {
                    if (tmpf)
                        json << ", ";
                    else
                        tmpf = !tmpf;
                    json << "\"" << el[i].desc.in() << "\"";
                }
                json << "] ";
                json << "} ";
            }
        }
        json << "}"; // \"argout\": {
        json << "}"; // "\"data\":" {
        json << "}"; // "{\"event\"
        return json.str();
    }

    void TangoProcessor::initOptionsForAttrOrComm(string nameAttrOrComm, const std::vector<string> &options, TYPE_WS_REQ type_req)
    {
        // Method initOptionsForAttrOrComm added for Searhing of additional options for attributes
        // Now it is option "prec" for precision
        for (auto opt : options) {
            if (type_req == TYPE_WS_REQ::ATTRIBUTE)
                optsForAttributes.insert(make_pair(nameAttrOrComm,opt));
            if (type_req == TYPE_WS_REQ::COMMAND)
                optsForCommands.insert(make_pair(nameAttrOrComm,opt));
            if (type_req == TYPE_WS_REQ::PIPE)
                optsForPipe.insert(make_pair(nameAttrOrComm,opt));
        }
    }

    template <typename T>
    void TangoProcessor::generateStringJsonFromDevData(Tango::DeviceData &devData, std::stringstream& json, string command_name)
    {
        TYPE_OF_DEVICE_DATA type = typeOfData[devData.get_type()];

        std::vector<T> vecFromData;
        T data;

        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            devData >> data;
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            devData >> vecFromData;
        } else if (type == TYPE_OF_DEVICE_DATA::VOID_D) {
            json << " \"OK\"";
        }

        if (type == TYPE_OF_DEVICE_DATA::DATA) {
            json << " ";
            dataFromAttrsOrCommToJson(data,json, TYPE_WS_REQ::COMMAND, command_name);
        } else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
            json << " [";
            dataArrayFromAttrOrCommToJson(vecFromData,json, TYPE_WS_REQ::COMMAND, command_name);
            json << " ]";
        }
    }

    template <typename T>
    void TangoProcessor::dataArrayFromAttrOrCommToJson(std::vector<T>& vecFromData, std::stringstream& json,  TYPE_WS_REQ type_req, string nameOfAttrOrComm) {
        bool begin = true;

        for (T fromData : vecFromData) {
            if (!begin) json << ", ";
            else begin = false;
            dataFromAttrsOrCommToJson(fromData, json, type_req, nameOfAttrOrComm);
        }
    }


    template <typename T>
    Tango::DeviceData TangoProcessor::getDeviceDataFromDataType(const std::string& jsonData) {
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
    Tango::DeviceData TangoProcessor::getDeviceDataFromArrayType(const std::string& jsonData) {
        boost::property_tree::ptree pt;
        std::stringstream ss;
        ss << jsonData;
        boost::property_tree::read_json(ss, pt);

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
    Tango::DeviceData TangoProcessor::parsingJsonForGenerateData(/*T& devData,*/ const std::string& jsonData, int typeForDeviceData) {

        Tango::DeviceData dOut;

        TYPE_OF_DEVICE_DATA type = typeOfData[typeForDeviceData];

        if (type == TYPE_OF_DEVICE_DATA::DATA) 
            dOut = getDeviceDataFromDataType<T>(jsonData);
        else if (type == TYPE_OF_DEVICE_DATA::ARRAY)
            dOut = getDeviceDataFromArrayType<T>(jsonData);

        return dOut;
    }

    std::string TangoProcessor::devAttrToStr(Tango::DeviceAttribute *attr) {
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

    void TangoProcessor::generateArgoutForJson(Tango::DeviceData &devData, stringstream &json, const string &command_name)
    {
        int type;
        try {
            type = devData.get_type();
        }
        catch (Tango::WrongData &e) {
            type = Tango::DEV_VOID;
        }
        string noneComm = " \"This type Not supported\"";

        switch (type)
        {
        case Tango::DEV_VOID:
            json << " \"OK\"";
            break;
        case Tango::DEV_BOOLEAN: // ??? not boolean?
        {
            Tango::DevBoolean bl;
            devData >> bl;
            json << " ";
            dataFromAttrsOrCommToJson(bl, json, TYPE_WS_REQ::COMMAND, command_name);
        }
        break;
        case Tango::DEV_SHORT:
        {
            generateStringJsonFromDevData<Tango::DevShort>(devData, json, command_name);
        }
        break;
        case Tango::DEV_LONG:
        {
            generateStringJsonFromDevData<Tango::DevLong>(devData, json, command_name);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            generateStringJsonFromDevData<Tango::DevFloat>(devData, json, command_name);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            generateStringJsonFromDevData<Tango::DevDouble>(devData, json, command_name);
        }
        break;
        case Tango::DEV_USHORT:
        {
            generateStringJsonFromDevData<Tango::DevUShort>(devData, json, command_name);
        }
        break;
        case Tango::DEV_ULONG:
        {
            generateStringJsonFromDevData<Tango::DevULong>(devData, json, command_name);
        }
        break;
        case Tango::DEV_STRING:
        {
            generateStringJsonFromDevData<std::string>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
        {
            std::vector<unsigned char> vecFromData;
            devData >> vecFromData;
            json << " [";
            bool begin = true;
            for (const auto& fromData : vecFromData) {
                unsigned short int tmp;
                if (!begin) json << ", ";
                else begin = false;
                tmp = (unsigned short int)fromData;
                dataFromAttrsOrCommToJson(tmp, json, TYPE_WS_REQ::COMMAND, command_name);
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
            generateStringJsonFromDevData<Tango::DevShort>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            generateStringJsonFromDevData<Tango::DevLong>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            generateStringJsonFromDevData<Tango::DevFloat>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            generateStringJsonFromDevData<Tango::DevDouble>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            generateStringJsonFromDevData<Tango::DevUShort>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            generateStringJsonFromDevData<Tango::DevULong>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            generateStringJsonFromDevData<std::string>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_LONGSTRINGARRAY:
            //        {
            //            Tango::DevLong parsed;
            //            deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
            //        }
            json << noneComm;
            break;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
            json << noneComm;
            break;
            //        case Tango::DEV_STATE:
            //            json << devStateToStr(&data);
            //            break;
            //        case Tango::CONST_DEV_STRING:
            //            json << devConstStringToStr(&data);
            //            break;
        case Tango::DEV_STATE:
            Tango::DevState stateIn;
            devData >> stateIn;
            json << " \"" << tangoState[stateIn] << "\"";
            break;
        case Tango::DEVVAR_BOOLEANARRAY:
            //        {
            //            Tango::DevBoolean parsed;
            //            deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
            //        }
            json << noneComm;
            break;
        case Tango::DEV_UCHAR:
            //        {
            //            Tango::DevUChar parsed;
            //            deviceData = generateStringJsonFromDevData(jsonData,typeForDeviceData);
            //        }
            json << noneComm;
            break;
        case Tango::DEV_LONG64:
        {
            generateStringJsonFromDevData<Tango::DevLong64>(devData, json, command_name);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            generateStringJsonFromDevData<Tango::DevULong64>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            generateStringJsonFromDevData<Tango::DevLong64>(devData, json, command_name);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            generateStringJsonFromDevData<Tango::DevULong64>(devData, json, command_name);
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
            json << noneComm;
            break;
        }
    }

    Tango::DeviceData TangoProcessor::gettingDevDataFromJsonStr(const string &jsonData, int typeForDeviceData)
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


    void TangoProcessor::initQualityNState() {
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

    std::string TangoProcessor::process_device_attribute_json(Tango::DeviceAttribute& data)
    {
        std::string argout;
        (data) >> argout;
        return argout;
    }

    std::string TangoProcessor::process_device_data_json(Tango::DeviceData& data)
    {
        std::string argout;
        (data) >> argout;
        return argout;
    }

    pair<bool, string> TangoProcessor::checkOption(string nameOfAttrOrComm, string option, TYPE_WS_REQ type_req) {
        /**
        Получение дополнительных опций из прописанных команд или атрибутов.
        Формат для дополнения опций
        Attrname;opt1=10;opt2=12;opt3
        Здесь AttrDevDouble - имя атрибута
        Остальное опции. Либо имя опции, либо опция со значением через "="
        */

        auto gettedOpts = getOpts(nameOfAttrOrComm, type_req);

        if (gettedOpts.find(option) != gettedOpts.end())
            return make_pair(true, gettedOpts[option]);
        else
            return make_pair(false, "");
    }

    //--------------------------------------------------------
    /**
    *	Method      : TangoProcessor::getOpts()
    *	Description : Getting option from name of attribute or command
    */
    //--------------------------------------------------------

    stringmap TangoProcessor::getOpts(string nameOfAttrOrComm, TYPE_WS_REQ type_req) {
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

        // typedef in "TangoProcessor.h"
        // typedef std::unordered_multimap < std::string, std::string > stringunmap;
        // typedef std::pair<stringunmap::iterator, stringunmap::iterator> stringunmap_iter;
        auto optsMap = [](stringunmap_iter opts_in) {
            stringmap outMap;

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
        if (type_req == TYPE_WS_REQ::PIPE) {
            opts = optsForPipe.equal_range(nameOfAttrOrComm);
        }

        //std::unordered_map<string, string> forRet = optsMap(opts);
        return optsMap(opts);
    }

    template <typename T>
    void TangoProcessor::outForFloat(T &data, stringstream &ss, TYPE_IOS_OPT ios_opt, std::streamsize precIn) {

        if (precIn > 20 || precIn < 0)
            precIn = 0;

        if (ios_opt == TYPE_IOS_OPT::PREC)
            ss << std::setprecision(precIn) << data;
        if (ios_opt == TYPE_IOS_OPT::PRECF)
            ss << std::fixed << std::setprecision(precIn) << data;
        if (ios_opt == TYPE_IOS_OPT::PRECS)
            ss << std::scientific << std::setprecision(precIn) << data;
    }

    template <typename T>
    void TangoProcessor::forExtractingFromPipe(DevicePipe &pipe, stringstream &json,const string& nameOfAttr, bool isArray)
    {
        vector<T> dtArray;
        T dt;

        try {
            if (isArray) {
                pipe >> dtArray;
                json << "[";
                for (int i = 0; i < dtArray.size(); i++) {
                    if (i)
                        json << ", ";
                    T tmpVal = dtArray[i];
                    dataFromAttrsOrCommToJson(tmpVal,json,TYPE_WS_REQ::PIPE,nameOfAttr);
                }
                json << "]";
            }
            else {
                pipe >> dt;
                dataFromAttrsOrCommToJson(dt,json,TYPE_WS_REQ::PIPE,nameOfAttr);
            }
        }
        catch (Tango::DevFailed &df) {
            json << "\"Exception from TangoProcessor::forExtractingFromPipe\"";
        }
    }

    void TangoProcessor::extractFromPipe(DevicePipe &pipe, stringstream &json, int dataType, const string &nameOfAttr)
    {

        switch (dataType)
        {
        case Tango::DEV_VOID:
        {
            json << NONE;
        }
            break;
        case Tango::DEV_BOOLEAN: // ??? not boolean?
        {
            forExtractingFromPipe<bool>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_SHORT:
        {
            forExtractingFromPipe<Tango::DevShort>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_LONG:
        {
            forExtractingFromPipe<Tango::DevLong>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_FLOAT:
        {
            forExtractingFromPipe<Tango::DevFloat>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_DOUBLE:
        {
            forExtractingFromPipe<Tango::DevDouble>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_USHORT:
        {
            forExtractingFromPipe<Tango::DevUShort>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_ULONG:
        {
            forExtractingFromPipe<Tango::DevULong>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_STRING:
        {
            forExtractingFromPipe<string>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
        {
            json << NONE;
        }
            break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            forExtractingFromPipe<short>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_LONGARRAY:
        {
            forExtractingFromPipe<Tango::DevLong>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            forExtractingFromPipe<float>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            forExtractingFromPipe<double>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            forExtractingFromPipe<unsigned short>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            forExtractingFromPipe<Tango::DevULong>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            forExtractingFromPipe<string>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_LONGSTRINGARRAY:
        {
            json << NONE;
        }
            break;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
        {
            json << NONE;
        }
            break;
        case Tango::DEV_STATE:
        {
            Tango::DevState state;
            pipe >> state;
            json << "\"" << tangoState[state] << "\"";
        }
            break;
        case Tango::DEVVAR_BOOLEANARRAY:
        {
            forExtractingFromPipe<bool>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEV_UCHAR:
        {
            json << NONE;
            // ??? forExtractingFromPipe<Tango::DevUChar>(pipe, json, false);
        }
            break;
        case Tango::DEV_LONG64:
        {
            forExtractingFromPipe<Tango::DevLong64>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEV_ULONG64:
        {
            forExtractingFromPipe<Tango::DevULong64>(pipe, json, nameOfAttr, false);
        }
            break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            forExtractingFromPipe<Tango::DevLong64>(pipe, json, nameOfAttr, true);
        }
            break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            forExtractingFromPipe<Tango::DevULong64>(pipe, json, nameOfAttr, true);
        }
            break;
        default:
        {
            json << NONE;
        }
            break;
        }
    }
}
