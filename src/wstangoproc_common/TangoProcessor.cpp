#include "TangoProcessor.h"
#include "GroupForWs.h"
#include "DeviceForWs.h"
#include <sstream>
#include <string>
#include "common.h"
#include "ParsingInputJson.h"

#include "StringProc.h"

#include "ConnectionData.h"
#include "ResponseFromEvent.h"

namespace WebSocketDS_ns
{
#ifdef SERVER_MODE
    string TangoProcessor::processUpdate(GroupForWs * groupForWs)
    {
        stringstream json;
        _generateHeadForJson(json, "group_attribute");
        vector<string> deviceList = groupForWs->getDeviceList();
        auto bgn = deviceList.begin();
        json << "{"; // BEGIN FOR DATA OBJECT
        for (auto it = bgn; it != deviceList.end(); ++it) {
            if (it != bgn) {
                json << ", ";
            }

            // name of device
            json << "\"" << *it << "\": ";
            std::vector<Tango::DeviceAttribute> *attrDevList = nullptr;
            try {
                attrDevList = groupForWs->getDeviceAttributeList(*it);
            }
            catch (...) {}

            if (attrDevList == nullptr) {
                json << "\"Device unavailable. Check the status of corresponding tango-server\"";
                continue;
            }
            _generateJsonForAttrList(json, attrDevList, groupForWs->getPrecisionOptionsForAttrs());
            if (attrDevList != nullptr) {
                delete attrDevList;
            }
        }
        json << "}"; // END FOR DATA OBJECT
        if (groupForWs->hasPipe()) {
            json << ", \"pipe\": {";
            for (auto it = bgn; it != deviceList.end(); ++it) {
                if (it != bgn) {
                    json << ", ";
                }

                // name of device
                json << "\"" << *it << "\": ";

                try {
                    Tango::DevicePipe devicePipe = groupForWs->getDevicePipe(*it);
                    _generateJsonForPipe(json, devicePipe, groupForWs->getPrecisionOptionsForAttrsFromPipes());
                } catch (Tango::DevFailed &e) {
                    json << "[";
                    for (int i = 0; i < e.errors.length(); i++) {
                        if (i > 0)
                            json << ", ";
                        json << "\"" << e.errors[i].desc << "\"";
                    }
                    json << "]";
                }

            }
            json << "}"; // END FOR PIPE OBJECT
        }
        json << "}"; // END FOR MAIN OBJECT
        return json.str();
    }

    string TangoProcessor::processUpdate(DeviceForWs * deviceForWs)
    {
        stringstream json;
        _generateHeadForJson(json, "attribute");
        vector<Tango::DeviceAttribute>* attrDevList = deviceForWs->getDeviceAttributeList();
        
        _generateJsonForAttrList(json, attrDevList, deviceForWs->getPrecisionOptionsForAttrs());

        if (attrDevList != nullptr) {
            delete attrDevList;
        }

        if (deviceForWs->hasPipe()) {
            json << ", \"pipe\":";
            try {
                Tango::DevicePipe devicePipe = deviceForWs->getDevicePipe();
                _generateJsonForPipe(json, devicePipe, deviceForWs->getPrecisionOptionsForAttrsFromPipes());
            }
            catch (Tango::DevFailed &e) {
                json << "[";
                for (int i = 0; i < e.errors.length(); i++) {
                    if (i > 0)
                        json << ", ";
                    json << "\"" << e.errors[i].desc << "\"";
                }
                json << "]";
            }
        }
        json << "}"; // END FOR MAIN OBJECT
        return json.str();
    }
#endif

    vector<pair<long, TaskInfo>> TangoProcessor::processAsyncReq(GroupForWs * groupForWs, const ParsedInputJson & parsedInput, vector<string>& errorsFromGroupReq)
    {
        vector<pair<long, TaskInfo>> taskReqList;
#ifdef SERVER_MODE
        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
            if (!(parsedInput.check_key("group_request") == TYPE_OF_VAL::VALUE && parsedInput.otherInpStr.at("group_request") == "true")) {
                string deviceName = parsedInput.otherInpStr.at("device_name");
                Tango::DeviceProxy *dp;
                try {
                    dp = groupForWs->get_device(deviceName);
                }
                catch (Tango::DevFailed &e) {
                    vector<string> tangoErrors;

                    for (unsigned int i = 0; i < e.errors.length(); i++) {
                        tangoErrors.push_back((string)e.errors[i].desc);
                    }

                    string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, tangoErrors, parsedInput.type_req_str);
                    throw std::runtime_error(errorMessInJson);
                }

                // DONE: Для девайсов из группы сделано SINGLE_OR_GROUP::SINGLE_FROM_GROUP
                // Для правильной сортировки задачи
                processAsyncReq(dp, parsedInput, taskReqList, SINGLE_OR_GROUP::SINGLE_FROM_GROUP);
                return taskReqList;
            }
        }
