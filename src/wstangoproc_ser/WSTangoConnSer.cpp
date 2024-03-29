#include "WSTangoConnSer.h"
#include "WebSocketDS.h"

#include "StringProc.h"
#include "GroupForWs.h"
#include "DeviceForWs.h"
#include "ConnectionData.h"
#include "UserControl.h"

#include "WSThread_plain.h"
#include "WSThread_tls.h"

#include "EventProcSer.h"

#include "TangoProcessor.h"
#include "ErrorInfo.h"


namespace WebSocketDS_ns
{
    WSTangoConnSer::WSTangoConnSer(WebSocketDS * dev)
        : _wsds(dev)
    {
        uc = unique_ptr<UserControl>(new UserControl(dev->authDS));
        try {
            logger = dev->get_logger();
        } catch (...){}

        initOptions();
        initDeviceServer();
        try {

            if (dev->secure) {
                wsThread = new WSThread_tls(this, dev->port, dev->certificate, dev->key);
            }
            else {
                wsThread = new WSThread_plain(this, dev->port);
            }

            if (_hasAttrsForUpdate) {
                updTh = new thread(&WSTangoConnSer::updateData, this);
            }

            // Проверка наличия подписчика на события в Property
            // true, если есть хотя бы одна подписка
            auto hasEventSubscr = [](const array<vector<string>, 4> &event_subcr) {
                for (const auto& ev : event_subcr) {
                    if (ev.size())
                        return true;
                }
                return false;
            };

            list_of_event_subcr = {
                dev->list_subscr_event_change,
                dev->list_subscr_event_periodic,
                dev->list_subscr_event_user,
                dev->list_subscr_event_archive
            };

            _hasEventSubscr = hasEventSubscr(list_of_event_subcr);
        }
        catch (...) {}
    }

    // DONE: Убрано выполнение бинарных команд
    string WSTangoConnSer::sendRequest(const ParsedInputJson & parsedInput, ConnectionData * connData)
    {
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;

        ErrorInfo err;
        err.id = parsedInput.id;

        if (!_isInitDs) {
            err.errorMessage = _errorMessage;
            err.typeofReq = parsedInput.type_req_str;
            err.typeofError = ERROR_TYPE::INIT_FAILED;
            return StringProc::exceptionStringOut(err);
        }

        if (
            typeWsReq == TYPE_WS_REQ::UNKNOWN
            ) {
            err.typeofReq = parsedInput.type_req_str;
            err.errorMessage = "This request type is not supported in the current mode";
            err.typeofError = ERROR_TYPE::UNKNOWN_REQ_TYPE;
            return StringProc::exceptionStringOut(err);
        }

        if (
            typeWsReq == TYPE_WS_REQ::PIPE_COMM
            ) {
            return sendRequest_PipeComm(parsedInput, connData);
        }

        // В обычном случае не возвращается никогда
        err.typeofReq = parsedInput.type_req_str;
        err.errorMessage = "Unknown Request";
        err.typeofError = ERROR_TYPE::UNKNOWN_REQ_TYPE;
        return StringProc::exceptionStringOut(err);
    }

    vector<pair<long, TaskInfo>> WSTangoConnSer::sendRequestAsync(const ParsedInputJson& parsedInput, ConnectionData *connData, vector<string>& errorsFromGroupReq)
    {
        
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;
        string deviceName;

        ErrorInfo err;
        err.id = parsedInput.id;

        if (!_isInitDs) {
            err.errorMessage = _errorMessage;
            err.typeofReq = parsedInput.type_req_str;
            err.typeofError = ERROR_TYPE::INIT_FAILED;
            string errorMessage = StringProc::exceptionStringOut(err);
            throw std::runtime_error(errorMessage);
        }

        if (
            typeWsReq == TYPE_WS_REQ::COMMAND
            || typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE
            ) {

            if (_isGroup) {
                bool isGroupReq;
                if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
                    deviceName = parsedInput.otherInpStr.at("device_name");
                    isGroupReq = false;
                }
                else {
                    deviceName = _wsds->deviceServer;
                    isGroupReq = true;
                }
                checkPermissionForRequest(parsedInput, connData, deviceName, isGroupReq);
                return TangoProcessor::processAsyncReq(groupForWs, parsedInput, errorsFromGroupReq);
            }
            else {
                deviceName = _wsds->deviceServer;
                checkPermissionForRequest(parsedInput, connData, deviceName, false);
                vector<pair<long, TaskInfo>> taskReqList;
                TangoProcessor::processAsyncReq(deviceForWs, parsedInput, taskReqList, SINGLE_OR_GROUP::SINGLE);
                return taskReqList;
            }
        }

