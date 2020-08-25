#ifndef TANGO_PROCESSOR_H
#define TANGO_PROCESSOR_H

#include <tango.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "StringProc.h"
#include "common.h"
#include "TaskInfo.h"
#include "CurrentMode.h"
#include <boost/lexical_cast.hpp>

using std::string;
using std::vector;

namespace WebSocketDS_ns
{
    class GroupForWs;
    class DeviceForWs;
    class WsTangoCallback;
    struct ParsedInputJson;

    class TangoProcessor
    {
    public:
        TangoProcessor() = delete;
        TangoProcessor(TangoProcessor&) = delete;
#ifdef SERVER_MODE
        static string processUpdate(GroupForWs* groupForWs);
        static string processUpdate(DeviceForWs* deviceForWs);
#endif
        static vector<pair<long, TaskInfo>> processAsyncReq(GroupForWs* groupForWs, const ParsedInputJson& parsedInput, vector<string>& errorsFromGroupReq);

        static void processAsyncReq(Tango::DeviceProxy* deviceForWs, const ParsedInputJson& parsedInput, vector<pair<long, TaskInfo>>& taskReqList, const SINGLE_OR_GROUP& sog);

        static pair<long, TaskInfo> processCommand(Tango::DeviceProxy* deviceForWs, const ParsedInputJson& parsedInput, const SINGLE_OR_GROUP& sog);

        static pair<long, TaskInfo> processAttrRead(Tango::DeviceProxy* deviceForWs, const ParsedInputJson& parsedInput, const SINGLE_OR_GROUP& sog);

        static pair<long, TaskInfo> processAttrWrite(Tango::DeviceProxy* deviceForWs, const ParsedInputJson& parsedInput, const SINGLE_OR_GROUP& sog);

        static string processEvent(Tango::EventData * dt, const std::string& precOpt);

        static string processPipeRead(GroupForWs* groupForWs, const ParsedInputJson& parsedInput);
        static string processPipeRead(Tango::DevicePipe& pipe, const ParsedInputJson& parsedInput);

        static string processCommandResponse(Tango::DeviceProxy* dp, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion);
        static string processCommandResponse(Tango::Group* groupForWs, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion);

        static string processAttrReadResponse(Tango::DeviceProxy* dp, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion);
        static string processAttrReadResponse(Tango::Group* groupForWs, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion);

        static string processAttrWriteResponse(Tango::DeviceProxy* dp, const std::pair<long, TaskInfo>& idInfo);
        static string processAttrWriteResponse(Tango::Group* groupForWs, const std::pair<long, TaskInfo>& idInfo);

    private:
        static void _generateHeadForJson(std::stringstream& json, const string& type_req);
        static void _generateHeadForJson(std::stringstream& json, const string& type_req, const string& id_req, string deviceName = "");

        static void _generateHeadForJson(std::stringstream& json, const TaskInfo& reqInfo, bool useOldVersion);

        static void _generateJsonForAttrList(std::stringstream & json, std::vector<Tango::DeviceAttribute>* devAttrList, const std::unordered_map<std::string, std::string>& precOpts);
        static void _generateJsonForAttribute(std::stringstream & json, Tango::DeviceAttribute* devAttr, const std::string& precOpt);
        static void _deviceAttributeToStr(std::stringstream & json, Tango::DeviceAttribute* devAttr, const std::string& precOpt);

        static void _generateJsonForPipe(std::stringstream& json, Tango::DevicePipe& devPipe, const std::unordered_map<std::string, std::string>& precOpts);
        static void _extractFromPipe(std::stringstream& json, Tango::DevicePipe& devPipe, int dataType, const string& precOpt);

        static Tango::DeviceData _getDeviceDataFromParsedJson(const ParsedInputJson& parsedInput, int typeForDeviceData);

        static Tango::DeviceAttribute _getDeviceAttributeDataFromParsedJson(const ParsedInputJson& parsedInput, const Tango::AttributeInfoEx& attr_info);