#endif
        vector<string> deviceList = groupForWs->getDeviceList();
        for (auto &dev : deviceList) {
            Tango::DeviceProxy *dp;
            try {
#ifdef SERVER_MODE
                dp = groupForWs->get_device(dev);
#endif // SERVER_MODE
#ifdef CLIENT_MODE
                // Для каждой задачи создаётся свой девайс из группы, для упрощения удаления
                dp = new Tango::DeviceProxy(dev);
#endif // CLIENT_MODE
            }
            catch (Tango::DevFailed &e) {
                vector<string> tangoErrors;

                for (unsigned int i = 0; i < e.errors.length(); i++) {
                    tangoErrors.push_back((string)e.errors[i].desc);
                }

                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, tangoErrors, parsedInput.type_req_str);
                errorsFromGroupReq.push_back(errorMessInJson);
                continue;
            }
            try {
                processAsyncReq(dp, parsedInput, taskReqList, SINGLE_OR_GROUP::GROUP);
            }
            catch (std::runtime_error &re) {
                errorsFromGroupReq.push_back(re.what());
            }
        }
        return taskReqList;
    }

    void TangoProcessor::processAsyncReq(Tango::DeviceProxy* deviceForWs, const ParsedInputJson& parsedInput, vector<pair<long, TaskInfo>>& taskReqList, const SINGLE_OR_GROUP& sog)
    {
        if (parsedInput.type_req == TYPE_WS_REQ::COMMAND) {
            taskReqList.push_back(
                processCommand(deviceForWs, parsedInput, sog)
            );
            return;
        }
        if (parsedInput.type_req == TYPE_WS_REQ::ATTRIBUTE_READ) {
            taskReqList.push_back(
                processAttrRead(deviceForWs, parsedInput, sog)
            );
            return;
        }
        if (parsedInput.type_req == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
            taskReqList.push_back(
                processAttrWrite(deviceForWs, parsedInput, sog)
            );
            return;
        }
    }

    pair<long, TaskInfo> TangoProcessor::processCommand(Tango::DeviceProxy* deviceForWs, const ParsedInputJson & parsedInput, const SINGLE_OR_GROUP& sog)
    {
        long commId = -1;
        pair<long, TaskInfo > procId;
        string errorMessInJson;

        // В режимах SERVER* дополнительные параметры команд 
        // прописываются в property
        // Здесь, при отправлении команды от клиента дополнительные выставляются
        // в имени команды.
        // Пример: CommandName;precf=15

        string commandName = parsedInput.otherInpStr.at("command_name");
        string optStr = StringProc::checkPrecisionOptions(commandName, parsedInput);

        try {
            Tango::CommandInfo info = deviceForWs->command_query(commandName);
            int type = info.in_type;
            if (type == Tango::DEV_VOID) {
                commId = deviceForWs->command_inout_asynch(commandName);
            }
            else {
                if (parsedInput.check_key("argin") == TYPE_OF_VAL::NONE) {
                    errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "argin not found", parsedInput.type_req_str);
                }

                // если argin - массив
                // и если требуемый type не является массивом
                else if (parsedInput.check_key("argin") == TYPE_OF_VAL::ARRAY && !isMassive(type)) {
                    errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "The input data should not be an array", parsedInput.type_req_str);
                }
                else {
                    Tango::DeviceData inDeviceData;
                    inDeviceData = _getDeviceDataFromParsedJson(parsedInput, type);
                    commId = deviceForWs->command_inout_asynch(commandName, inDeviceData);
                }
            }

            if (!errorMessInJson.size()) {

                TaskInfo inf;
                inf.idReq = parsedInput.id;
                inf.deviceName = deviceForWs->name();
                inf.reqName = commandName;
                inf.precision = optStr;
                inf.typeReqStr = parsedInput.type_req_str;
                inf.typeAsynqReq = TYPE_WS_REQ::COMMAND;
                inf.singleOrGroup = sog;
#ifdef CLIENT_MODE
                inf.deviceForWs = deviceForWs;
#endif

                procId = make_pair(
                    commId,
                    inf
                );
            }
        }
        catch (Tango::DevFailed &e) {
            vector<string> tangoErrors;

            for (unsigned int i = 0; i < e.errors.length(); i++) {
                tangoErrors.push_back((string)e.errors[i].desc);
            }

            errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, tangoErrors, parsedInput.type_req_str);
        }

        if (errorMessInJson.size()) {
            throw std::runtime_error(errorMessInJson);
        }
        
        return procId;
    }

    pair<long, TaskInfo> TangoProcessor::processAttrRead(Tango::DeviceProxy* deviceForWs, const ParsedInputJson & parsedInput, const SINGLE_OR_GROUP& sog)
    {
        long commId = -1;
        vector<string> attributes;
        pair<long, TaskInfo > procId;

        if (parsedInput.check_key("attr_name") == TYPE_OF_VAL::VALUE) {
            string attribute = parsedInput.otherInpStr.at("attr_name");
            attributes.push_back(attribute);
        }
        else if (parsedInput.check_key("attr_name") == TYPE_OF_VAL::ARRAY) {
            attributes = parsedInput.otherInpVec.at("attr_name");
        }

        unordered_map<string, string> optStr = StringProc::checkPrecisionOptions(attributes, parsedInput);

        try {
            commId = deviceForWs->read_attributes_asynch(attributes);
            TaskInfo inf;
            inf.idReq = parsedInput.id;
            inf.deviceName = deviceForWs->name();
            inf.reqNames = attributes;
            inf.precisions = optStr;
            inf.typeReqStr = parsedInput.type_req_str;
            inf.typeAsynqReq = TYPE_WS_REQ::ATTRIBUTE_READ;
            inf.singleOrGroup = sog;
#ifdef CLIENT_MODE
            inf.deviceForWs = deviceForWs;
#endif

            procId = make_pair(
                commId,
                inf
            );
        }
        catch (Tango::DevFailed &e) {
            vector<string> tangoErrors;

            for (unsigned int i = 0; i < e.errors.length(); i++) {
                tangoErrors.push_back((string)e.errors[i].desc);
            }

            string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, tangoErrors, parsedInput.type_req_str);
            throw std::runtime_error(errorMessInJson);
        }

        return procId;
    }

    pair<long, TaskInfo> TangoProcessor::processAttrWrite(Tango::DeviceProxy* deviceForWs, const ParsedInputJson & parsedInput, const SINGLE_OR_GROUP& sog)
    {
        string attr_name = parsedInput.otherInpStr.at("attr_name");
        try {
            Tango::AttributeInfoEx attr_info = deviceForWs->attribute_query(attr_name);
            Tango::DeviceAttribute inpDevAttr = _getDeviceAttributeDataFromParsedJson(parsedInput, attr_info);

            long commId = deviceForWs->write_attribute_asynch(inpDevAttr);

            pair<long, TaskInfo > procId;
            TaskInfo inf;
            inf.idReq = parsedInput.id;
            inf.deviceName = deviceForWs->name();
            inf.reqName = attr_name;
            inf.typeReqStr = parsedInput.type_req_str;
            inf.typeAsynqReq = TYPE_WS_REQ::ATTRIBUTE_WRITE;
            inf.singleOrGroup = sog;
#ifdef CLIENT_MODE
            inf.deviceForWs = deviceForWs;
#endif
            procId = make_pair(
                commId,
                inf
            );
            return procId;
        }
        catch (Tango::DevFailed &e) {
            vector<string> tangoErrors;

            for (unsigned int i = 0; i < e.errors.length(); i++) {
                tangoErrors.push_back((string)e.errors[i].desc);
            }

            string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, tangoErrors, parsedInput.type_req_str);
            throw std::runtime_error(errorMessInJson);
        }
    }

    string TangoProcessor::processEvent(Tango::EventData * dt, const  std::string& precOpt)
    {
        try {
            if (dt->err) {
                vector<string> errors;

                for (int i = 0; i < dt->errors.length(); i++) {
                    errors.push_back((string)dt->errors[i].desc);
                }

                ResponseFromEventReq errResp;

                auto splt = StringProc::split(dt->attr_name, '/');

                if (splt.size() == 7) {
                    errResp.attrName = splt[6];
                }
                else {
                    errResp.attrName = dt->attr_name;
                }

                errResp.deviceName = dt->device->name();
                errResp.respType = RESPONSE_TYPE::ERROR_M;
                errResp.errorMessages = errors;
                errResp.eventTypeStr = dt->event;

                return StringProc::exceptionStringOutForEvent(ERROR_TYPE::EVENT_ERR, vector<ResponseFromEventReq>({ errResp }));
            }

            stringstream json;
            json << "{\"event\": \"read\", ";
            json << "\"type_req\" : \"from_event\", ";
            json << "\"event_type\": \"" << dt->event << "\", ";
            json << "\"timestamp\": " << dt->get_date().tv_sec << ", ";
            json << "\"attr\": \"" << dt->attr_name << "\", ";

            string tmpAttrName = dt->attr_name;
            std::transform(tmpAttrName.begin(), tmpAttrName.end(), tmpAttrName.begin(), ::tolower);

            if (tmpAttrName.find("json") != std::string::npos) {
                Tango::DeviceAttribute *attr = dt->attr_value;
                string data;
                (*attr) >> data;
                json << "\"data\":" << data;
            }
            else {
                _deviceAttributeToStr(json, dt->attr_value, precOpt);
            }

            json << "}";
            return json.str();
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            return StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, errors, "exc_from_event_dev");
        }

        catch (...) {
            return StringProc::exceptionStringOut(ERROR_TYPE::UNKNOWN_EXC, "Unknown exception from event", "exc_from_event_dev");
        }
    }

    string TangoProcessor::processPipeRead(GroupForWs * groupForWs, const ParsedInputJson & parsedInput)
    {
        std::stringstream json;
        string pipeName = parsedInput.otherInpStr.at("pipe_name");

        _generateHeadForJson(json, parsedInput.type_req_str, parsedInput.id);

        vector<string> device_list;
        try {
            device_list = groupForWs->get_device_list(true);
        }
        catch (Tango::DevFailed &e) {
            return StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, "Device list not received", parsedInput.type_req_str);
        }

        unordered_map<string, string> optStr = StringProc::checkPrecisionOptions(parsedInput);

        bool fiter = true;
        bool hasActDev = false;
        bool hasPipe = false;
        vector<string> errorsMess;
        json << "{";
        for (auto& deviceFromGroup : device_list) {
            Tango::DeviceProxy *dp = groupForWs->get_device(deviceFromGroup);
            if (!fiter)
                json << ", ";
            fiter = false;

            json << "\"" << deviceFromGroup << "\": ";

            if (dp == 0) {
                string tmpErrMess = "Device unavailable. Check the status of corresponding tango-server";
                json << "\"" << tmpErrMess << "\"";
                errorsMess.push_back(deviceFromGroup + ": " + tmpErrMess);
                continue;
            }
            hasActDev = true;

            try {
                Tango::DevicePipe devicePipe = dp->read_pipe(pipeName);
                _generateJsonForPipe(json, devicePipe, optStr);
                hasPipe = true;
            }
            catch (Tango::DevFailed &e) {
                json << "[";
                string tmpErrMess;
                for (unsigned int i = 0; i < e.errors.length(); i++) {
                    if (i > 0) {
                        json << ", ";
                        tmpErrMess += " ||| ";
                    }
                    json << "\"" << e.errors[i].desc << "\"";
                    tmpErrMess += (string)e.errors[i].desc;
                }
                json << "]";
                errorsMess.push_back(deviceFromGroup + ": " + tmpErrMess);
            }
        }
        json << "}";
        json << "}";
        if (hasActDev && hasPipe)
            return json.str();

        if (!hasActDev)
            return StringProc::exceptionStringOut(ERROR_TYPE::UNAVAILABLE_DEVS, parsedInput.id, "All device unavailable. Check the status of corresponding tango-server", parsedInput.type_req_str);

        // TODO: CHECK. Проверить, при каких случаях вызывается здесь
        return StringProc::exceptionStringOut(ERROR_TYPE::TANGO_EXCEPTION, parsedInput.id, errorsMess, parsedInput.type_req_str);
    }

    string TangoProcessor::processPipeRead(Tango::DevicePipe& pipe, const ParsedInputJson & parsedInput)
    {
        std::stringstream json;
        string deviceName;

        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
            deviceName = parsedInput.otherInpStr.at("device_name");
        }

        _generateHeadForJson(json, parsedInput.type_req_str, parsedInput.id, deviceName);
        unordered_map<string, string> optStr = StringProc::checkPrecisionOptions(parsedInput);
        _generateJsonForPipe(json, pipe, optStr);
        json << "}";

        return json.str();
    }

    string TangoProcessor::processCommandResponse(Tango::DeviceProxy* dp, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion)
    {
        std::stringstream json;
        TaskInfo reqInfo = idInfo.second;
        Tango::DeviceData deviceData = dp->command_inout_reply(idInfo.first);
        _generateHeadForJson(json, reqInfo, useOldVersion);
        _generateJsonFromDeviceData(json, deviceData, reqInfo.precision);
        if (useOldVersion) {
            json << "}";
        }
        json << "}";

        return json.str();
    }

    string TangoProcessor::processCommandResponse(Tango::Group* groupForWs, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion)
    {
        std::stringstream json;
        string deviceName = idInfo.second.deviceName;
        TaskInfo reqInfo = idInfo.second;
        Tango::DeviceProxy *dp = groupForWs->get_device(deviceName);
        Tango::DeviceData deviceData = dp->command_inout_reply(idInfo.first);
        _generateHeadForJson(json, reqInfo, useOldVersion);
        _generateJsonFromDeviceData(json, deviceData, reqInfo.precision);
        if (useOldVersion) {
            json << "}";
            if (idInfo.second.singleOrGroup == SINGLE_OR_GROUP::GROUP) {
                json << "}";
            }
        }
        json << "}";

        return json.str();
    }

    string TangoProcessor::processAttrReadResponse(Tango::DeviceProxy* dp, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion)
    {
        std::stringstream json;
        TaskInfo reqInfo = idInfo.second;
        vector<Tango::DeviceAttribute> *devAttrs = dp->read_attributes_reply(idInfo.first);
        _generateHeadForJson(json, reqInfo, useOldVersion);
        _generateJsonForAttrList(json, devAttrs, reqInfo.precisions);
        json << "}";
        delete devAttrs;
        return json.str();
    }

    string TangoProcessor::processAttrReadResponse(Tango::Group* groupForWs, const std::pair<long, TaskInfo>& idInfo, bool useOldVersion)
    {
        std::stringstream json;
        string deviceName = idInfo.second.deviceName;
        TaskInfo reqInfo = idInfo.second;
        Tango::DeviceProxy *dp = groupForWs->get_device(deviceName);
        vector<Tango::DeviceAttribute> *devAttrs = dp->read_attributes_reply(idInfo.first);
        _generateHeadForJson(json, reqInfo, useOldVersion);
        _generateJsonForAttrList(json, devAttrs, reqInfo.precisions);
        if (useOldVersion) {
            json << "}";
        }
        json << "}";
        delete devAttrs;
        return json.str();
    }

    string TangoProcessor::processAttrWriteResponse(Tango::DeviceProxy* dp, const std::pair<long, TaskInfo>& idInfo)
    {
        std::stringstream json;
        TaskInfo reqInfo = idInfo.second;
        dp->write_attribute_reply(idInfo.first);

        _generateHeadForJson(json, reqInfo, false);
        json << "\"OK\"";
        json << "}";
        return json.str();
    }

    // TODO: Ошибки и данные отдельно
    string TangoProcessor::processAttrWriteResponse(Tango::Group* groupForWs, const std::pair<long, TaskInfo>& idInfo)
    {
        std::stringstream json;
        string deviceName = idInfo.second.deviceName;
        TaskInfo reqInfo = idInfo.second;
        Tango::DeviceProxy *dp = groupForWs->get_device(deviceName);
        dp->write_attribute_reply(idInfo.first);
        _generateHeadForJson(json, reqInfo, false);

        json << "\"OK\"";
        json << "}";
        return json.str();
    }

    void TangoProcessor::_generateJsonForAttrList(std::stringstream & json, std::vector<Tango::DeviceAttribute>* devAttrList, const std::unordered_map<std::string, std::string>& precOpts)
    {
        json << "{";
        auto attBegin = devAttrList->begin();
        for (auto att = attBegin; att != devAttrList->end(); ++att) {
            // Если задан niter для данного атрибута
            // Вывод будет только если iterator кратно nIters
            //Tango::DeviceAttribute att = attrList->at(i);

            // TODO: FOR NITER Доделать, или убрать
            //if (nIters.find(att.get_name()) != nIters.end()) {
            //    if (nIters[att.get_name()].first != 0) {
            //        if ((iterator + (nIters[att.get_name()].first - nIters[att.get_name()].second)) % nIters[att.get_name()].first != 0)
            //            continue;
            //    }
            //}
            if (att != attBegin)
                json << ", ";

            string nameAttr = att->get_name();
            string precOpt = (precOpts.find(nameAttr) != precOpts.end() ? precOpts.at(nameAttr) : "");
            _generateJsonForAttribute(json, &(*att), precOpt);
        }
        json << "}";
    }

    // Генерация JSON из DeviceAttribute
    void TangoProcessor::_generateJsonForAttribute(std::stringstream & json, Tango::DeviceAttribute* devAttr, const std::string& precOpt)
    {
        /*
        "attribute_name_KEY" : {
            "data": "data from attribute",
            "set": "set data from attribute"
        }
        */
        std::string quality = attrQuality[devAttr->get_quality()];
        json << "\"" << devAttr->get_name() << "\": {";
        if (quality != "VALID")
            json << "\"qual\": \"" << quality << "\", ";

        if (quality == "INVALID")
            json << "\"data\": " << NONE;
        else
            _deviceAttributeToStr(json, devAttr, precOpt);

        json << "}";
    }

    void TangoProcessor::_deviceAttributeToStr(std::stringstream & json, Tango::DeviceAttribute* devAttr, const std::string& precOpt)
    {
        int type = devAttr->get_type();
        std::string out;
        switch (type) {
        case Tango::DEV_BOOLEAN:
        {
            _devAttrToStrTmpl<Tango::DevBoolean>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_SHORT:
        {
            _devAttrToStrTmpl<Tango::DevShort>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_LONG:
        {
            _devAttrToStrTmpl<Tango::DevLong>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_LONG64:
        {
            _devAttrToStrTmpl<Tango::DevLong64>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            _devAttrToStrTmpl<Tango::DevFloat>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            _devAttrToStrTmpl<Tango::DevDouble>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_UCHAR:
        {
            Tango::AttrDataFormat format = devAttr->get_data_format();
            string nameAttr = devAttr->get_name();

            int dim_x = devAttr->dim_x;
            int dim_y = devAttr->dim_y;
            if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE)
                json << "\"dimX\": " << devAttr->dim_x << ", ";
            if (format == Tango::AttrDataFormat::IMAGE)
                json << "\"dimY\": " << devAttr->dim_y << ", ";

            json << "\"data\": ";
            Tango::DevUChar data;
            unsigned short tmp;
            vector<unsigned char> dataVector;
            vector<unsigned short> tmpVec;
            if (format == Tango::AttrDataFormat::SCALAR) {
                (*devAttr) >> data;
                tmp = (unsigned short)data;
                _dataValueToStr(json, tmp, precOpt);
            }
            else
                if (format == Tango::AttrDataFormat::SPECTRUM || format == Tango::AttrDataFormat::IMAGE) {
                    if (!dim_y)
                        dim_y = 1;
                    (*devAttr) >> dataVector;

                    int iter = 0;
                    for (auto& i : dataVector) {
                        // for writable attribute
                        // Read only Read_data (without Write_data)
                        if (iter >= dim_x * dim_y)
                            break;

                        tmpVec.push_back((unsigned short)i);
                        iter++;
                    }
                    json << "[";
                    _dataArrayToStr(json, tmpVec, precOpt);
                    json << "]";
                }
        }
        break;
        case Tango::DEV_USHORT:
        {
            _devAttrToStrTmpl<Tango::DevUShort>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_ULONG:
        {
            _devAttrToStrTmpl<Tango::DevULong>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            _devAttrToStrTmpl<Tango::DevULong64>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_STRING:
        {
            _devAttrToStrTmpl<std::string>(json, devAttr, precOpt);
        }
        break;
        case Tango::DEV_STATE:
        {
            _devAttrToStrTmpl<Tango::DevState>(json, devAttr, precOpt);
        }
        break;
#if TANGO_VERSION_MAJOR > 8
        case Tango::DEV_ENUM:
        {
            _devAttrToStrTmpl<Tango::DevEnum>(json, devAttr, precOpt);
        }
        break;
#endif
        case Tango::DEV_ENCODED:
        {
            json << "\"data\": " << "null";
        }
        break;
        default:
        {
            json << "\"data\": " << "null";
        }
        break;
        }
    }

    void TangoProcessor::_generateJsonForPipe(std::stringstream & json, Tango::DevicePipe & devPipe, const std::unordered_map<std::string, std::string>& precOpts)
    {
        size_t numElements = devPipe.get_data_elt_nb();

        json << "{";
        for (size_t i = 0; i < numElements; i++) {
            string nameOfAttr = devPipe.get_data_elt_name(i);
            int typeD = devPipe.get_data_elt_type(i);
            if (i)
                json << ", ";
            json << "\"" << nameOfAttr << "\": ";
            string precOpt = (precOpts.find(nameOfAttr) != precOpts.end() ? precOpts.at(nameOfAttr) : "");
            _extractFromPipe(json, devPipe, typeD, precOpt);
        }
        json << "}";
    }

    void TangoProcessor::_extractFromPipe(std::stringstream& json, Tango::DevicePipe& devPipe, int dataType, const string& precOpt)
    {
        switch (dataType)
        {
        case Tango::DEV_VOID:
        {
            json << NONE;
        }
        break;
        case Tango::DEV_BOOLEAN:
        {
            _extractFromPipeTmpl<Tango::DevBoolean>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_SHORT:
        {
            _extractFromPipeTmpl<Tango::DevShort>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_LONG:
        {
            _extractFromPipeTmpl<Tango::DevLong>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            _extractFromPipeTmpl<Tango::DevFloat>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            _extractFromPipeTmpl<Tango::DevDouble>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_USHORT:
        {
            _extractFromPipeTmpl<Tango::DevUShort>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_ULONG:
        {
            _extractFromPipeTmpl<Tango::DevULong>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_STRING:
        {
            _extractFromPipeTmpl<string>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
        {
            json << NONE;
        }
        break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            _extractFromPipeTmpl<Tango::DevShort>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            _extractFromPipeTmpl<Tango::DevLong>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            _extractFromPipeTmpl<Tango::DevFloat>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            _extractFromPipeTmpl<Tango::DevDouble>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            _extractFromPipeTmpl<Tango::DevUShort>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            _extractFromPipeTmpl<Tango::DevULong>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            _extractFromPipeTmpl<string>(json, devPipe, precOpt, true);
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
            devPipe >> state;
            json << "\"" << Tango::DevStateName[state] << "\"";
        }
        break;
        case Tango::DEVVAR_BOOLEANARRAY:
        {
            _extractFromPipeTmpl<Tango::DevBoolean>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEV_UCHAR:
        {
            json << NONE;
            // ??? _extractFromPipeTmpl<Tango::DevUChar>(pipe, json, false);
        }
        break;
        case Tango::DEV_LONG64:
        {
            _extractFromPipeTmpl<Tango::DevLong64>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            _extractFromPipeTmpl<Tango::DevULong64>(json, devPipe, precOpt, false);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            _extractFromPipeTmpl<Tango::DevLong64>(json, devPipe, precOpt, true);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            _extractFromPipeTmpl<Tango::DevULong64>(json, devPipe, precOpt, true);
        }
        break;
#if TANGO_VERSION_MAJOR > 8
        case Tango::DEV_ENUM:
        {
            _extractFromPipeTmpl<Tango::DevEnum>(json, devPipe, precOpt, false);
        }
        break;
#endif
        default:
        {
            json << NONE;
        }
        break;
        }
    }

    Tango::DeviceData TangoProcessor::_getDeviceDataFromParsedJson(const ParsedInputJson & parsedInput, int typeForDeviceData)
    {
        Tango::DeviceData deviceData;
        string inpStr;
        vector<string> inpVecStr;

        TYPE_OF_VAL typeOfVaArgin = parsedInput.check_key("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::VALUE)
            inpStr = parsedInput.otherInpStr.at("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::ARRAY)
            inpVecStr = parsedInput.otherInpVec.at("argin");

        try {
            switch (typeForDeviceData)
            {
            case Tango::DEV_VOID:
                break;
            case Tango::DEV_BOOLEAN: // ??? not boolean?
            {
                deviceData = _getDeviceDataTmpl<Tango::DevBoolean>(inpStr);
            }
            break;
            case Tango::DEV_SHORT:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevShort>(inpStr);
            }
            break;
            case Tango::DEV_LONG:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevLong>(inpStr);
            }
            break;
            case Tango::DEV_FLOAT:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevFloat>(inpStr);
            }
            break;
            case Tango::DEV_DOUBLE:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevDouble>(inpStr);
            }
            break;
            case Tango::DEV_USHORT:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevUShort>(inpStr);
            }
            break;
            case Tango::DEV_ULONG:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevULong>(inpStr);
            }
            break;
            case Tango::DEV_STRING:
            {
                deviceData = _generateDeviceDataFromArgin(inpStr);
            }
            break;
            case Tango::DEVVAR_CHARARRAY: // ??? why not DEVVAR_CHARARRAY
            {
                // ??? !!! FOR DEVVAR_CHARARRAY
                // ??? WHY unsigned char
                deviceData = _getDeviceDataTmpl<unsigned char>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_SHORTARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevShort>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_LONGARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevLong>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_FLOATARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevFloat>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_DOUBLEARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevDouble>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_USHORTARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevUShort>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_ULONGARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevULong>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_STRINGARRAY:
            {
                deviceData = _generateDeviceDataFromArgin(inpVecStr);
            }
            break;
            case Tango::DEVVAR_LONGSTRINGARRAY:
            {
                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP, parsedInput.id, "DEVVAR_LONGSTRINGARRAY This format is not supported", parsedInput.type_req_str);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEVVAR_DOUBLESTRINGARRAY:
            {
                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP, parsedInput.id, "DEVVAR_DOUBLESTRINGARRAY This format is not supported", parsedInput.type_req_str);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEV_STATE:
            {
                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP, parsedInput.id, "DEV_STATE This format is not supported", parsedInput.type_req_str);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEVVAR_BOOLEANARRAY:
            {
                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP, parsedInput.id, "DEVVAR_BOOLEANARRAY This format is not supported", parsedInput.type_req_str);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEV_UCHAR:
            {
                if (inpStr.size() != 1) {
                    string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "DEV_UCHAR Must be 1 char", parsedInput.type_req_str);
                    throw std::runtime_error(errorMessInJson);
                }
                Tango::DeviceData dOut;
                dOut << inpStr.c_str();

            }
            break;
            case Tango::DEV_LONG64:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevLong64>(inpStr);
            }
            break;
            case Tango::DEV_ULONG64:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevULong64>(inpStr);
            }
            break;
            case Tango::DEVVAR_LONG64ARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevLong64>(inpVecStr);
            }
            break;
            case Tango::DEVVAR_ULONG64ARRAY:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevULong64>(inpVecStr);
            }
            break;
#if TANGO_VERSION_MAJOR > 8
            case Tango::DEV_ENUM:
            {
                deviceData = _getDeviceDataTmpl<Tango::DevEnum>(inpStr);
            }
            break;
#endif
            default:
            {
            }
            break;
            }
        }
        catch (const boost::bad_lexical_cast &lc) {
            string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::IS_NOT_VALID, parsedInput.id, lc.what(), parsedInput.type_req_str);
            throw std::runtime_error(errorMessInJson);
        }
        return deviceData;
    }

    Tango::DeviceAttribute TangoProcessor::_getDeviceAttributeDataFromParsedJson(const ParsedInputJson & parsedInput, const Tango::AttributeInfoEx & attr_info)
    {
        Tango::DeviceAttribute outDevAttr;
        outDevAttr.set_name(attr_info.name.c_str());

        string argin;
        vector<string> arginArray;

        TYPE_OF_VAL typeOfVaArgin = parsedInput.check_key("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::VALUE)
            argin = parsedInput.otherInpStr.at("argin");
        if (typeOfVaArgin == TYPE_OF_VAL::ARRAY)
            arginArray = parsedInput.otherInpVec.at("argin");

        int data_type = attr_info.data_type;
        Tango::AttrDataFormat data_format = attr_info.data_format;

        // By Default. dimX - size of arginArray
        int dimX = arginArray.size();
        int dimY = 0;

        if (data_format == Tango::AttrDataFormat::IMAGE)
            dimY = 1;

        if (parsedInput.check_key("dimX") == TYPE_OF_VAL::VALUE) {
            dimX = stoi(parsedInput.otherInpStr.at("dimX"));
        }
        if (parsedInput.check_key("dimY") == TYPE_OF_VAL::VALUE) {
            dimY = stoi(parsedInput.otherInpStr.at("dimY"));
        }

        try {
            switch (data_type)
            {
            case Tango::DEV_VOID:
                break;
            case Tango::DEV_BOOLEAN:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevBoolean>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevBoolean>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_SHORT:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevShort>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevShort>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_LONG:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevLong>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevLong>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_FLOAT:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevFloat>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevFloat>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_DOUBLE:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevDouble>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevDouble>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_USHORT:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevUShort>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevUShort>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_ULONG:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevULong>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevULong>(outDevAttr, arginArray, dimX, dimY);
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
                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP, parsedInput.id, "DEV_STATE This format is not supported", parsedInput.type_req_str);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEV_UCHAR:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR) {
                    if (argin.size() != 1) {
                        string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::CHECK_REQUEST, parsedInput.id, "DEV_UCHAR Must be 1 char", parsedInput.type_req_str);
                        throw std::runtime_error(errorMessInJson);
                    }
                        
                    _getDataForDeviceAttribute<Tango::DevUChar>(outDevAttr, argin);
                }
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevUChar>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_LONG64:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevLong64>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevLong64>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
            case Tango::DEV_ULONG64:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR)
                    _getDataForDeviceAttribute<Tango::DevULong64>(outDevAttr, argin);
                if (data_format == Tango::AttrDataFormat::IMAGE || data_format == Tango::AttrDataFormat::SPECTRUM)
                    _getDataForDeviceAttribute<Tango::DevULong64>(outDevAttr, arginArray, dimX, dimY);
            }
            break;
#if TANGO_VERSION_MAJOR > 8
            case Tango::DEV_ENUM:
            {
                _getDataForDeviceAttribute<Tango::DevEnum>(outDevAttr, argin);
            }
            break;
#endif
            default:
            {
                string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::NOT_SUPP, parsedInput.id, "Unknown type of atrribute", parsedInput.type_req_str);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            }
        }
        catch (const boost::bad_lexical_cast &lc) {
            string errorMessInJson = StringProc::exceptionStringOut(ERROR_TYPE::IS_NOT_VALID, parsedInput.id, lc.what(), parsedInput.type_req_str);
            throw std::runtime_error(errorMessInJson);
        }

        return outDevAttr;
    }

    void TangoProcessor::_generateJsonFromDeviceData(std::stringstream & json, Tango::DeviceData & deviceData, const string & precOpt)
    {
        int type;
        try {
            type = deviceData.get_type();
            // TODO: CHECK получается так. Если не возвращает, то - 1
            if (type == -1) {
                type = Tango::DEV_VOID;
            }
        }
        catch (Tango::WrongData &e) {
            // TODO: NO!?
            type = Tango::DEV_VOID;
        }
        // TODO: throw exception and error_message?
        string noneComm = " \"This type Not supported\"";
        switch (type)
        {
        case Tango::DEV_VOID:
            json << " \"OK\"";
            break;
        case Tango::DEV_BOOLEAN:
        {
            Tango::DevBoolean bl;
            deviceData >> bl;
            json << " ";
            _dataValueToStr(json, bl, precOpt);
        }
        break;
        case Tango::DEV_SHORT:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevShort>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_LONG:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevFloat>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevDouble>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_USHORT:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevUShort>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_ULONG:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_STRING:
        {
            _generateJsonFromDeviceDataTmpl<std::string>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_CHARARRAY:
        {
            std::vector<unsigned char> vecFromData;
            deviceData >> vecFromData;
            json << " [";
            bool begin = true;
            for (const auto& fromData : vecFromData) {
                unsigned short int tmp;
                if (!begin) json << ", ";
                else begin = false;
                tmp = (unsigned short int)fromData;
                _dataValueToStr(json, tmp, precOpt);
            }
            json << " ]";

        }
        break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevShort>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevFloat>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevDouble>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevUShort>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            _generateJsonFromDeviceDataTmpl<std::string>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_LONGSTRINGARRAY:
            //        {
            //            Tango::DevLong parsed;
            //            deviceData = _generateJsonFromDeviceDataTmpl(jsonData,typeForDeviceData);
            //        }
            json << noneComm;
            break;
        case Tango::DEVVAR_DOUBLESTRINGARRAY:
            json << noneComm;
            break;
        case Tango::DEV_STATE:
            Tango::DevState stateIn;
            deviceData >> stateIn;
            json << " \"" << Tango::DevStateName[stateIn] << "\"";
            break;
        case Tango::DEVVAR_BOOLEANARRAY:
            //        {
            //            Tango::DevBoolean parsed;
            //            deviceData = _generateJsonFromDeviceDataTmpl(jsonData,typeForDeviceData);
            //        }
            json << noneComm;
            break;
        case Tango::DEV_UCHAR:
            //        {
            //            Tango::DevUChar parsed;
            //            deviceData = _generateJsonFromDeviceDataTmpl(jsonData,typeForDeviceData);
            //        }
            json << noneComm;
            break;
        case Tango::DEV_LONG64:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong64>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong64>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong64>(json, deviceData, precOpt);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong64>(json, deviceData, precOpt);
        }
        break;
        //case Tango::DEV_INT:
        //{
        //    int parsed;
        //    deviceData = _generateJsonFromDeviceDataTmpl(jsonData,typeForDeviceData);
        //}
        //            break;
        ////        case Tango::DEV_ENCODED:
        ////            json << devEncodedToStr(&data);
        ////            break;
#if TANGO_VERSION_MAJOR > 8
        case Tango::DEV_ENUM:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevEnum>(json, deviceData, precOpt);
        }
        break;
#endif
        default:
            json << noneComm;
            break;
        }
    }

    bool TangoProcessor::isMassive(int inType)
    {
        TYPE_OF_DEVICE_DATA td = typeOfData[inType];
        if (td == TYPE_OF_DEVICE_DATA::ARRAY) return true;
        else return false;
    }

    void TangoProcessor::_generateHeadForJson(std::stringstream & json, const string & type_req)
    {
        json << "{\"event\": \"read\", \"type_req\": \"" << type_req << "\", \"data\":";
    }

    void TangoProcessor::_generateHeadForJson(std::stringstream & json, const string & type_req, const string & id_req, string deviceName)
    {
        json << "{\"event\": \"read\", \"type_req\": \"" << type_req << "\",";
        if (deviceName.size()) {
            json << " \"device_name\": \"" << deviceName << "\",";
        }
        // ID MUST BE VALUE
        try {
            auto idTmp = stoi(id_req);
            json << " \"id_req\": " << idTmp << ",";
        }
        catch (...) {}

        json << " \"data\": ";
    }

    void TangoProcessor::_generateHeadForJson(std::stringstream & json, const TaskInfo& reqInfo, bool useOldVersion)
    {
        json << "{\"event\": \"read\", \"type_req\": \"" << reqInfo.typeReqStr << "\",";
        if (!useOldVersion) {
            if (reqInfo.deviceName.size()) {
                json << "\"device_name\": \"" << reqInfo.deviceName << "\", ";
            }
        
            if (reqInfo.typeAsynqReq == TYPE_WS_REQ::COMMAND) {
                json << "\"command_name\": \"" << reqInfo.reqName << "\", ";
            }
        }
        if (reqInfo.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
            json << "\"attr_name\": \"" << reqInfo.reqName << "\", ";
        }
        // ID MUST BE VALUE
        try {
            auto idTmp = stoi(reqInfo.idReq);
            json << " \"id_req\": " << idTmp << ",";
        }
        catch (...) {}

        if (reqInfo.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
            json << " \"resp\": ";
        }
        else {
            json << " \"data\": ";
        }
        if (useOldVersion) {
            if (reqInfo.typeAsynqReq == TYPE_WS_REQ::COMMAND) {
                json << "{";
                json << "\"command_name\": \"" << reqInfo.reqName << "\", ";
                if (useOldVersion && reqInfo.singleOrGroup != SINGLE_OR_GROUP::GROUP) {
                    json << "\"device_name\": \"" << reqInfo.deviceName << "\", ";
                }
                json << "\"argout\":";
                if (useOldVersion && reqInfo.singleOrGroup == SINGLE_OR_GROUP::GROUP) {
                    json << "{";
                    json << "\"" << reqInfo.deviceName << "\": ";
                }
            }
            if (reqInfo.typeAsynqReq == TYPE_WS_REQ::ATTRIBUTE_READ) {
                json << "{";
                json << "\"" << reqInfo.deviceName << "\":";
            }
        }
    }

    const string TangoProcessor::attrQuality[] = {
           "VALID",     // Tango::AttrQuality::ATTR_VALID
           "INVALID",   // Tango::AttrQuality::ATTR_INVALID
           "ALARM",     // Tango::AttrQuality::ATTR_ALARM
           "CHANGING",  // Tango::AttrQuality::ATTR_CHANGING
           "WARNING"    // Tango::AttrQuality::ATTR_WARNING
    };

    const /*TangoProcessor::*/TYPE_OF_DEVICE_DATA TangoProcessor::typeOfData[] = {
        TYPE_OF_DEVICE_DATA::VOID_D, // Tango::DEV_VOID

        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_BOOLEAN
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_SHORT
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_LONG
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_FLOAT
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_DOUBLE
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_USHORT
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_ULONG
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_STRING

        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_CHARARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_SHORTARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_LONGARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_FLOATARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_DOUBLEARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_USHORTARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_ULONGARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_STRINGARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_LONGSTRINGARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_DOUBLESTRINGARRAY

        TYPE_OF_DEVICE_DATA::NOTSUPPORTED, // Tango::DEV_STATE
        TYPE_OF_DEVICE_DATA::NOTSUPPORTED, // Tango::CONST_DEV_STRING]

        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_BOOLEANARRAY

        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_UCHAR
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_LONG64
        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_ULONG64
        
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_LONG64ARRAY
        TYPE_OF_DEVICE_DATA::ARRAY, // Tango::DEVVAR_ULONG64ARRAY

        TYPE_OF_DEVICE_DATA::DATA, // Tango::DEV_INT
        TYPE_OF_DEVICE_DATA::NOTSUPPORTED // Tango::DEV_ENCODED
#if TANGO_VERSION_MAJOR > 8
        ,
        TYPE_OF_DEVICE_DATA::DATA // 
#endif
    };
}
