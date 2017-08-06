#ifndef TANGO_PROCESSOR_H
#define TANGO_PROCESSOR_H


#include <tango.h>
#include <array>
#include <unordered_map>

#include <iomanip> // for setpecision

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include "ParsingInputJson.h"

using Tango::DevicePipe;

typedef std::unordered_map < std::string, std::string > stringmap;
typedef std::pair<stringmap::iterator, stringmap::iterator> stringmap_iter;

typedef std::unordered_multimap < std::string, std::string > stringunmap;
typedef std::pair<stringunmap::iterator, stringunmap::iterator> stringunmap_iter;

namespace WebSocketDS_ns
{
    class TangoProcessor
    {
    public:
        TangoProcessor();
        bool isMassive(int inType);

        string processPipe(DevicePipe& pipe, TYPE_WS_REQ pipeType);

        std::string process_attribute_t(Tango::DeviceAttribute& att, bool isShortAttr);

        Tango::DeviceData getDeviceDataFromParsedJson(const ParsedInputJson& dataFromJson, int typeForDeviceData);

        std::string getJsonStrFromDevData(Tango::DeviceData& devData, const ParsedInputJson &inputArgs);
        std::string getJsonStrFromGroupCmdReplyList(Tango::GroupCmdReplyList &replyList, const ParsedInputJson& parsedInput);

        pair<bool, string> checkOption(string nameOfAttrOrComm, string option, TYPE_WS_REQ type_req);
        void initOptionsForAttrOrComm(string nameAttrOrComm, const std::vector<string> &options, TYPE_WS_REQ type_req);

        std::string devAttrToStr(Tango::DeviceAttribute *attr);

        Tango::DeviceAttribute getDeviceAttributeDataFromJson(const ParsedInputJson& dataFromJson, const Tango::AttributeInfoEx& attr_info, vector<string>& errors);

    private:
        void initQualityNState();
        stringmap getOpts(string nameOfAttrOrComm, TYPE_WS_REQ type_req);

        // argout
        void generateArgoutForJson(Tango::DeviceData& devData, std::stringstream& json,const string& command_name);


        // FOR PIPE
        void extractFromPipe(DevicePipe& pipe, std::stringstream& json , int dataType, std::pair<string,TYPE_WS_REQ> nameOfAttrAndTypeWsReq);

        //EGOR
    public:
        std::string process_device_attribute_json(Tango::DeviceAttribute& data);
        std::string process_device_data_json(Tango::DeviceData& data);
        // Attributes
    private:
        enum class TYPE_OF_DEVICE_DATA { VOID_D = 0, DATA = 1, ARRAY = 2 };

        stringunmap optsForAttributes;
        stringunmap optsForCommands;
        stringunmap optsForPipe;
        stringunmap optsForPipeComm;
        std::array<string, 5> attrQuality;
        std::array<string, 14> tangoState;
        std::array<TYPE_OF_DEVICE_DATA,28> typeOfData;

    private: // templates
        template <typename T>
        Tango::DeviceData getDeviceDataFromDataType(const std::string& jsonData) {
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
        void generateStringJsonFromDevData(Tango::DeviceData &devData, std::stringstream& json, string command_name)
        {
            TYPE_OF_DEVICE_DATA type = typeOfData[devData.get_type()];

            std::vector<T> vecFromData;
            T data;

            if (type == TYPE_OF_DEVICE_DATA::DATA) {
                devData >> data;
            }
            else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
                devData >> vecFromData;
            }
            else if (type == TYPE_OF_DEVICE_DATA::VOID_D) {
                json << " \"OK\"";
            }

            if (type == TYPE_OF_DEVICE_DATA::DATA) {
                json << " ";
                dataFromAttrsOrCommToJson(data, json, TYPE_WS_REQ::COMMAND, command_name);
            }
            else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
                json << " [";
                dataArrayFromAttrOrCommToJson(vecFromData, json, TYPE_WS_REQ::COMMAND, command_name);
                json << " ]";
            }
        }