        static void _generateJsonFromDeviceData(std::stringstream& json, Tango::DeviceData& deviceData, const string& precOpt);

        static bool isMassive(int inType);

    private:

        template <typename T>
        static void _devAttrToStrTmpl(std::stringstream & json, Tango::DeviceAttribute* devAttr, const std::string& precOpt) {
            Tango::AttrDataFormat format = devAttr->get_data_format();
            int type = devAttr->get_type();
            std::vector<T> dataVector, dataVectorFromSet;
            T data;
            Tango::DevState stateIn;
            string stateStr;

            if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE)
                json << "\"dimX\": " << devAttr->dim_x << ", ";
            if (format == Tango::AttrDataFormat::IMAGE)
                json << "\"dimY\": " << devAttr->dim_y << ", ";

            json << "\"data\": ";
            if (format == Tango::AttrDataFormat::SCALAR) {
                if (type == Tango::DEV_STATE) {
                    (*devAttr) >> stateIn;
                    stateStr = Tango::DevStateName[stateIn];
                    _dataValueToStr(json, stateStr, precOpt);
                }
                else {
                    (*devAttr) >> data;
                    _dataValueToStr(json, data, precOpt);

                    try {
                        devAttr->extract_set(dataVectorFromSet);

                        if (dataVectorFromSet.size() == 1) {
                            json << ", \"set\": ";
                            _dataValueToStr(json, dataVectorFromSet[0], precOpt);
                        }
                    }
                    catch (Tango::DevFailed &e) {}
                }
            }
            else
                if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
                    (*devAttr) >> dataVector;

                    int dim_x = devAttr->dim_x;
                    int dim_y = devAttr->dim_y;

                    if (!dim_y)
                        dim_y = 1;

                    // for writable attribute
                    // Read only Read_data (without Write_data)
                    if (dataVector.size() > dim_x*dim_y) {
                        dataVector.erase(dataVector.begin() + dim_x * dim_y, dataVector.end());
                    }

                    json << "[";
                    _dataArrayToStr(json, dataVector, precOpt);
                    json << "]";

