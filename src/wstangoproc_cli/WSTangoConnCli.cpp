#include "WSTangoConnCli.h"
#include "WebSocketDS_cli.h"

#include "StringProc.h"
#include "GroupForWs.h"
#include "UserControl.h"

#include "WSThread_plain.h"
#include "WSThread_tls.h"

#include "TangoProcessor.h"

#include "EventProcCli.h"
#include "ErrorInfo.h"


namespace WebSocketDS_ns
{
    WSTangoConnCli::WSTangoConnCli(WebSocketDS_cli* dev)
        : _wsds(dev)
    {
        uc = unique_ptr<UserControl>(new UserControl(dev->authDS));
        initOptions();
        try {
            logger = dev->get_logger();

            if (dev->secure) {
                wsThread = new WSThread_tls(this, dev->port, dev->certificate, dev->key);
            }
            else {
                wsThread = new WSThread_plain(this, dev->port);
            }
        }
        catch (...) {
            // TODO:
        }
    }

    // DONE: Убрано выполнение бинарных команд
    string WSTangoConnCli::sendRequest(const ParsedInputJson& parsedInput, ConnectionData *connData)
    {
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;

        if (
            typeWsReq == TYPE_WS_REQ::PIPE_COMM
            ) {
            return sendRequest_PipeComm(parsedInput, connData);
        }

        ErrorInfo err;
        err.typeofError = ERROR_TYPE::UNKNOWN_REQ_TYPE;
        err.errorMessage = "This request type is not supported";
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;

        return StringProc::exceptionStringOut(err);
    }

    string WSTangoConnCli::sendRequest_Event(websocketpp::connection_hdl hdl, const ParsedInputJson & parsedInput)
    {
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;

        // TODO: Так же исключения и дальше если eventProc != nullptr
        // Но для данного hdl ещё нет
        if (
            eventProc == nullptr
            &&
            (typeWsReq == TYPE_WS_REQ::EVENT_REQ_CHECK_DEV || typeWsReq == TYPE_WS_REQ::EVENT_REQ_OFF || typeWsReq == TYPE_WS_REQ::EVENT_REQ_REM_DEV)
            ) {
            ErrorInfo err;
            err.typeofError = ERROR_TYPE::NOT_SUBSCR_YET;
            err.errorMessage = "No subscribers yet";
            err.typeofReq = parsedInput.type_req_str;
            if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
                err.device_name = parsedInput.otherInpStr.at("device_name");
            }

            return StringProc::exceptionStringOut(err);
        }

        if (eventProc == nullptr && typeWsReq == TYPE_WS_REQ::EVENT_REQ_ADD_DEV) {
            eventProc = new EventProcCli(wsThread, _isOldVersionOfJson);
        }

