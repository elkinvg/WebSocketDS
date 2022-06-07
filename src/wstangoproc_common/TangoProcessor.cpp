#include "TangoProcessor.h"
#include "GroupForWs.h"

#ifdef SERVER_MODE
#include "DeviceForWs.h"
#endif

#include <sstream>
#include <string>
#include "common.h"
#include "ParsingInputJson.h"

#include "StringProc.h"

#include "ConnectionData.h"
#include "ResponseFromEvent.h"

#include "ErrorInfo.h"

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
        ErrorInfo err;
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;

        vector<pair<long, TaskInfo>> taskReqList;
#ifdef SERVER_MODE
        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
            if (!(parsedInput.check_key("group_request") == TYPE_OF_VAL::VALUE && parsedInput.otherInpStr.at("group_request") == "true")) {
                string deviceName = parsedInput.otherInpStr.at("device_name");
                Tango::DeviceProxy *dp;
                try {
                    dp = groupForWs->get_device(deviceName);
                    if (dp == NULL) {
                        err.errorMessage = "Device: " + deviceName + " not in group";
                        err.typeofError = ERROR_TYPE::DEVICE_NOT_IN_GROUP;
                        throw std::runtime_error(StringProc::exceptionStringOut(err));
                    }
                }
                catch (Tango::DevFailed &e) {
                    vector<string> tangoErrors;

                    for (unsigned int i = 0; i < e.errors.length(); i++) {
                        tangoErrors.push_back((string)e.errors[i].desc);
                    }

                    err.errorMessages = tangoErrors;
                    err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
                    err.device_name = deviceName;
                    throw std::runtime_error(StringProc::exceptionStringOut(err));
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
                err.errorMessages = tangoErrors;
                err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
                err.device_name = dev;

                string errorMessInJson = StringProc::exceptionStringOut(err);
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

        ErrorInfo err;
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;
        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
            err.device_name = parsedInput.otherInpStr.at("device_name");
        }

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
                    err.typeofError = ERROR_TYPE::CHECK_REQUEST;
                    err.errorMessage = "argin not found";                    
                    errorMessInJson = StringProc::exceptionStringOut(err);
                }

                // если argin - массив
                // и если требуемый type не является массивом
                else if (parsedInput.check_key("argin") == TYPE_OF_VAL::ARRAY && !isMassive(type)) {
                    err.typeofError = ERROR_TYPE::CHECK_REQUEST;
                    err.errorMessage = "The input data should not be an array";
                    errorMessInJson = StringProc::exceptionStringOut(err);
                }
                else {
                    Tango::DeviceData inDeviceData;
                    inDeviceData = _getDeviceDataFromParsedJson(parsedInput, type);
                    commId = deviceForWs->command_inout_asynch(commandName, inDeviceData);
                }
            }

            if (!errorMessInJson.size()) {
                UserOptions uo;
                // Проверка опции jsonout
                // Опция указывающая о запрете заключения вывода в кавычки
                uo.isJsonString = parsedInput.check_key(OPT_JSON_OUT) == TYPE_OF_VAL::VALUE;
                uo.precision = optStr;

                TaskInfo inf;
                inf.userOptions = uo;
                inf.idReq = parsedInput.id;
                inf.deviceName = deviceForWs->name();
                inf.reqName = commandName;
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

            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
            err.errorMessages = tangoErrors;
            errorMessInJson = StringProc::exceptionStringOut(err);
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
            UserOptions uo;
            // Проверка опции jsonout
            // Опция указывающая о запрете заключения вывода в кавычки
            uo.isJsonString = parsedInput.check_key(OPT_JSON_OUT) == TYPE_OF_VAL::VALUE;
            uo.precisions = optStr;


            TaskInfo inf;
            inf.userOptions = uo;
            inf.idReq = parsedInput.id;
            inf.deviceName = deviceForWs->name();
            inf.reqNames = attributes;
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

            ErrorInfo err;
            err.typeofReq = parsedInput.type_req_str;
            err.id = parsedInput.id;
            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
            err.errorMessages = tangoErrors;
            if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
                err.device_name = parsedInput.otherInpStr.at("device_name");
            }

            string errorMessInJson = StringProc::exceptionStringOut(err);
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

            UserOptions uo;

            TaskInfo inf;
            inf.userOptions = uo;
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

            ErrorInfo err;
            err.typeofReq = parsedInput.type_req_str;
            err.id = parsedInput.id;
            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
            err.errorMessages = tangoErrors;
            if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
                err.device_name = parsedInput.otherInpStr.at("device_name");
            }
            err.attr_name = attr_name;

            throw std::runtime_error(StringProc::exceptionStringOut(err));
        }
    }

