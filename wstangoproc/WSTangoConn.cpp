#include "WSTangoConn.h"
#include "WSThread_plain.h"
#include "WSThread_tls.h"

#include "StringProc.h"
#include "DeviceForWs.h"
#include "GroupForWs.h"

#include "UserControl.h"

#include "WebSocketDS.h"
#include "EventProcForServerMode.h"


namespace WebSocketDS_ns
{
    WSTangoConn::WSTangoConn(WebSocketDS *dev, pair<string, vector<string>> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, array<vector<string>, 4> event_subcr)
        :Tango::LogAdapter(dev)
    {
        groupOrDevice = nullptr;
        //init_wstc(dev, dsAndOptions, attrCommPipe);
        initOptionsAndDeviceServer(dsAndOptions, attrCommPipe);

        _wsds = dev;
        uc = unique_ptr<UserControl>(new UserControl(dev->authDS, typeOfIdent, _isLogActive));

        if (_command_name_for_user_control.size()) {
            uc->setCommandNameForCheckUser(_command_name_for_user_control);
        }
        if (_command_name_for_check_permission.size()) {
            uc->setCommandNameForCheckPermission(_command_name_for_check_permission);
        }

        wsThread = new WSThread_plain(this, portNumber);

        if (_isInitDs && isServerMode())
            initEventSubscrForServerMode(event_subcr);
    }

    WSTangoConn::WSTangoConn(WebSocketDS *dev, pair<string, vector<string>> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, array<vector<string>, 4> event_subcr, string cert, string key)
        :Tango::LogAdapter(dev)
    {
        groupOrDevice = nullptr;
        initOptionsAndDeviceServer(dsAndOptions, attrCommPipe);
        
        _wsds = dev;
        uc = unique_ptr<UserControl>(new UserControl(dev->authDS, typeOfIdent, _isLogActive));

        if (_command_name_for_user_control.size()) {
            uc->setCommandNameForCheckUser(_command_name_for_user_control);
        }
        if (_command_name_for_check_permission.size()) {
            uc->setCommandNameForCheckPermission(_command_name_for_check_permission);
        }
        
        wsThread = new WSThread_tls(this, portNumber, cert, key);
        if (_isInitDs && isServerMode())
            initEventSubscrForServerMode(event_subcr);
    }