                    try {
                        devAttr->extract_set(dataVectorFromSet);

                        if (dataVectorFromSet.size()) {
                            json << ", \"set\": [";
                            _dataArrayToStr(json, dataVectorFromSet, precOpt);
                            json << "]";
                        }
                    }
                    catch (Tango::DevFailed &e) {}
                }
        }

        template <typename T>
        static void _dataValueToStr(std::stringstream & json, const T& data, const string& precOpt) {
            if (is_floating_point<T>::value) {
                // default streamsize.
                std::streamsize srsz = std::stringstream().precision();

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

                string _pos = precOpt;
                vector<string> _po = StringProc::parseInputString(_pos, "=", true);

                string _poName = (_po.size() ? _po[0]: "");
                string _poVal = (_po.size() > 1 ? _po[1] : "");

                bool hasIosOpt = false;
                TYPE_IOS_OPT ios_opt;

                if (_poName == "prec") {
                    hasIosOpt = true;
                    ios_opt = TYPE_IOS_OPT::PREC;
                }
                else if (_poName == "precf") {
                    hasIosOpt = true;
                    ios_opt = TYPE_IOS_OPT::PRECF;
                }
                else if (_poName == "precs") {
                    hasIosOpt = true;
                    ios_opt = TYPE_IOS_OPT::PRECS;
                }

                if (!hasIosOpt) {
                    ios_opt = TYPE_IOS_OPT::PREC;
                }
                else {
                    srsz = get_srsz(_poVal);
                }

                _outForFloat(json, data, ios_opt, srsz);
                return;
            }
            else if (std::is_same<T, bool>::value) json << std::boolalpha << data;
            else if (std::is_same<T, const std::string>::value || std::is_same<T, std::string>::value) json << "\"" << data << "\"";
            else json << data;
        }

        template <typename T>
        static void _dataArrayToStr(std::stringstream & json, const std::vector<T>& dataVec, const string& precOpt) {
            bool begin = true;

            for (const T& data : dataVec) {
                if (!begin) json << ", ";
                else begin = false;
                _dataValueToStr(json, data, precOpt);
            }
        }

        // getting ios options for floating type
        template <typename T>
        static void _outForFloat(std::stringstream & json, const T& data, TYPE_IOS_OPT ios_opt, std::streamsize precIn = 0) {
            stringstream ss_in;

            if (precIn > 20 || precIn < 0)
                precIn = 0;

            if (ios_opt == TYPE_IOS_OPT::PREC)
                ss_in << std::setprecision(precIn) << data;
            else if (ios_opt == TYPE_IOS_OPT::PRECF)
                ss_in << std::fixed << std::setprecision(precIn) << data;
            else if (ios_opt == TYPE_IOS_OPT::PRECS)
                ss_in << std::scientific << std::setprecision(precIn) << data;
            json << ss_in.str();
        }

        template <typename T>
        static void _extractFromPipeTmpl(std::stringstream& json, Tango::DevicePipe& devPipe, const string& precOpt, bool isArray) {
            vector<T> valueArray;
            T value;

            try {
                if (isArray) {
                    devPipe >> valueArray;
                    json << "[";
                    _dataArrayToStr(json, valueArray, precOpt);
                    json << "]";
                }
                else {
                    devPipe >> value;
                    _dataValueToStr(json, value, precOpt);
                }
            }
            catch (Tango::DevFailed &df) {
                // TODO: MESSAGE FROM EXCEPTION?
                json << "\"Exception from TangoProcessor_bck::forExtractingFromPipe\"";
            }
        }

        template <typename T>
        static Tango::DeviceData _getDeviceDataTmpl(const string& inputStr) {
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
            return _generateDeviceDataFromArgin(inp);
        }

        template <typename T>
        static Tango::DeviceData _getDeviceDataTmpl(vector<string>& inputVecStr) {
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
            return _generateDeviceDataFromArgin(inpVec);
        }

        template <typename T>
        static void _getDataForDeviceAttribute(Tango::DeviceAttribute& devAttr, const string& inputStr) {
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
            devAttr << inp;
        }

        template <typename T>
        static void _getDataForDeviceAttribute(Tango::DeviceAttribute& devAttr, vector<string>& inputVecStr, int dimX, int dimY) {
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
                    inpVec.push_back(boost::lexical_cast<T>(val));
                }
            }

            devAttr.insert(inpVec, dimX, dimY);
        }

        template <typename T>
        static Tango::DeviceData _generateDeviceDataFromArgin(T fromArgin) {
            Tango::DeviceData dOut;
            dOut << fromArgin;
            return dOut;
        }

        template <typename T>
        static Tango::DeviceData _generateDeviceDataFromArgin(vector<T>& fromArgin) {
            Tango::DeviceData dOut;
            dOut << fromArgin;
            return dOut;
        }

        template <typename T>
        static void _generateJsonFromDeviceDataTmpl(std::stringstream& json, Tango::DeviceData& deviceData, const string& precOpt) {
            TYPE_OF_DEVICE_DATA type = typeOfData[deviceData.get_type()];

            std::vector<T> vecFromData;
            T data;

            if (type == TYPE_OF_DEVICE_DATA::DATA) {
                deviceData >> data;
            }
            else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
                deviceData >> vecFromData;
            }
            else if (type == TYPE_OF_DEVICE_DATA::VOID_D) {
                json << " \"OK\"";
            }

            if (type == TYPE_OF_DEVICE_DATA::DATA) {
                json << " ";
                _dataValueToStr(json, data, precOpt);
            }
            else if (type == TYPE_OF_DEVICE_DATA::ARRAY) {
                json << " [";
                _dataArrayToStr(json, vecFromData, precOpt);
                json << " ]";
            }
        }

    private:
        static const string attrQuality[];
        static const TYPE_OF_DEVICE_DATA typeOfData[];
    };
}    //    End of namespace

#endif   //    TANGO_PROCESSOR_H