#ifdef CLIENT_MODE
    string TangoProcessor::processEvent(MyEventData &eventData, const UserOptions& userOpt)
    {
        ErrorInfo err;
        string attrName;
        string deviceName;

        try {
            auto splt = StringProc::split(eventData.attrName, '/');

            if (splt.size() == 7) {
                attrName = splt[6];
            }
            else {
                attrName = eventData.attrName;
            }

            deviceName = eventData.deviceName;
        }
        catch (...) {}

        try {
            if (eventData.err) {
                vector<string> errors;

                for (int i = 0; i < eventData.errors.length(); i++) {
                    errors.push_back((string)eventData.errors[i].desc);
                }

                err.typeofError = ERROR_TYPE::EVENT_ERR;
                err.device_name = deviceName;
                err.errorMessages = errors;
                err.attr_name = attrName;
                err.event_type = eventData.eventType;


                return StringProc::exceptionStringOut(err);
            }

            stringstream json;
            json << "{\"event\": \"read\", ";
            json << "\"type_req\" : \"from_event\", ";
            json << "\"event_type\": \"" << eventData.eventType << "\", ";
            json << "\"timestamp\": " << eventData.tv_sec << ", ";
            json << "\"attr\": \"" << eventData.attrName << "\", ";

            string tmpAttrName = eventData.attrName;
            std::transform(tmpAttrName.begin(), tmpAttrName.end(), tmpAttrName.begin(), ::tolower);

            if (tmpAttrName.find("json") != std::string::npos) {
                Tango::DeviceAttribute *attr = &eventData.attr_value;
                string data;
                (*attr) >> data;
                json << "\"data\":" << data;
            }
            else {
                _deviceAttributeToStr(json, &eventData.attr_value, userOpt);
            }

            json << "}";
            return json.str();
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }

            ErrorInfo err;
            err.typeofReq = "from_event";
            err.errorMessages = errors;
            err.device_name = deviceName;
            err.attr_name = attrName;
            err.typeofError = ERROR_TYPE::EXC_FROM_EVENT_DEV;

            return StringProc::exceptionStringOut(err);
        }

        catch (...) {
            err.typeofError = ERROR_TYPE::EXC_FROM_EVENT_DEV;
            err.errorMessage = "Unknown exception from event";
            err.typeofReq = "from_event";
            err.device_name = deviceName;
            err.attr_name = attrName;
            return StringProc::exceptionStringOut(err);
        }
    }
#endif

#ifdef SERVER_MODE
    string TangoProcessor::processEvent(Tango::EventData * dt, const UserOptions& userOpt) {
        ErrorInfo err;
        string attrName;
        string deviceName;

        try {
            auto splt = StringProc::split(dt->attr_name, '/');

            if (splt.size() == 7) {
                attrName = splt[6];
            }
            else {
                attrName = dt->attr_name;
            }

            deviceName = dt->device->dev_name();
        }
        catch (...) {}

        try {
            if (dt->err) {
                vector<string> errors;

                for (int i = 0; i < dt->errors.length(); i++) {
                    errors.push_back((string)dt->errors[i].desc);
                }

                err.typeofError = ERROR_TYPE::EVENT_ERR;
                err.device_name = deviceName;
                err.errorMessages = errors;
                err.attr_name = attrName;
                err.event_type = dt->event;


                return StringProc::exceptionStringOut(err);
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
                _deviceAttributeToStr(json, dt->attr_value, userOpt);
            }

            json << "}";
            return json.str();
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }

            ErrorInfo err;
            err.typeofReq = "from_event";
            err.errorMessages = errors;
            err.device_name = deviceName;
            err.attr_name = attrName;
            err.typeofError = ERROR_TYPE::EXC_FROM_EVENT_DEV;

            return StringProc::exceptionStringOut(err);
        }

        catch (...) {
            err.typeofError = ERROR_TYPE::EXC_FROM_EVENT_DEV;
            err.errorMessage = "Unknown exception from event";
            err.typeofReq = "from_event";
            err.device_name = deviceName;
            err.attr_name = attrName;
            return StringProc::exceptionStringOut(err);
        }
    }