        if (
            typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ
            ) {
            if (_isGroup) {
                return TangoProcessor::processAsyncReq(groupForWs, parsedInput, errorsFromGroupReq);
            }
            else {
                vector<pair<long, TaskInfo>> taskReqList;
                TangoProcessor::processAsyncReq(deviceForWs, parsedInput, taskReqList, SINGLE_OR_GROUP::SINGLE);
                return taskReqList;
            }
        }

        // В обычном случае не возвращается никогда
        err.typeofReq = parsedInput.type_req_str;
        err.errorMessage = "Unknown Request";
        err.typeofError = ERROR_TYPE::UNKNOWN_REQ_TYPE;
        string errorMessage = StringProc::exceptionStringOut(err);
        throw std::runtime_error(errorMessage);
    }

    log4tango::Logger * WSTangoConnSer::get_logger(void)
    {
        return logger;
    }

    string WSTangoConnSer::checkResponse(const std::pair<long, TaskInfo>& idInfo)
    {
        if (_isGroup) {
            return _forCheckResponse(idInfo, groupForWs);
        }
        else {
            return _forCheckResponse(idInfo, deviceForWs);
        }
    }

    void WSTangoConnSer::setNumOfConnections(unsigned long num)
    {
        _numOfConnections = num;

        // Подписка на события, только если есть подключения
        if (_numOfConnections && _hasEventSubscr && eventSubscr == nullptr) {
            initEventSubscr();
        }

        if (!_numOfConnections && _hasEventSubscr && eventSubscr != nullptr) {
            deleteEventSubscr();
        }
    }

    void WSTangoConnSer::setFalsedConnectionStatus()
    {
        string status = "Port: " + to_string(_wsds->port) + "  already in use";
        _wsds->set_state(Tango::FAULT);
        _wsds->set_status(status);
        _connectionStatus = false;
    }

    void WSTangoConnSer::update()
    {
        if (_numOfConnections) {
            DEBUG_STREAM << _numOfConnections << " CONNECTIONS";
        }
        else {
            DEBUG_STREAM << "NO CONNECTIONS";
        }

        if (!_hasAttrsForUpdate) {
            return;
        }
        do_update = true;
        _upd_cond.notify_one();
    }

    string WSTangoConnSer::sendRequest_PipeComm(const ParsedInputJson & parsedInput, ConnectionData * connData)
    {
        TYPE_WS_REQ typeWsReq = parsedInput.type_req;
        ErrorInfo err;
        err.id = parsedInput.id;
        err.typeofReq = parsedInput.type_req_str;

        // Для режима одного девайса read_pipe
        if (!_isGroup && parsedInput.type_req != TYPE_WS_REQ::PIPE_COMM) {
            err.typeofError = ERROR_TYPE::IS_NOT_VALID;
            err.errorMessage = "type_req must be read_pipe";
            return StringProc::exceptionStringOut(err);
        }

        string deviceName;
        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE) {
            deviceName = parsedInput.otherInpStr.at("device_name");
            err.device_name = parsedInput.otherInpStr.at("device_name");
        }

        try {
            if (_isGroup) {
                if (
                    (typeWsReq == TYPE_WS_REQ::PIPE_COMM
                        || typeWsReq == TYPE_WS_REQ::PIPE_COMM_DEV)
                    && deviceName.size()
                    ) {

                    string pipeName = parsedInput.otherInpStr.at("pipe_name");
                    Tango::DeviceProxy *dp = groupForWs->get_device(deviceName);

                    if (dp == NULL) {
                        err.typeofError = ERROR_TYPE::DEVICE_NOT_IN_GROUP;
                        err.errorMessage = "Device: " + deviceName + " not in group";
                        err.typeofReq = parsedInput.type_req_str;
                        return StringProc::exceptionStringOut(err);
                    }
                    Tango::DevicePipe pipe = dp->read_pipe(pipeName);
                    return TangoProcessor::processPipeRead(pipe, parsedInput);
                }

                return TangoProcessor::processPipeRead(groupForWs, parsedInput);
            }
            else {
                deviceName = _wsds->deviceServer;
                string pipeName = parsedInput.otherInpStr.at("pipe_name");
                Tango::DevicePipe pipe = deviceForWs->read_pipe(pipeName);
                return TangoProcessor::processPipeRead(pipe, parsedInput);
            }
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            err.typeofError = ERROR_TYPE::TANGO_EXCEPTION;
            err.errorMessages = errors;
            return StringProc::exceptionStringOut(err);
        }

        catch (...) {
            err.typeofError = ERROR_TYPE::UNKNOWN_EXC;
            err.errorMessage = "Unknown exception";
            return StringProc::exceptionStringOut(err);
        }
    }

    void WSTangoConnSer::initEventSubscr()
    {
        try {
            if (_isGroup) {
                eventSubscr = new EventProcSer(wsThread, groupForWs, list_of_event_subcr);
            }
            else {
                eventSubscr = new EventProcSer(wsThread, deviceForWs, list_of_event_subcr);
            }
        }
        catch (...) {}
    }

    void WSTangoConnSer::deleteEventSubscr()
    {
        delete eventSubscr;
        eventSubscr = nullptr;
    }

    WSTangoConnSer::~WSTangoConnSer()
    {
        if (_hasEventSubscr && eventSubscr != nullptr) {
            deleteEventSubscr();
        }

        do_update = false;
        local_th_exit = true;
        _upd_cond.notify_one();

        if (updTh != nullptr) {
            updTh->join();
            delete updTh;
        }

        if (wsThread != nullptr) {
            wsThread->stop();
            void *ptr;
            wsThread->join(&ptr);
        }

        if (deviceForWs != nullptr) {
            delete deviceForWs;
        }
        if (groupForWs != nullptr) {
            delete groupForWs;
        }
    }

    vector<string> WSTangoConnSer::getDevOptions()
    {
        return _wsds->options;
    }

    void WSTangoConnSer::updateData()
    {
        while (true) {
            std::unique_lock<std::mutex> lock(_upd_lock);
            _upd_cond.wait(lock, [&]() {return do_update || local_th_exit; });
            if (local_th_exit) {
                lock.unlock();
                break;
            }
            lock.unlock();

            if (_numOfConnections) {
                DEBUG_STREAM << "Update data";
                for_update_data();
            }
            do_update = false;
        }
    }

    void WSTangoConnSer::for_update_data()
    {
        string jsonStrOut;
        bool wasExc = false;
        ERROR_TYPE et;
        try {
            if (!_isInitDs)
            {
                et = ERROR_TYPE::INIT_FAILED;
                jsonStrOut = _errorMessage;
                wasExc = true;
            }
            else {
                jsonStrOut = (
                    _isGroup
                    ? TangoProcessor::processUpdate(groupForWs)
                    : TangoProcessor::processUpdate(deviceForWs)
                    );
            }
        }
        catch (Tango::ConnectionFailed &e)
        {
            jsonStrOut = fromException(e, "WSTangoConnSer::update_data() ConnectionFailed ");
            wasExc = true;
            et = ERROR_TYPE::CONNECTION_FAILED;
        }
        catch (Tango::CommunicationFailed &e)
        {
            jsonStrOut = fromException(e, "WSTangoConnSer::update_data() CommunicationFailed ");
            wasExc = true;
            et = ERROR_TYPE::COMMUNICATION_FAILED;
        }
        catch (Tango::DevFailed &e)
        {
            jsonStrOut = fromException(e, "WSTangoConnSer::update_data() DevFailed ");
            wasExc = true;
            et = ERROR_TYPE::TANGO_EXCEPTION;
        }
        catch (...) {
            ERROR_STREAM << "Unknown Exception from WSTangoConnSer::update_data()" << endl;
            jsonStrOut = "Unknown Exception from WSTangoConnSer::update_data())";
            wasExc = true;
            et = ERROR_TYPE::UNKNOWN_EXC;
        }
        if (wasExc) {
            removeSymbolsForString(jsonStrOut);
            ErrorInfo err;
            err.device_name = _wsds->deviceServer;
            err.typeofReq = "attribute"; // TODO: update?
            err.typeofError = et;
            err.errorMessage = jsonStrOut;
            jsonStrOut = StringProc::exceptionStringOut(err);
        }
        else {
            if (_wsds->get_status() != "The listening server is running")
                _wsds->set_status("The listening server is running");
        }
        wsThread->send_all(jsonStrOut);
    }

    void WSTangoConnSer::initDeviceServer()
    {
        try {
            if (_isGroup) {
                groupForWs = new GroupForWs(_wsds->deviceServer, _wsds->attributes, _wsds->pipeName);
            }
            else
                deviceForWs = new DeviceForWs(_wsds->deviceServer, _wsds->attributes, _wsds->pipeName);

            _hasAttrsForUpdate = (_wsds->attributes.size() > 0 || _wsds->pipeName.size() > 0);
            _isInitDs = true;
        }
        catch (Tango::DevFailed &e)
        {
            _errorMessage = fromException(e, "WSTangoConn::initDeviceServer()");
            removeSymbolsForString(_errorMessage);
        }
        return;
    }
}