        return eventProc->request(parsedInput, hdl);
    }

    void WSTangoConnCli::setFalsedConnectionStatus()
    {
        string status = "Port: " + to_string(_wsds->port) + "  already in use";
        _wsds->set_state(Tango::FAULT);
        _wsds->set_status(status);
        _connectionStatus = false;
    }

    void WSTangoConnCli::clientDisconnected(websocketpp::connection_hdl hdl)
    {
        if (eventProc == nullptr) {
            return;
        }
        eventProc->unsubscribeAllDev(hdl);
    }

    log4tango::Logger * WSTangoConnCli::get_logger(void)
    {
        return logger;
    }

    vector<pair<long, TaskInfo>> WSTangoConnCli::sendRequestAsync(const ParsedInputJson & parsedInput, ConnectionData * connData, vector<string>& errorsFromGroupReq)
    {
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;

        bool isGroupReq = false;
        if (
            parsedInput.check_key("group_request") == TYPE_OF_VAL::VALUE
            && parsedInput.otherInpStr.at("group_request") == "true"
            ) {
            isGroupReq = true;
        }

        string deviceName = parsedInput.otherInpStr.at("device_name");

        if (
            typeWsReq == TYPE_WS_REQ::COMMAND
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE
            ) {

            checkPermissionForRequest(parsedInput, connData, deviceName, isGroupReq);

            return _sendRequestAsyncDevOrGr(parsedInput, deviceName, isGroupReq, errorsFromGroupReq);
        }

        if (
            typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ
            ) {
            return _sendRequestAsyncDevOrGr(parsedInput, deviceName, isGroupReq, errorsFromGroupReq);
        }

        ErrorInfo err;
        err.typeofError = ERROR_TYPE::CHECK_CODE;
        err.errorMessage = "Check code. WSTangoConnCli::sendRequestAsync";
        err.typeofReq = parsedInput.type_req_str;
        err.id = parsedInput.id;

        // В обычном случае не возвращается никогда
        string errorMessage = StringProc::exceptionStringOut(err);
        throw std::runtime_error(errorMessage);
    }

    string WSTangoConnCli::checkResponse(const std::pair<long, TaskInfo>& idInfo)
    {
        TaskInfo taskInfo = idInfo.second;

        Tango::DeviceProxy* device = taskInfo.deviceForWs;
        string response;

        try {
            response = _forCheckResponse(idInfo, device);
            delete taskInfo.deviceForWs;
        }
        catch (Tango::AsynReplyNotArrived) {
            throw;
        }
        catch (Tango::DevFailed &e) {
            delete taskInfo.deviceForWs;
            throw;
        }
        return response;
    }

    void WSTangoConnCli::setNumOfConnections(unsigned long num)
    {
        _numOfConnections = num;
    }
    
    WSTangoConnCli::~WSTangoConnCli()
    {
        if (wsThread != nullptr) {
            wsThread->stop();
            void *ptr;
            wsThread->join(&ptr);
        }
    }
    vector<string> WSTangoConnCli::getDevOptions()
    {
        return _wsds->options;
    }

    // TODO: USE?
    // TODO: Возможность прямого использования псевдонимов
    // Проверяется, что используется либо стандартное имя, либо псевдоним.
    string WSTangoConnCli::getDeviceName(const ParsedInputJson & inputReq)
    {
        string device_name_from_alias;
        try {
            Tango::Database *db = Tango::Util::instance()->get_database();
            db->get_device_alias(inputReq.otherInpStr.at("device_name"), device_name_from_alias);
            return device_name_from_alias;
        }
        catch (Tango::DevFailed& e) {
            return inputReq.otherInpStr.at("device_name");
        }
    }

    Tango::DeviceProxy * WSTangoConnCli::_genDeviceForWs(string deviceName, const ParsedInputJson& parsedInput)
    {
        Tango::DeviceProxy *device;

        try {
            device = new Tango::DeviceProxy(deviceName);
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }

            ErrorInfo err;
            err.id = parsedInput.id;
            err.typeofReq = parsedInput.type_req_str;
            err.errorMessages = errors;
            err.device_name = deviceName;
            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;

            throw std::runtime_error(
                StringProc::exceptionStringOut(err)
            );
        }
        return device;
    }

    GroupForWs * WSTangoConnCli::_genGroupForWs(string devicePatt, const ParsedInputJson& parsedInput)
    {
        GroupForWs *group = nullptr;
        try {
            group = new GroupForWs(devicePatt);
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            ErrorInfo err;
            err.id = parsedInput.id;
            err.typeofReq = parsedInput.type_req_str;
            err.errorMessages = errors;
            err.device_name = devicePatt;
            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;

            throw std::runtime_error(
                StringProc::exceptionStringOut(err)
            );
        }
        return group;
    }

    vector<pair<long, TaskInfo>> WSTangoConnCli::_sendRequestAsyncDevOrGr(const ParsedInputJson & parsedInput, const string & deviceName, const bool& isGroupReq, vector<string>& errorsFromGroupReq)
    {
        if (isGroupReq) {
            GroupForWs* group = _genGroupForWs(deviceName, parsedInput);
            vector<pair<long, TaskInfo>> taskReqList = TangoProcessor::processAsyncReq(group, parsedInput, errorsFromGroupReq);
            delete group;
            return taskReqList;
        }
        else {
            Tango::DeviceProxy* device = _genDeviceForWs(deviceName, parsedInput);
            vector<pair<long, TaskInfo>> taskReqList;
            TangoProcessor::processAsyncReq(device, parsedInput, taskReqList, SINGLE_OR_GROUP::SINGLE);
            return taskReqList;
        }
    }

    string WSTangoConnCli::sendRequest_PipeComm(const ParsedInputJson & parsedInput, ConnectionData * connData)
    {
        string resp;
        bool isGroupReq = false;
        if (
            parsedInput.check_key("group_request") == TYPE_OF_VAL::VALUE
            && parsedInput.otherInpStr.at("group_request") == "true"
            ) {
            isGroupReq = true;
        }

        string deviceName = parsedInput.otherInpStr.at("device_name");
        if (isGroupReq) {
            // Если группа не создаётся, выбрасывается runtime_error
            GroupForWs* group = _genGroupForWs(deviceName, parsedInput);
            resp = TangoProcessor::processPipeRead(group, parsedInput);
            delete group;
            return resp;
        }
        else {
            // Если device не создаётся, выбрасывается runtime_error
            Tango::DeviceProxy* device = _genDeviceForWs(deviceName, parsedInput);
            string pipeName = parsedInput.otherInpStr.at("pipe_name");
            Tango::DevicePipe pipe = device->read_pipe(pipeName);
            resp = TangoProcessor::processPipeRead(pipe, parsedInput);
            delete device;
            return resp;
        }
    }
}