    void WSTangoConn::initOptionsAndDeviceServer(pair<string, vector<string>>& dsAndOptions, array<vector<string>, 3> &attrCommPipe)
    {
        _deviceName = dsAndOptions.first;
        vector<string> gottenOptions = dsAndOptions.second;

        for (auto& opt : gottenOptions) {
			if (opt == "array_out") {
				_isObjData = false;
			}
            if (opt == "group")
                _isGroup = true;
            if (opt == "uselog")
                _isLogActive = true;
            if (opt.find("tident") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    if (gettedIdentOpt[1] == "rndid")
                        typeOfIdent = TYPE_OF_IDENT::RANDIDENT;
                    else if (gettedIdentOpt[1] == "smpl")
                        typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                    else if (gettedIdentOpt[1] == "rndid2")
                        typeOfIdent = TYPE_OF_IDENT::RANDIDENT2;
                    // Для аутентификации в Егоровом AuthDS в check_permissions_www
                    else if (gettedIdentOpt[1] == "permission_www")
                        typeOfIdent = TYPE_OF_IDENT::PERMISSION_WWW;
                    else
                        typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                }
            }
            if (opt == "notshrtatt") {
                _isShortAttr = false;
            }
            if (opt.find("mode") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    if (gettedIdentOpt[1] == "ser_cli_all")
                        ws_mode = MODE::SERVNCLIENT_ALL;
                    if (gettedIdentOpt[1] == "ser_cli_all_ro")
                        ws_mode = MODE::SERVNCLIENT_ALL_RO;

                    if (gettedIdentOpt[1] == "ser_cli_ali")
                        ws_mode = MODE::SERVNCLIENT_ALIAS;
                    if (gettedIdentOpt[1] == "ser_cli_ali_ro")
                        ws_mode = MODE::SERVNCLIENT_ALIAS_RO;

                    if (gettedIdentOpt[1] == "cli_all")
                        ws_mode = MODE::CLIENT_ALL;
                    if (gettedIdentOpt[1] == "cli_all_ro")
                        ws_mode = MODE::CLIENT_ALL_RO;

                    if (gettedIdentOpt[1] == "cli_ali")
                        ws_mode = MODE::CLIENT_ALIAS;
                    if (gettedIdentOpt[1] == "cli_ali_ro")
                        ws_mode = MODE::CLIENT_ALIAS_RO;
                }
            }
            if (opt == "tm100ms") {
                _istm100ms = true;
            }
            if (opt.find("command_name_for_check_user") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    _command_name_for_user_control = gettedIdentOpt[1];
                }
            }
            if (opt.find("command_name_for_check_permission") != string::npos) {
                auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                if (gettedIdentOpt.size() > 1) {
                    _command_name_for_check_permission = gettedIdentOpt[1];
                }
            }
        }
        _isInitDs = initDeviceServer(attrCommPipe);

    }

    WSTangoConn::~WSTangoConn()
    {
        if (wsThread != nullptr) {
            wsThread->stop();
            void *ptr;
            wsThread->join(&ptr);
        }
    }

    string WSTangoConn::for_update_data()
    {
        // ??? !!! If server mode is not used
        string jsonStrOut;
        bool wasExc = false;
        try {
            if (!_isInitDs)
            {
                jsonStrOut = _errorMessage;
                wasExc = true;
            }
            else {
                if (isServerMode())
                    jsonStrOut = groupOrDevice->generateJsonForUpdate();
                else
                    jsonStrOut = "Current mode is not SERVER";
            }
        }
        catch (Tango::ConnectionFailed &e)
        {
            jsonStrOut = fromException(e, "WSTangoConn::for_update_data() ConnectionFailed ");
            wasExc = true;
        }
        catch (Tango::CommunicationFailed &e)
        {
            jsonStrOut = fromException(e, "WSTangoConn::for_update_data() CommunicationFailed ");
            wasExc = true;
        }
        catch (Tango::DevFailed &e)
        {
            jsonStrOut = fromException(e, "WSTangoConn::for_update_data() DevFailed ");
            wasExc = true;
        }
        catch (...) {
            ERROR_STREAM << "Unknown Exception from WSTangoConn::for_update_data()" << endl;
            jsonStrOut = "Unknown Exception from WSTangoConn::for_update_data()";
            wasExc = true;
        }
        if (wasExc) {
            // State остаётся ON, но в STATUS выводится сообщение
            if (isServerMode())
                _wsds->set_status(jsonStrOut);

            removeSymbolsForString(jsonStrOut);
            jsonStrOut = StringProc::exceptionStringOut(jsonStrOut);
        }
        else {
            if (isServerMode())
                if (_wsds->get_status() != "The listening server is running")
                    _wsds->set_status("The listening server is running");
        }
        if (isServerMode() && hasAttrOrPipe) {
            // Только если используется серверный режим
            wsThread->send_all(jsonStrOut);
        }
        return jsonStrOut;
    }

    void WSTangoConn::sendRequest(const ParsedInputJson& inputReq, bool& isBinary, ConnectionData *connData, string& out)
    {
        isBinary = false;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        if (typeWsReq == TYPE_WS_REQ::UNKNOWN) {
            out = StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported", inputReq.type_req);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::COMMAND) {
            if (!isServerMode()) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
                return;
            }
            if (!_isInitDs) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, _errorMessage, inputReq.type_req);
                return;            
            }
            sendRequest_Command(inputReq, connData, isBinary, out);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::PIPE_COMM) {
            if (!isServerMode()) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
                return;
            }
            if (!_isInitDs) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, _errorMessage, inputReq.type_req);
                return;
            }
            sendRequest_PipeComm(inputReq, connData, out);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::RIDENT_REQ) {
            out = sendRequest_RidentReq(inputReq, connData);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::RIDENT_ANS) {
            out = sendRequest_RidentAns(inputReq, connData);
            return;
        }
        
        if (typeWsReq == TYPE_WS_REQ::RIDENT) {
            out = sendRequest_Rident(inputReq, connData);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT) {
            if (ws_mode == MODE::SERVER
                || ws_mode == MODE::CLIENT_ALIAS_RO
                || ws_mode == MODE::CLIENT_ALL_RO
                || ws_mode == MODE::SERVNCLIENT_ALIAS_RO
                || ws_mode == MODE::SERVNCLIENT_ALL_RO) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported in the current mode", inputReq.type_req);
                return;
            }
            sendRequest_Command_DevClient(inputReq, connData, isBinary, out);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT || typeWsReq == TYPE_WS_REQ::ATTR_GR_CLIENT) {
            if (ws_mode == MODE::SERVER) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported in the current mode", inputReq.type_req);
                return;
            }
            sendRequest_AttrClient(inputReq, connData, out);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
            if (!isServerMode()) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
                return;
            }
            if (!_isInitDs) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, _errorMessage, inputReq.type_req);
                return;
            }
            
            sendRequest_AttrWrite(inputReq, connData, out);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT_WR) {
            if (ws_mode == MODE::SERVER
                || ws_mode == MODE::CLIENT_ALIAS_RO
                || ws_mode == MODE::CLIENT_ALL_RO
                || ws_mode == MODE::SERVNCLIENT_ALIAS_RO
                || ws_mode == MODE::SERVNCLIENT_ALL_RO) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported in the current mode", inputReq.type_req);
                return;
            }

            sendRequest_AttrWrite_DevClient(inputReq, connData, out);
            return;
        }
        
        if (typeWsReq == TYPE_WS_REQ::USER_CHECK_STATUS) {
            sendRequest_UserStatus(inputReq, connData, out);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::CHANGE_USER) {
            out = sendRequest_ForAuth(inputReq, connData);
            return;
        }

        if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_READ) {
            if (!isServerMode()) {
                out = StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
                return;
            }
            sendRequest_AttrRead(inputReq, connData, out);
            return;
        }

        // В обычном случае не возвращается никогда
        out = "{\"error\": \"Unknown Request\"}";
    }

    void WSTangoConn::checkUser(ConnectionData* connData)
    {
        if (typeOfIdent == TYPE_OF_IDENT::SIMPLE) {
            if (connData->login.size())
                connData->userCheckStatus = uc->check_user(connData->login, connData->password);
            else {
                connData->userCheckStatus.second =  "You need to authenticate";
            }
        }
        if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
            if (connData->login.size() && connData->forRandIdent.rand_ident_str.size() && connData->forRandIdent.rand_ident_hash.size()) {
                connData->userCheckStatus = uc->check_user_rident(connData->login, connData->forRandIdent.rand_ident_str, connData->forRandIdent.rand_ident_hash);
            }
            else {
                connData->userCheckStatus.second = "You need to authenticate";
            }
        }
        if (typeOfIdent == TYPE_OF_IDENT::PERMISSION_WWW) {
            // Для аутентификации в Егоровом AuthDS в check_permissions_www
            // В данном случае логин и пароль просто сохраняется.
            if (connData->login.size() && connData->password.size()) {
                connData->userCheckStatus = make_pair(true, "");
            }
            else {
                connData->userCheckStatus.first = false;
                connData->userCheckStatus.second = "You need to authenticate";
            }
        }
    }

    unsigned int WSTangoConn::getMaxBuffSize()
    {
        return _wsds->maximumBufferSize;
    }

    unsigned short WSTangoConn::getMaxNumberOfConnections()
    {
        return _wsds->maxNumberOfConnections;
    }

    string WSTangoConn::getAuthDS()
    {
        return _wsds->authDS;
    }

    bool WSTangoConn::initDeviceServer(array<vector<string>, 3> &attrCommPipe)
    {
        _errorMessage.clear();
        bool isInit = false;

        try {
            if (_isGroup) {
				groupOrDevice = unique_ptr<GroupForWs>(new GroupForWs(_deviceName, attrCommPipe, _isObjData));
            }
            else
                groupOrDevice = unique_ptr<DeviceForWs>(new DeviceForWs(_deviceName, attrCommPipe, _isObjData));
            if (!_isShortAttr)
                groupOrDevice->useNotShortAttrOut();

            

            isInit = true;
        }
        catch (Tango::DevFailed &e)
        {
            _errorMessage = fromException(e, "WSTangoConn::initDeviceServer()");
            removeSymbolsForString(_errorMessage);
            return isInit;
        }

        auto testSize = [=](vector<string>& attr, vector<string>& pipe) {
            for (auto& a : attr) {
                if (a.size()) {
                    if (!groupOrDevice->isOnlyWrtAttribute(a)) {
                        return true;
                    }
                }
            }

            for (auto& p : pipe) {
                if (p.size())
                    return true;
            }
            return false;
        };

        hasAttrOrPipe = testSize(attrCommPipe[0], attrCommPipe[2]);

        return isInit;
    }

    bool WSTangoConn::initDeviceServer()
    {
        _errorMessage.clear();
        bool isInit = false;

        try {
            if (_isGroup) {
				groupOrDevice = unique_ptr<GroupForWs>(new GroupForWs(_deviceName, _isObjData));
            }
            else
				groupOrDevice = unique_ptr<DeviceForWs>(new DeviceForWs(_deviceName, _isObjData));
            if (!_isShortAttr)
                groupOrDevice->useNotShortAttrOut();
            isInit = true;
        }
        catch (Tango::DevFailed &e)
        {
            _errorMessage = fromException(e, "WSTangoConn::initDeviceServer()");
            removeSymbolsForString(_errorMessage);
        }
        return isInit;
    }

    void WSTangoConn::initEventSubscrForServerMode(const array<vector<string>, 4> &event_subcr)
    {
        // Проверка наличия подписчика на события в Property
        // true, если есть хотя бы одна подписка
        auto hasEventSubscr = [](const array<vector<string>, 4> &event_subcr) {
            for (const auto& ev : event_subcr) {
                if (ev.size())
                    return true;
            }
            return false;
        };

        // Пока только для одного девайса, но не для группы
        
        if (hasEventSubscr(event_subcr)) {
            if (_isGroup) {
                auto listOfdeviceNames = groupOrDevice->getListOfDevicesNames();
                eventSubscrServ = std::unique_ptr<EventProcForServerMode>(new EventProcForServerMode(wsThread, listOfdeviceNames, event_subcr));
            }
            else {
                eventSubscrServ = std::unique_ptr<EventProcForServerMode>(new EventProcForServerMode(wsThread, _deviceName, event_subcr));
            }
        }

    }

    string WSTangoConn::fromException(Tango::DevFailed &e, string func)
    {
        string outErrMess;
        auto lnh = e.errors.length();
        for (unsigned int i=0;i<lnh;i++) {
            if (i)
                outErrMess += " ";
            ERROR_STREAM << " From " + func + ": " << e.errors[i].desc << endl;
            outErrMess += e.errors[i].desc;
        }
        return outErrMess;
    }

    void  WSTangoConn::removeSymbolsForString(string &str) {
        //if (str.find('\0') != string::npos)
        //    str.erase(remove(str.begin(), str.end(), '\0'), str.end());
        if (str.find('\r') != string::npos)
            str.erase(remove(str.begin(), str.end(), '\r'), str.end());
        if (str.find('\n') != string::npos)
            std::replace(str.begin(), str.end(), '\n', ' ');
    }

    TYPE_WS_REQ WSTangoConn::getTypeWsReq(const string& req) {
        // for COMMAND is command or command_device or command_group
        if (req == "command" || req == "command_device" || req == "command_group")
            return TYPE_WS_REQ::COMMAND;
        // for PIPE_COMM is read_pipe or read_pipe_dev or read_pipe_gr
        if (req == "read_pipe" || req == "read_pipe_dev" || req == "read_pipe_gr")
            return TYPE_WS_REQ::PIPE_COMM;
        if (req == "rident_req")
            return TYPE_WS_REQ::RIDENT_REQ;
        if (req == "rident_ans")
            return TYPE_WS_REQ::RIDENT_ANS;
        if (req == "rident")
            return TYPE_WS_REQ::RIDENT;
        if (req == "command_device_cl")
            return TYPE_WS_REQ::COMMAND_DEV_CLIENT;
        if (req == "attr_device_cl")
            return TYPE_WS_REQ::ATTR_DEV_CLIENT;
        if (req == "write_attr" || req == "write_attr_dev" || req == "write_attr_gr")
            return TYPE_WS_REQ::ATTRIBUTE_WRITE;
        if (req == "write_attr_dev_cl")
            return TYPE_WS_REQ::ATTR_DEV_CLIENT_WR;
        if (req == "attr_group_cl") 
            return TYPE_WS_REQ::ATTR_GR_CLIENT;
        if (req == "user_status")
            return TYPE_WS_REQ::USER_CHECK_STATUS;
        if (req == "change_user_smpl")
            return TYPE_WS_REQ::CHANGE_USER;
        if (req == "read_attr" || req == "read_attr_dev" || req == "read_attr_gr")
            return TYPE_WS_REQ::ATTRIBUTE_READ;

        return TYPE_WS_REQ::UNKNOWN;
    }

    bool WSTangoConn::isServerMode()
    {
        if (
            ws_mode == MODE::SERVER
            || ws_mode == MODE::SERVNCLIENT_ALIAS
            || ws_mode == MODE::SERVNCLIENT_ALIAS_RO
            || ws_mode == MODE::SERVNCLIENT_ALL
            || ws_mode == MODE::SERVNCLIENT_ALL_RO
            )
            return true;
        else
            return false;
    }


    void WSTangoConn::sendRequest_Command(const ParsedInputJson &inputReq, ConnectionData* connData, bool &isBinary, string& resp_json)
    {
        if (_isGroup) {
            if (inputReq.type_req != "command_device" && inputReq.type_req != "command_group") {
                resp_json = StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("command_name"), "type_req must be command_device or command_group", "command");
                return;
            }
        }
        else {
            if (inputReq.type_req != "command") {
                resp_json = StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("command_name"), "type_req must be command", "command");
                return;
            }
        }

        string devicename;
        string commandName;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        if (_isGroup && inputReq.type_req == "command_device") {
            devicename = inputReq.otherInpStr.at("device_name");
        }
        else {
            devicename = _wsds->deviceServer;
        }

        string errorMessage = checkPermissionForRequest(inputReq, connData, commandName, devicename, typeWsReq);

        if (errorMessage.size()) {
            resp_json = errorMessage;
            return;
        }

        bool statusComm;
        OUTPUT_DATA_TYPE odt = groupOrDevice->checkDataType(commandName);
        if (odt == OUTPUT_DATA_TYPE::JSON)
            resp_json = groupOrDevice->sendCommand(inputReq,statusComm);
        if (odt == OUTPUT_DATA_TYPE::BINARY) {
            resp_json = groupOrDevice->sendCommandBin(inputReq,statusComm);
            isBinary = true;
        }
        bool isSent = uc->sendLogCommand(inputReq, connData, devicename, _isGroup, statusComm, typeWsReq);
        if (isSent)
            INFO_STREAM << "Command. Information was sent to the log" << endl;
    }

    void WSTangoConn::sendRequest_Command_DevClient(const ParsedInputJson& inputReq, ConnectionData* connData, bool& isBinary, string& resp_json)
    {
        string errorMessage;
        string device_name = checkDeviceNameKey(inputReq, errorMessage);

        if (errorMessage.size()) {
            resp_json = errorMessage;
            return;
        }

        string commandName;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        errorMessage = checkPermissionForRequest(inputReq, connData, commandName, device_name, typeWsReq);

        if (errorMessage.size()) {
            resp_json = errorMessage;
            return;
        }

        vector<string> commands{ commandName };
        try {
			DeviceForWs deviceForWs(device_name, commandName, TYPE_WS_REQ::COMMAND_DEV_CLIENT, _isObjData);
            bool statusComm;
            OUTPUT_DATA_TYPE odt = deviceForWs.checkDataType(commands[0]);
            if (odt == OUTPUT_DATA_TYPE::JSON)
                resp_json = deviceForWs.sendCommand(inputReq, statusComm);
            if (odt == OUTPUT_DATA_TYPE::BINARY) {
                resp_json = deviceForWs.sendCommandBin(inputReq, statusComm);
                if (statusComm)
                    isBinary = true;
            }
            bool isSent = uc->sendLogCommand(inputReq, connData, device_name, _isGroup, statusComm, typeWsReq);
            if (isSent)
                INFO_STREAM << "Information was sent to the log" << endl;
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, errors, inputReq.type_req);
        }
    }

    void WSTangoConn::sendRequest_PipeComm(const ParsedInputJson &inputReq, ConnectionData* connData, string& resp_json)
    {
        if (inputReq.check_key("pipe_name") != TYPE_OF_VAL::VALUE) {
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key pipe_name or pipe_name is not value", inputReq.type_req);
            return;
        }
        resp_json = groupOrDevice->sendPipeCommand(inputReq);
        removeSymbolsForString(resp_json);
    }

    string WSTangoConn::sendRequest_RidentReq(const ParsedInputJson &inputReq, ConnectionData* connData)
    {
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT2)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is RANDIDENT2", inputReq.type_req);

        if (inputReq.check_key("login") == TYPE_OF_VAL::VALUE) {
            connData->forRandIdent.tmp_login = inputReq.otherInpStr.at("login");
        }
        else {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Key login not found or login is not value", inputReq.type_req);
        } 

        if (connData->userCheckStatus.first && (connData->login == connData->forRandIdent.tmp_login))
            return StringProc::exceptionStringOut(inputReq.id, NONE, "User " + connData->login + " have already been authorized ", inputReq.type_req);

        std::uniform_int_distribution<int> dis(1000000, 9999999);
        
        int rand_ident = dis(generator);

        stringstream json;
        json << "{\"event\": \"read\", \"type_req\": \"rident_req\", ";
        try {
            auto idTmp = stoi(inputReq.id);
            json << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (inputReq.id == NONE)
                json << "\"id_req\": " << inputReq.id << ", ";
            else
                json << "\"id_req\": \"" << inputReq.id << "\", ";
        }
        json << "\"rident\": " << rand_ident << "}";
        connData->forRandIdent.rand_ident_str = to_string(rand_ident);
        connData->forRandIdent.isRandSended = true;
        return json.str();
    }

    string WSTangoConn::sendRequest_RidentAns(const ParsedInputJson &inputReq, ConnectionData* connData)
    {
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT2)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is RANDIDENT2", inputReq.type_req);

        if (inputReq.check_key("login") != TYPE_OF_VAL::VALUE) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Key login not found or login is not value", inputReq.type_req);
        }

        string inp_login = inputReq.otherInpStr.at("login");

        if (inp_login != connData->forRandIdent.tmp_login) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Key login must match the one sent before", inputReq.type_req);
        }

        if (connData->userCheckStatus.first && (connData->login == inp_login))
            return StringProc::exceptionStringOut(inputReq.id, NONE, "You have already been authorized ", inputReq.type_req);

        if (!connData->forRandIdent.isRandSended)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);

        if (inputReq.check_key("rident_hash") != TYPE_OF_VAL::VALUE){
            if (connData->forRandIdent.isRandSended)
                connData->forRandIdent.isRandSended = false;

            connData->forRandIdent.rand_ident_hash.clear();
            connData->forRandIdent.rand_ident_str.clear();
            connData->forRandIdent.tmp_login.clear();

            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }

        connData->forRandIdent.rand_ident_hash = inputReq.otherInpStr.at("rident_hash");

        auto userStatus = uc->check_user_rident(inp_login, connData->forRandIdent.rand_ident_str, connData->forRandIdent.rand_ident_hash);

        if (!userStatus.first){
            connData->forRandIdent.rand_ident_hash.clear();
            connData->forRandIdent.isRandSended = false;
            connData->forRandIdent.rand_ident_str.clear();
            return StringProc::exceptionStringOut(inputReq.id, NONE, userStatus.second + " Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }

        connData->userCheckStatus.first = true;
        connData->login = inp_login;
        connData->forRandIdent.tmp_login.clear();

        stringstream json;
        json << "{\"event\": \"read\", \"type_req\": \"" + inputReq.type_req + "\", ";
        try {
            auto idTmp = stoi(inputReq.id);
            json << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (inputReq.id == NONE)
                json << "\"id_req\": " << inputReq.id << ", ";
            else
                json << "\"id_req\": \"" << inputReq.id << "\", ";
        }
        json << "\"success\": true }";
        return json.str();
    }

    string WSTangoConn::sendRequest_Rident(const ParsedInputJson& inputReq, ConnectionData* connData) 
    {
        
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is RANDIDENT", inputReq.type_req);

        if (inputReq.check_keys({ "rident_hash", "login", "rident" }) != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);

        // Проверка ... была ли ранняя аутентификация. 
        // Если была ... проверяется используется ли тот же логин
        if (connData->userCheckStatus.first) {
            if (connData->login == inputReq.otherInpStr.at("login"))
                return StringProc::exceptionStringOut(inputReq.id, NONE, ("User " + connData->login + " have already been authorized "), inputReq.type_req);
        }

        auto userStatus = uc->check_user_rident(inputReq.otherInpStr.at("login"), inputReq.otherInpStr.at("rident"), inputReq.otherInpStr.at("rident_hash"));

        if (!userStatus.first) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, connData->userCheckStatus.second + "Incorrect login or rand_ident or rand_ident_hash. ", inputReq.type_req);
        }
                

        connData->forRandIdent.rand_ident_hash = inputReq.otherInpStr.at("rident_hash");
        connData->login = inputReq.otherInpStr.at("login");
        connData->forRandIdent.rand_ident_str = inputReq.otherInpStr.at("rident");
        connData->userCheckStatus = userStatus;

        stringstream json;
        json << "{\"event\": \"read\", \"type_req\": \"" + inputReq.type_req + "\", ";
        try {
            auto idTmp = stoi(inputReq.id);
            json << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (inputReq.id == NONE)
                json << "\"id_req\": " << inputReq.id << ", ";
            else
                json << "\"id_req\": \"" << inputReq.id << "\", ";
        }
        json << "\"success\": true }";
        return json.str();
    }

    string WSTangoConn::sendRequest_ForAuth(const ParsedInputJson& inputReq, ConnectionData* connData) {
        if (typeOfIdent != TYPE_OF_IDENT::SIMPLE &&  typeOfIdent != TYPE_OF_IDENT::PERMISSION_WWW)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is SIMPLE", inputReq.type_req);
        
        // Производится проверка пользователя
        // Если проверка проводилась ранее, и была успешна, пользователь поменяется только если текущая проверка также успешна

        if (inputReq.check_keys({ "login", "password" }) != TYPE_OF_VAL::VALUE) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "keys login or password not found", inputReq.type_req);
        }

        string login = inputReq.otherInpStr.at("login");
        string pass = inputReq.otherInpStr.at("password");

        if (!login.size() || !pass.size()) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "keys login and password should not be empty", inputReq.type_req);
        }
        
        if (connData->login.size())
        // Информация о последнем клиенте хранится connData
        if (connData->login == login && connData->userCheckStatus.first)
        {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "User " + connData->login + " already verified", inputReq.type_req);
        }

        pair<bool, string> userCheckStatus;

        if (typeOfIdent != TYPE_OF_IDENT::PERMISSION_WWW) {
            userCheckStatus = uc->check_user(login, pass);
        }
        else {
            // Для аутентификации в Егоровом AuthDS в check_permissions_www
            // При подключении не проводится check_user. 
            userCheckStatus = make_pair(true,"");
        }
        

        if (!userCheckStatus.first) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, userCheckStatus.second, inputReq.type_req);
        }

        connData->userCheckStatus = userCheckStatus;
        connData->login = login;
        connData->password = pass;

        return StringProc::responseStringOut(inputReq.id, "User " + connData->login + " successfully authenticated", inputReq.type_req);
    }

    void WSTangoConn::sendRequest_AttrClient(const ParsedInputJson& inputReq, ConnectionData* connData, string& resp_json)
    {
        string errorMessage;
        string device_name = checkDeviceNameKey(inputReq, errorMessage);

        if (errorMessage.size()) {
            resp_json = errorMessage;
            return;
        }

        bool isAttrNotFound = false;
        bool isPipeNotFound = false;
        
        if (inputReq.check_key("attributes") != TYPE_OF_VAL::VALUE && inputReq.check_key("attributes") != TYPE_OF_VAL::ARRAY)
            isAttrNotFound = true;

        if (inputReq.check_key("pipe") != TYPE_OF_VAL::VALUE && inputReq.check_key("pipe") != TYPE_OF_VAL::ARRAY)
            isPipeNotFound = true;

        if (isAttrNotFound && isPipeNotFound) {
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, "Not found keys attributes / pipe or attributes / pipe is not value or array", inputReq.type_req);
            return;
        }

        vector<string> attributes;
        vector<string> pipe;

        if (inputReq.check_key("attributes") == TYPE_OF_VAL::ARRAY)
            attributes = inputReq.otherInpVec.at("attributes");
        if (inputReq.check_key("attributes") == TYPE_OF_VAL::VALUE)
            attributes.push_back(inputReq.otherInpStr.at("attributes"));

        if (inputReq.check_key("pipe") == TYPE_OF_VAL::ARRAY)
            pipe = inputReq.otherInpVec.at("pipe");
        if (inputReq.check_key("pipe") == TYPE_OF_VAL::VALUE)
            pipe.push_back(inputReq.otherInpStr.at("pipe"));

        std::pair<vector<string>, vector<string>> attr_pipe;
        attr_pipe.first = attributes;
        attr_pipe.second  = pipe;
        try {
            GroupOrDeviceForWs* dev = nullptr;
            TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);
            if (typeWsReq == WebSocketDS_ns::TYPE_WS_REQ::ATTR_DEV_CLIENT) {
				dev = new WebSocketDS_ns::DeviceForWs(device_name, attr_pipe, _isObjData);
            }
            if (typeWsReq == WebSocketDS_ns::TYPE_WS_REQ::ATTR_GR_CLIENT) {
				dev = new WebSocketDS_ns::GroupForWs(device_name, attr_pipe, _isObjData);
            }
            stringstream ss;
            dev->generateJsonForAttrRead(inputReq, ss);
            resp_json = ss.str();

            if (dev != nullptr)
                delete dev;
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, errors, inputReq.type_req);
        }        
    }

    void WSTangoConn::sendRequest_AttrRead(const ParsedInputJson& inputReq, ConnectionData* connData, string& resp_json)
    {
        if (inputReq.check_key("attr_name") != TYPE_OF_VAL::VALUE && inputReq.check_key("attr_name") != TYPE_OF_VAL::ARRAY) {
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key attr_name or attr_name is not value or array", inputReq.type_req);
            return;
        }

        if (_isGroup) {
            if (inputReq.type_req != "read_attr_dev" && inputReq.type_req != "read_attr_gr") {
                resp_json = StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("attr_name"), "type_req must be read_attr_dev or read_attr_gr", inputReq.type_req);
                return;
            }
        }
        else {
            if (inputReq.type_req != "read_attr") {
                resp_json = StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("attr_name"), "type_req must be read_attr", inputReq.type_req);
                return;
            }
        }
        try {
            resp_json = groupOrDevice->sendAttrRead(inputReq);
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, errors, inputReq.type_req);
        }
    }

    void WSTangoConn::sendRequest_AttrWrite(const ParsedInputJson& inputReq, ConnectionData* connData, string& resp_json)
    {
        if (_isGroup) {
            if (inputReq.type_req != "write_attr_dev" && inputReq.type_req != "write_attr_gr") {
                resp_json = StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("attr_name"), "type_req must be write_attr_dev or write_attr_gr", "command");
                return;
            }
        }
        else {
            if (inputReq.type_req != "write_attr") {
                resp_json = StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("attr_name"), "type_req must be write_attr", "command");
                return;
            }
        }

        string devicename;
        string attrName;

        if (_isGroup && inputReq.type_req == "write_attr_dev") {
            devicename = inputReq.otherInpStr.at("device_name");
        }
        else {
            devicename = _wsds->deviceServer;
        }

        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        // checking key attr_name in checkPermissionForRequest
        string errorMessage = checkPermissionForRequest(inputReq, connData, attrName, devicename, typeWsReq);

        bool statusAttr;

        if (errorMessage.size()) {
            resp_json = errorMessage;
            return;
        }

        resp_json = groupOrDevice->sendAttrWr(inputReq, statusAttr);

        bool isSent = uc->sendLogCommand(inputReq, connData, devicename, _isGroup, statusAttr, typeWsReq);
        if (isSent)
            INFO_STREAM << "Write attribute. Information was sent to the log" << endl;
    }

    void WSTangoConn::sendRequest_AttrWrite_DevClient(const ParsedInputJson& inputReq, ConnectionData* connData, string& resp_json)
    {
        string attrName;

        string errorMessage;
        string device_name = checkDeviceNameKey(inputReq, errorMessage);

        if (errorMessage.size()) {
            resp_json =  errorMessage;
            return;
        }

        // checking key attr_name in checkPermissionForRequest
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        errorMessage = checkPermissionForRequest(inputReq, connData, attrName, device_name, typeWsReq);

        if (errorMessage.size()) {
            resp_json = errorMessage;
            return;
        }

        try {
			DeviceForWs deviceForWs(device_name, attrName, TYPE_WS_REQ::ATTR_DEV_CLIENT_WR, _isObjData);

            bool statusAttr;
            resp_json = deviceForWs.sendAttrWr(inputReq, statusAttr);

            bool isSent = uc->sendLogCommand(inputReq, connData, device_name, _isGroup, statusAttr, typeWsReq);

            if (isSent)
                INFO_STREAM << "Write attribute. Information was sent to the log" << endl;

        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            resp_json = StringProc::exceptionStringOut(inputReq.id, NONE, errors, inputReq.type_req);
        }
    }

    void WSTangoConn::sendRequest_UserStatus(const ParsedInputJson& inputReq, ConnectionData* connData, string& resp_json)
    {
        // Проверка статуса клиента. Возвращает результат запроса check_user
        bool user_status = connData->userCheckStatus.first;

        std::stringstream json;
        json << "{\"event\": \"read\", \"type_req\": \"" << inputReq.type_req << "\", \"data\": {";
        json << "\"status\": " << boolalpha << user_status;
        json << ", \"username\": \"" << connData->login << "\"";
        json << "}";
        json << "}";

        resp_json = json.str();
    }

    string WSTangoConn::checkPermissionForRequest(const ParsedInputJson &inputReq, ConnectionData* connData, string &commandName, string device_name, TYPE_WS_REQ typeWsReq)
    {
        if (typeWsReq == TYPE_WS_REQ::COMMAND || typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT) {

            if (inputReq.check_key("command_name") != TYPE_OF_VAL::VALUE)
                return StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key command_name or command_name is not value", inputReq.type_req);
            commandName = inputReq.otherInpStr.at("command_name");
        }
        else if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE || typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT_WR) {
            if (inputReq.check_key("attr_name") != TYPE_OF_VAL::VALUE)
                return StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key attr_name or attr_name is not value", inputReq.type_req);
            commandName = inputReq.otherInpStr.at("attr_name");
        }
        else {
            // Этот ответ в нормальном случае не отправлется
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Unknown error. Check source", inputReq.type_req);
        }
        string mess;

        // Пока логин пароль (и прочие аут-данные) вводятся при запросе
        // В дальнейшем, когда будет режим проверки во время сессии
        // дописать дополнительно проверку режима.
        if (!connData->userCheckStatus.first && typeOfIdent != TYPE_OF_IDENT::RANDIDENT2 && typeOfIdent != TYPE_OF_IDENT::RANDIDENT) {
            mess = connData->userCheckStatus.second;
            INFO_STREAM << mess << endl;
            return StringProc::exceptionStringOut(inputReq.id, commandName, mess, inputReq.type_req);
        }
        else if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT2 || typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
            if (!connData->userCheckStatus.first) {
                if (connData->forRandIdent.isRandSended)
                    connData->forRandIdent.isRandSended = false;
                if (connData->forRandIdent.rand_ident_str.size())
                    connData->forRandIdent.rand_ident_str.clear();
                connData->login.clear();
                string mess;
                if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT2) {
                    mess = "Send request for rand_identification (Using method is USERANDIDENT2). For details read the manual.";
                }
                if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT) {
                    mess = "Send an authorization request (Using method is USERANDIDENT). For details read the manual.";
                }
                return StringProc::exceptionStringOut(inputReq.id, commandName, mess, inputReq.type_req);
            }
        }

        bool permission = uc->check_permission(inputReq, connData, device_name, _isGroup, mess, typeWsReq);

        if (mess.size())
            INFO_STREAM << mess << endl;

        if (!permission)
        {
            if (mess.size())
                return StringProc::exceptionStringOut(inputReq.id, commandName, mess, inputReq.type_req);
            else
                return StringProc::exceptionStringOut(inputReq.id, commandName, "Permission denied", inputReq.type_req);
        }

        return "";
    }

    string WSTangoConn::getDeviceNameFromAlias(string alias, string& errorMessage)
    {
        string device_name_from_alias;
        errorMessage.clear();
        try {
            Tango::Database *db = Tango::Util::instance()->get_database();
            db->get_device_alias(alias, device_name_from_alias);
        }
        catch (Tango::DevFailed& e) {
            for (unsigned int i = 0; i < e.errors.length(); i++) {
                if (i > 0)
                    errorMessage += " ||| ";
                errorMessage += (string)e.errors[i].desc;
            }
        }
        return device_name_from_alias;
    }

    std::string WSTangoConn::checkDeviceNameKey(const ParsedInputJson &inputReq, string &errorMessage)
    {
        string device_name;

        if (inputReq.check_key("device_name") != TYPE_OF_VAL::VALUE) {
            errorMessage = StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key device_name or device_name is not value", inputReq.type_req);
            return device_name;
        }

        // Проверка, является ли имя девайс псевдонимом
        device_name = getDeviceNameFromAlias(inputReq.otherInpStr.at("device_name"), errorMessage);

        

        // Если используется режим псевдонимов
        if (
            ws_mode == MODE::SERVNCLIENT_ALIAS
            || ws_mode == MODE::CLIENT_ALIAS
            || ws_mode == MODE::CLIENT_ALIAS_RO
            || ws_mode == MODE::SERVNCLIENT_ALIAS_RO
            )
        {
            if (errorMessage.size())
                errorMessage = StringProc::exceptionStringOut(inputReq.id, NONE, errorMessage, inputReq.type_req);
            return device_name;
        }
            
        // В остальных режимах можно использовать и псевдоним, и имя девайса
        if (!errorMessage.size())
            return device_name;

        errorMessage.clear();
        return inputReq.otherInpStr.at("device_name");
    }
}