#endif

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
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
            err.id = parsedInput.id;
            err.errorMessage = "Device list not received";
            err.typeofReq = parsedInput.type_req_str;
            return StringProc::exceptionStringOut(err);
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

        if (!hasActDev) {
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::UNAVAILABLE_DEVS;
            err.id = parsedInput.id;
            err.errorMessage = "All device unavailable. Check the status of corresponding tango-server";
            err.typeofReq = parsedInput.type_req_str;
            return StringProc::exceptionStringOut(err);
        }

        ErrorInfo err;
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;
        err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
        err.errorMessages = errorsMess;
        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
            err.device_name = parsedInput.otherInpStr.at("device_name");
        }

        // TODO: CHECK. Проверить, при каких случаях вызывается здесь
        return StringProc::exceptionStringOut(err);
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
        _generateJsonFromDeviceData(json, deviceData, reqInfo.userOptions);
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
        _generateJsonFromDeviceData(json, deviceData, reqInfo.userOptions);
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
        _generateJsonForAttrList(json, devAttrs, reqInfo.userOptions.precisions);
        if (useOldVersion) {
            json << "}";
        }
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
        _generateJsonForAttrList(json, devAttrs, reqInfo.userOptions.precisions);
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
            if (att != attBegin)
                json << ", ";

            string nameAttr = att->get_name();
            string precOpt = (precOpts.find(nameAttr) != precOpts.end() ? precOpts.at(nameAttr) : "");
            UserOptions uo;
            uo.precision = precOpt;
            uo.isJsonString = false;
            _generateJsonForAttribute(json, &(*att), uo);
        }
        json << "}";
    }

    // Генерация JSON из DeviceAttribute
    void TangoProcessor::_generateJsonForAttribute(std::stringstream & json, Tango::DeviceAttribute* devAttr, const UserOptions& userOpt)
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
            _deviceAttributeToStr(json, devAttr, userOpt);

        json << "}";
    }

    void TangoProcessor::_deviceAttributeToStr(std::stringstream & json, Tango::DeviceAttribute* devAttr, const UserOptions& userOpt)
    {
        int type = devAttr->get_type();
        std::string out;
        switch (type) {
        case Tango::DEV_BOOLEAN:
        {
            _devAttrToStrTmpl<Tango::DevBoolean>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_SHORT:
        {
            _devAttrToStrTmpl<Tango::DevShort>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_LONG:
        {
            _devAttrToStrTmpl<Tango::DevLong>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_LONG64:
        {
            _devAttrToStrTmpl<Tango::DevLong64>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            _devAttrToStrTmpl<Tango::DevFloat>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            _devAttrToStrTmpl<Tango::DevDouble>(json, devAttr, userOpt);
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
                _dataValueToStr(json, tmp, userOpt);
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
                    _dataArrayToStr(json, tmpVec, userOpt);
                    json << "]";
                }
        }
        break;
        case Tango::DEV_USHORT:
        {
            _devAttrToStrTmpl<Tango::DevUShort>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_ULONG:
        {
            _devAttrToStrTmpl<Tango::DevULong>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            _devAttrToStrTmpl<Tango::DevULong64>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_STRING:
        {
            _devAttrToStrTmpl<std::string>(json, devAttr, userOpt);
        }
        break;
        case Tango::DEV_STATE:
        {
            _devAttrToStrTmpl<Tango::DevState>(json, devAttr, userOpt);
        }
        break;
#if TANGO_VERSION_MAJOR > 8
        case Tango::DEV_ENUM:
        {
            _devAttrToStrTmpl<Tango::DevEnum>(json, devAttr, userOpt);
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
            UserOptions uo;
            uo.precision = precOpt;
            uo.isJsonString = false;
            _extractFromPipe(json, devPipe, typeD, uo);
        }
        json << "}";
    }

    void TangoProcessor::_extractFromPipe(std::stringstream& json, Tango::DevicePipe& devPipe, int dataType, const UserOptions& userOpt)
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
            _extractFromPipeTmpl<Tango::DevBoolean>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_SHORT:
        {
            _extractFromPipeTmpl<Tango::DevShort>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_LONG:
        {
            _extractFromPipeTmpl<Tango::DevLong>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            _extractFromPipeTmpl<Tango::DevFloat>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            _extractFromPipeTmpl<Tango::DevDouble>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_USHORT:
        {
            _extractFromPipeTmpl<Tango::DevUShort>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_ULONG:
        {
            _extractFromPipeTmpl<Tango::DevULong>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_STRING:
        {
            _extractFromPipeTmpl<string>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEVVAR_CHARARRAY: // TODO: why not DEVVAR_CHARARRAY
        {
            json << NONE;
        }
        break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            _extractFromPipeTmpl<Tango::DevShort>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            _extractFromPipeTmpl<Tango::DevLong>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            _extractFromPipeTmpl<Tango::DevFloat>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            _extractFromPipeTmpl<Tango::DevDouble>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            _extractFromPipeTmpl<Tango::DevUShort>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            _extractFromPipeTmpl<Tango::DevULong>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            _extractFromPipeTmpl<string>(json, devPipe, userOpt, true);
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
            string stateStr;
            if (state < Tango::DevState::ON || state > Tango::DevState::UNKNOWN) {
                stateStr = Tango::DevStateName[Tango::DevState::UNKNOWN];
            }
            else {
                stateStr = Tango::DevStateName[state];
            }
            json << "\"" << stateStr << "\"";
        }
        break;
        case Tango::DEVVAR_BOOLEANARRAY:
        {
            _extractFromPipeTmpl<Tango::DevBoolean>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEV_UCHAR:
        {
            json << NONE;
            // TODO: _extractFromPipeTmpl<Tango::DevUChar>(pipe, json, false);
        }
        break;
        case Tango::DEV_LONG64:
        {
            _extractFromPipeTmpl<Tango::DevLong64>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            _extractFromPipeTmpl<Tango::DevULong64>(json, devPipe, userOpt, false);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            _extractFromPipeTmpl<Tango::DevLong64>(json, devPipe, userOpt, true);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            _extractFromPipeTmpl<Tango::DevULong64>(json, devPipe, userOpt, true);
        }
        break;
#if TANGO_VERSION_MAJOR > 8
        case Tango::DEV_ENUM:
        {
            _extractFromPipeTmpl<Tango::DevEnum>(json, devPipe, userOpt, false);
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

        ErrorInfo err;
        err.typeofReq = parsedInput.type_req_str;
        err.typeofError = ERROR_TYPE::NOT_SUPP;
        err.id = parsedInput.id;

        try {
            switch (typeForDeviceData)
            {
            case Tango::DEV_VOID:
                break;
            case Tango::DEV_BOOLEAN: // TODO: not boolean?
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
            case Tango::DEVVAR_CHARARRAY: // TODO: why not DEVVAR_CHARARRAY
            {
                // TODO: !!! FOR DEVVAR_CHARARRAY
                // TODO: WHY unsigned char
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
                err.errorMessage = "DEVVAR_LONGSTRINGARRAY This format is not supported";
                string errorMessInJson = StringProc::exceptionStringOut(err);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEVVAR_DOUBLESTRINGARRAY:
            {
                err.errorMessage = "DEVVAR_DOUBLESTRINGARRAY This format is not supported";
                string errorMessInJson = StringProc::exceptionStringOut(err);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEV_STATE:
            {
                err.errorMessage = "DEV_STATE This format is not supported";
                string errorMessInJson = StringProc::exceptionStringOut(err);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEVVAR_BOOLEANARRAY:
            {
                err.errorMessage = "DEVVAR_BOOLEANARRAY This format is not supported";
                string errorMessInJson = StringProc::exceptionStringOut(err);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEV_UCHAR:
            {
                if (inpStr.size() != 1) {
                    err.errorMessage = "DEV_UCHAR Must be 1 char";
                    err.typeofError = ERROR_TYPE::CHECK_REQUEST;
                    string errorMessInJson = StringProc::exceptionStringOut(err);
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
            err.errorMessage = lc.what();
            err.typeofError = ERROR_TYPE::IS_NOT_VALID;
            string errorMessInJson = StringProc::exceptionStringOut(err);
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

        ErrorInfo err;
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;

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
                err.errorMessage = "DEV_STATE This format is not supported";
                err.typeofError = ERROR_TYPE::NOT_SUPP;
                string errorMessInJson = StringProc::exceptionStringOut(err);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            case Tango::DEV_UCHAR:
            {
                if (data_format == Tango::AttrDataFormat::SCALAR) {
                    if (argin.size() != 1) {
                        err.errorMessage = "DEV_UCHAR Must be 1 char";
                        err.typeofError = ERROR_TYPE::CHECK_REQUEST;
                        string errorMessInJson = StringProc::exceptionStringOut(err);
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
                err.errorMessage = "Unknown type of atrribute";
                err.typeofError = ERROR_TYPE::NOT_SUPP;
                string errorMessInJson = StringProc::exceptionStringOut(err);
                throw std::runtime_error(errorMessInJson);
            }
            break;
            }
        }
        catch (const boost::bad_lexical_cast &lc) {
            err.errorMessage = lc.what();
            err.typeofError = ERROR_TYPE::IS_NOT_VALID;
            string errorMessInJson = StringProc::exceptionStringOut(err);
            throw std::runtime_error(errorMessInJson);
        }

        return outDevAttr;
    }

    void TangoProcessor::_generateJsonFromDeviceData(std::stringstream & json, Tango::DeviceData & deviceData, const UserOptions& userOpt)
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
            _dataValueToStr(json, bl, userOpt);
        }
        break;
        case Tango::DEV_SHORT:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevShort>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_LONG:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_FLOAT:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevFloat>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_DOUBLE:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevDouble>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_USHORT:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevUShort>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_ULONG:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_STRING:
        {
            _generateJsonFromDeviceDataTmpl<std::string>(json, deviceData, userOpt);
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
                _dataValueToStr(json, tmp, userOpt);
            }
            json << " ]";

        }
        break;
        case Tango::DEVVAR_SHORTARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevShort>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_LONGARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_FLOATARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevFloat>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_DOUBLEARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevDouble>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_USHORTARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevUShort>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_ULONGARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_STRINGARRAY:
        {
            _generateJsonFromDeviceDataTmpl<std::string>(json, deviceData, userOpt);
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
        {
            Tango::DevState stateIn;
            string stateStr;
            deviceData >> stateIn;
            if (stateIn < Tango::DevState::ON || stateIn > Tango::DevState::UNKNOWN) {
                stateStr = Tango::DevStateName[Tango::DevState::UNKNOWN];
            }
            else {
                stateStr = Tango::DevStateName[stateIn];
            }
            json << " \"" << stateStr << "\"";
        }
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
            _generateJsonFromDeviceDataTmpl<Tango::DevLong64>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEV_ULONG64:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong64>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_LONG64ARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevLong64>(json, deviceData, userOpt);
        }
        break;
        case Tango::DEVVAR_ULONG64ARRAY:
        {
            _generateJsonFromDeviceDataTmpl<Tango::DevULong64>(json, deviceData, userOpt);
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
            _generateJsonFromDeviceDataTmpl<Tango::DevEnum>(json, deviceData, userOpt);
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
