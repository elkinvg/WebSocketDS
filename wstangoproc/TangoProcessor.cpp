#include "TangoProcessor.h"

#include <sstream>
#include <string>
#include <type_traits>

#include <algorithm>
#include "StringProc.h"

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

    string TangoProcessor::processPipe(DevicePipe &pipe, TYPE_WS_REQ pipeType)
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
            extractFromPipe(pipe, json, typeD, make_pair(nameOfAttr, pipeType));
        }
        json << "}";
        return json.str();
    }



    std::string TangoProcessor::process_attribute_t(Tango::DeviceAttribute& att, bool isShortAttr) {
        // Генерация JSON из DeviceAttribute
        std::stringstream json;
        json << "";

        std::string quality = attrQuality[att.get_quality()]; //SwitchAttrQuality(att.get_quality());
        json << "{";
        json << "\"attr\": \"" << att.get_name() << "\", ";

        // Выводятся TIMESTAMP и quality, только isShortAttr == false
        if (!isShortAttr) {
            json << "\"qual\": \"" << quality << "\", ";
            json << "\"time\": " << att.time.tv_sec << ", ";
        }
        // Если isShortAttr == true, quality выводится если оно не равно VALID
        else if (quality != "VALID")
            json << "\"qual\": \"" << quality << "\", ";

        if (quality == "INVALID")
            json << "\"data\": " << NONE;
        else
            json << devAttrToStr(&att);
          
        json << "}";
        return json.str();
    }

    string TangoProcessor::getJsonStrFromDevData(Tango::DeviceData &devData, const ParsedInputJson& inputArgs)
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

        json << "{\"event\": \"read\", \"type_req\": \"" << inputArgs.type_req << "\", ";
        try {
            auto idTmp = stoi(inputArgs.id);
            json << "\"id_req\": " << idTmp << ",";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (inputArgs.id == NONE)
                json << "\"id_req\": " << inputArgs.id << ",";
            else
                json << "\"id_req\": \"" << inputArgs.id << "\",";
        }
        json << " \"data\":";

        string command_name = inputArgs.otherInpStr.at("command_name");

        // В режимах SERVER* дополнительные параметры команд 
        // прописываются в property
        // Здесь, при отправлении команды от клиента дополнительные выставляются
        // в имени команды.
        // Пример: CommandName;precf=15
        if (inputArgs.type_req == "command_device_cl")
            StringProc::parseInputString(command_name, ";");

        json << "{";
        json << "\"command_name\": " << "\"" << command_name << "\",";

        if (inputArgs.type_req == "command_device" || inputArgs.type_req == "command_device_cl")
        {
            if (inputArgs.check_key("device_name") == TYPE_OF_VAL::VALUE)
                json << "\"device_name\": " << "\"" << inputArgs.otherInpStr.at("device_name") << "\",";
        }

        json << "\"argout\":";
        generateArgoutForJson(devData,json,command_name);

        json << "} ";
        json << "}";
        return json.str();
    }

    string TangoProcessor::getJsonStrFromGroupCmdReplyList(Tango::GroupCmdReplyList &replyList, const ParsedInputJson& parsedInput)
    {
        std::stringstream json;
        string commandName = parsedInput.otherInpStr.at("command_name");

        json << "{\"event\": \"read\", \"type_req\": \"command_group\", ";

        try {
            auto idTmp = stoi(parsedInput.id);
            json << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (parsedInput.id == NONE)
                json << "\"id_req\": " << parsedInput.id << ", ";
            else
                json << "\"id_req\": \"" << parsedInput.id << "\", ";
        }

        json << "\"data\":";
        json << "{";
        json << "\"command_name\": " << "\"" << commandName << "\", ";
       

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
                generateArgoutForJson(devData, json, commandName);
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
            if (type_req == TYPE_WS_REQ::PIPE_COMM)
                optsForPipeComm.insert(make_pair(nameAttrOrComm,opt));
        }
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
            int dim_x = attr->dim_x;
            int dim_y = attr->dim_y;
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
                    if (!dim_y)
                        dim_y = 1;
                    (*attr) >> dataVector;

                    int iter = 0;
                    for (auto& i : dataVector) {
                        // for writable attribute
                        // Read only Read_data (without Write_data)
                        if (iter >= dim_x*dim_y)
                            break;

                        tmpVec.push_back((unsigned short)i);
                        iter++;
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

    Tango::DeviceData TangoProcessor::getDeviceDataFromParsedJson(const ParsedInputJson& dataFromJson, int typeForDeviceData)
    {

        Tango::DeviceData deviceData;
        string inpStr;
        vector<string> inpVecStr;

        TYPE_OF_VAL typeOfVaArgin = dataFromJson.check_key("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::VALUE)
            inpStr = dataFromJson.otherInpStr.at("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::ARRAY)
            inpVecStr = dataFromJson.otherInpVec.at("argin");

        try {
            switch (typeForDeviceData)
            {
            case Tango::DEV_VOID:
                break;
            case Tango::DEV_BOOLEAN: // ??? not boolean?
            {
                deviceData = getDeviceData<bool>(inpStr);
            }
            break;
            case Tango::DEV_SHORT:
            {
                deviceData = getDeviceData<short>(inpStr);
            }
            break;
            case Tango::DEV_LONG:
            {
                deviceData = getDeviceData<long>(inpStr);
            }
            break;
            case Tango::DEV_FLOAT:
            {
                deviceData = getDeviceData<float>(inpStr);
            }
            break;
            case Tango::DEV_DOUBLE:
            {
                deviceData = getDeviceData<double>(inpStr);
            }
            break;
            case Tango::DEV_USHORT:
            {
                deviceData = getDeviceData<unsigned short>(inpStr);
            }
            break;
            case Tango::DEV_ULONG:
            {
                deviceData = getDeviceData<unsigned long>(inpStr);
            }
            break;
            case Tango::DEV_STRING:
            {
                deviceData = generateDeviceDataFromArgin(inpStr);
            }
            break;
            case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
            {
                // ??? !!! FOR DEVVAR_CHARARRAY
                // ??? WHY unsigned char
                deviceData = getDeviceData<unsigned char>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_SHORTARRAY:
            {
                deviceData = getDeviceData<short>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_LONGARRAY:
            {
                deviceData = getDeviceData<long>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_FLOATARRAY:
            {
                deviceData = getDeviceData<float>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_DOUBLEARRAY:
            {
                deviceData = getDeviceData<double>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_USHORTARRAY:
            {
                deviceData = getDeviceData<unsigned short>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_ULONGARRAY:
            {
                deviceData = getDeviceData<unsigned long>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_STRINGARRAY:
            {
                deviceData = generateDeviceDataFromArgin(inpVecStr);
            }
            break;
            case Tango::DEVVAR_LONGSTRINGARRAY:
            {
                // ??? !!! NO
                throw std::runtime_error("DEVVAR_LONGSTRINGARRAY This format is not supported");
            }
            break;
            case Tango::DEVVAR_DOUBLESTRINGARRAY:
            {
                // ??? !!! NO
                throw std::runtime_error("DEVVAR_DOUBLESTRINGARRAY This format is not supported");
            }
            break;
            case Tango::DEV_STATE:
            {
                // // ??? !!! NO
                throw std::runtime_error("DEV_STATE This format is not supported");
            }
            break;
            case Tango::DEVVAR_BOOLEANARRAY:
            {
                // ??? !!! for bool array
                //deviceData = getDeviceData<bool>(inpVecStr);
                throw std::runtime_error("DEVVAR_BOOLEANARRAY This format is not supported");
            }
            break;
            case Tango::DEV_UCHAR:
            {
                // ??? !!! for uchar
                //deviceData = getDeviceData<unsigned char>(inpStr
                if (inpStr.size() != 1)
                    throw std::runtime_error("DEV_UCHAR Must be 1 char");
                Tango::DeviceData dOut;
                dOut << inpStr.c_str();

            }
            break;
            case Tango::DEV_LONG64:
            {
                deviceData = getDeviceData<Tango::DevLong64>(inpStr);
            }
            break;
            case Tango::DEV_ULONG64:
            {
                deviceData = getDeviceData<Tango::DevULong64>(inpStr);
            }
            break;
            case Tango::DEVVAR_LONG64ARRAY:
            {
                deviceData = getDeviceData<Tango::DevLong64>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_ULONG64ARRAY:
            {
                deviceData = getDeviceData<Tango::DevULong64>(inpVecStr);
            }
            break;
            default:
            {
            }
            break;
            }
        }
        catch (const boost::bad_lexical_cast &lc) {
            throw std::runtime_error(lc.what());
        }
        return deviceData;
    }

    Tango::DeviceAttribute TangoProcessor::getDeviceAttributeDataFromJson(const ParsedInputJson& dataFromJson, const Tango::AttributeInfoEx& attr_info, vector<string>& errors)
    {
        Tango::DeviceAttribute outDevAttr; outDevAttr.dim_x = 1; outDevAttr.dim_y = 2;
        outDevAttr.set_name(attr_info.name.c_str());

        string argin;
        vector<string> arginArray;

        TYPE_OF_VAL typeOfVaArgin = dataFromJson.check_key("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::VALUE)
            argin = dataFromJson.otherInpStr.at("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::ARRAY)
            arginArray = dataFromJson.otherInpVec.at("argin");

        int data_type = attr_info.data_type;
        Tango::AttrDataFormat data_format = attr_info.data_format;

        // By Default. dimX - size of arginArray
        int dimX = arginArray.size();
        int dimY = 0;
        
        if (data_format == Tango::AttrDataFormat::IMAGE)
            dimY = 1;

        if (dataFromJson.check_key("dimX") == TYPE_OF_VAL::VALUE) {
            dimX = stoi(dataFromJson.otherInpStr.at("dimX"));
        }
        if (dataFromJson.check_key("dimY") == TYPE_OF_VAL::VALUE) {
            dimY = stoi(dataFromJson.otherInpStr.at("dimY"));
        }       

        try {
            switch (data_type)
            {
            case Tango::DEV_VOID:
                break;
            case Tango::DEV_BOOLEAN:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<bool>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<bool>(arginArray, outDevAttr,dimX,dimY);
            }
            break;
            case Tango::DEV_SHORT:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<short>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<short>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_LONG:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<long>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<long>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_FLOAT:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<float>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<float>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_DOUBLE:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<double>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<double>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_USHORT:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<unsigned short>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<unsigned short>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_ULONG:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<unsigned long>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<unsigned long>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_STRING:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    outDevAttr << argin;
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    outDevAttr.insert(arginArray, dimX, dimY);
                
            }
            break;           
            case Tango::DEV_STATE:
            {
                // // ??? !!! NO
                throw std::runtime_error("DEV_STATE This format is not supported");
            }
            break;
            case Tango::DEV_UCHAR:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR) {
                    if (argin.size() != 1)
                        throw std::runtime_error("DEV_UCHAR Must be 1 char");
                    getDeviceAttribute<unsigned char>(argin, outDevAttr);
                }
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<unsigned char>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_LONG64:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<Tango::DevLong64>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<Tango::DevLong64>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            case Tango::DEV_ULONG64:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    getDeviceAttribute<Tango::DevULong64>(argin, outDevAttr);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    getDeviceAttributeImageOrSpectr<Tango::DevULong64>(arginArray, outDevAttr, dimX, dimY);
            }
            break;
            default:
            {
            }
            break;
            }
        }
        catch (const boost::bad_lexical_cast &lc) {
            throw std::runtime_error(lc.what());
        }

        return outDevAttr;
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
        if (type_req == TYPE_WS_REQ::PIPE_COMM) {
            opts = optsForPipeComm.equal_range(nameOfAttrOrComm);
        }

        return optsMap(opts);
    }

    void TangoProcessor::extractFromPipe(DevicePipe &pipe, stringstream &json, int dataType, std::pair<string, TYPE_WS_REQ> nameOfAttrAndTypeWsReq)
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
            forExtractingFromPipe<bool>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_SHORT:
        {
            forExtractingFromPipe<Tango::DevShort>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_LONG:
        {
            forExtractingFromPipe<Tango::DevLong>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_FLOAT:
        {
            forExtractingFromPipe<Tango::DevFloat>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_DOUBLE:
        {
            forExtractingFromPipe<Tango::DevDouble>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_USHORT:
        {
            forExtractingFromPipe<Tango::DevUShort>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_ULONG:
        {
            forExtractingFromPipe<Tango::DevULong>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_STRING:
        {
            forExtractingFromPipe<string>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
        {
            json << NONE;
        }
            break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            forExtractingFromPipe<short>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_LONGARRAY:
        {
            forExtractingFromPipe<Tango::DevLong>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            forExtractingFromPipe<float>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            forExtractingFromPipe<double>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            forExtractingFromPipe<unsigned short>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            forExtractingFromPipe<Tango::DevULong>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            forExtractingFromPipe<string>(pipe, json, nameOfAttrAndTypeWsReq, true);
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
            forExtractingFromPipe<bool>(pipe, json, nameOfAttrAndTypeWsReq, true);
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
            forExtractingFromPipe<Tango::DevLong64>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEV_ULONG64:
        {
            forExtractingFromPipe<Tango::DevULong64>(pipe, json, nameOfAttrAndTypeWsReq, false);
        }
            break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            forExtractingFromPipe<Tango::DevLong64>(pipe, json, nameOfAttrAndTypeWsReq, true);
        }
            break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            forExtractingFromPipe<Tango::DevULong64>(pipe, json, nameOfAttrAndTypeWsReq, true);
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