        template <typename T>
        Tango::DeviceData getDeviceDataFromArrayType(const std::string& jsonData) {
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
        void forExtractingFromPipe(DevicePipe &pipe, stringstream &json, std::pair<string, TYPE_WS_REQ> &nameOfAttrAndTypeWsReq, bool isArray)
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
                        dataFromAttrsOrCommToJson(tmpVal, json, nameOfAttrAndTypeWsReq.second, nameOfAttrAndTypeWsReq.first);
                    }
                    json << "]";
                }
                else {
                    pipe >> dt;
                    dataFromAttrsOrCommToJson(dt, json, nameOfAttrAndTypeWsReq.second, nameOfAttrAndTypeWsReq.first);
                }
            }
            catch (Tango::DevFailed &df) {
                json << "\"Exception from TangoProcessor::forExtractingFromPipe\"";
            }
        }

        template <typename T>
        Tango::DeviceData parsingJsonForGenerateData(const std::string& jsonData, int typeForDeviceData) {

            Tango::DeviceData dOut;

            TYPE_OF_DEVICE_DATA type = typeOfData[typeForDeviceData];

            if (type == TYPE_OF_DEVICE_DATA::DATA)
                dOut = getDeviceDataFromDataType<T>(jsonData);
            else if (type == TYPE_OF_DEVICE_DATA::ARRAY)
                dOut = getDeviceDataFromArrayType<T>(jsonData);

            return dOut;
        }

        template <typename T>
        Tango::DeviceData getDeviceData(string inputStr) {
            T inp;
            if (std::is_same<T, bool>::value) {
                // если не то и не другое будет кинуто исключение
                if (inputStr == "0" || inputStr == "false")
                    inp = false;
                else if (inputStr == "1" || inputStr == "true")
                    inp = true;
                else
                    inp = boost::lexical_cast<T>(inputStr);
            }
            else
                inp = boost::lexical_cast<T>(inputStr);
            return generateDeviceDataFromArgin(inp);
        }

        template <typename T>
        Tango::DeviceData getDeviceData(vector<string>& inputVecStr) {
            vector<T> inpVec;
            inpVec.reserve(inputVecStr.size());
            for (auto &val : inputVecStr) {
                if (std::is_same<T, bool>::value) {
                    // если не то и не другое будет кинуто исключение
                    T inp;
                    if (val == "0" || val == "false") {
                        inp = false;
                        inpVec.push_back(inp);
                    }
                    else if (val == "1" || val == "true") {
                        inp = true;
                        inpVec.push_back(inp);
                    }
                    else
                        inpVec.push_back(boost::lexical_cast<T>(val));                 
                }
                else
                    inpVec.push_back(boost::lexical_cast<T>(val));
            }
            return generateDeviceDataFromArgin(inpVec);
        }

        template <typename T>
        void getDeviceAttribute(string inputStr, Tango::DeviceAttribute& outDevAttr) {
            T inp;
            if (std::is_same<T, bool>::value) {
                // если не то и не другое будет кинуто исключение
                if (inputStr == "0" || inputStr == "false")
                    inp = false;
                else if (inputStr == "1" || inputStr == "true")
                    inp = true;
                else
                    inp = boost::lexical_cast<T>(inputStr);
            }
            else
                inp = boost::lexical_cast<T>(inputStr); 
            outDevAttr << inp;
        }

        template <typename T>
        void  getDeviceAttributeImageOrSpectr(vector<string>& inputVecStr, Tango::DeviceAttribute& outDevAttr, int dimX, int dimY) {
            vector<T> inpVec;
            inpVec.reserve(inputVecStr.size());
            for (auto &val : inputVecStr) {
                if (std::is_same<T, bool>::value) {
                    // если не то и не другое будет кинуто исключение
                    T inp;
                    if (val == "0" || val == "false") {
                        inp = false;
                        inpVec.push_back(inp);
                    }
                    else if (val == "1" || val == "true") {
                        inp = true;
                        inpVec.push_back(inp);
                    }
                    else
                        inpVec.push_back(boost::lexical_cast<T>(val));
                }
                else {
                    auto test = boost::lexical_cast<T>(val);
                    inpVec.push_back(boost::lexical_cast<T>(val));
                }
            }

            outDevAttr.insert(inpVec,dimX,dimY);
        }

        template <typename T>
        Tango::DeviceData generateDeviceDataFromArgin(T fromArgin) {
            Tango::DeviceData dOut;
            dOut << fromArgin;
            return dOut;
        }

        template <typename T>
        Tango::DeviceData generateDeviceDataFromArgin(vector<T>& fromArgin) {
            Tango::DeviceData dOut;
            dOut << fromArgin;
            return dOut;
        }

        template <typename T>
        std::string attrsToString(Tango::DeviceAttribute *attr){
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
                    dataFromAttrsOrCommToJson(stateStr, ss, TYPE_WS_REQ::ATTRIBUTE, nameAttr);
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
                    dataArrayFromAttrOrCommToJson(dataVector, ss, TYPE_WS_REQ::ATTRIBUTE, nameAttr);
                    ss << "]";
                }
            return ss.str();
        }

        template <typename T>
        void dataArrayFromAttrOrCommToJson(std::vector<T>& vecFromData, std::stringstream& json, TYPE_WS_REQ type_req, string nameOfAttrOrComm){
            bool begin = true;

            for (T fromData : vecFromData) {
                if (!begin) json << ", ";
                else begin = false;
                dataFromAttrsOrCommToJson(fromData, json, type_req, nameOfAttrOrComm);
            }
        }

        template <typename T>
        void dataFromAttrsOrCommToJson(T& data, std::stringstream& ss, TYPE_WS_REQ type_req, string nameOfAttrOrComm)
        {
            auto gettedOpts = getOpts(nameOfAttrOrComm, type_req);
            if (is_floating_point<T>::value) {
                // default streamsize.
                std::streamsize srsz = ss.precision();

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

        // getting ios options for floating type
        template <typename T>
        void outForFloat(T &data, stringstream &ss, TYPE_IOS_OPT ios_opt, std::streamsize precIn = 0) {

            if (precIn > 20 || precIn < 0)
                precIn = 0;

            if (ios_opt == TYPE_IOS_OPT::PREC)
                ss << std::setprecision(precIn) << data;
            if (ios_opt == TYPE_IOS_OPT::PRECF)
                ss << std::fixed << std::setprecision(precIn) << data;
            if (ios_opt == TYPE_IOS_OPT::PRECS)
                ss << std::scientific << std::setprecision(precIn) << data;
        }
    };
}    //    End of namespace

#endif   //    TANGO_PROCESSOR_H
