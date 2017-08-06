#include "WSTangoConn.h"
#include "WSThread_plain.h"
#include "WSThread_tls.h"

#include "StringProc.h"
#include "DeviceForWs.h"
#include "GroupForWs.h"

#include "UserControl.h"

#include "WebSocketDS.h"


namespace WebSocketDS_ns
{
    WSTangoConn::WSTangoConn(WebSocketDS *dev, pair<string, vector<string>> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber)
        :Tango::LogAdapter(dev)
    {
        groupOrDevice = nullptr;
        //init_wstc(dev, dsAndOptions, attrCommPipe);
        initOptionsAndDeviceServer(dsAndOptions, attrCommPipe);

        _wsds = dev;
        uc = unique_ptr<UserControl>(new UserControl(dev->authDS, typeOfIdent, _isLogActive));

        wsThread = new WSThread_plain(this, portNumber);
    }

    WSTangoConn::WSTangoConn(WebSocketDS *dev, pair<string, vector<string>> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, string cert, string key)
        :Tango::LogAdapter(dev)
    {
        groupOrDevice = nullptr;
        initOptionsAndDeviceServer(dsAndOptions, attrCommPipe);
        
        _wsds = dev;
        uc = unique_ptr<UserControl>(new UserControl(dev->authDS, typeOfIdent, _isLogActive));
        
        wsThread = new WSThread_tls(this, portNumber, cert, key);
    }

    void WSTangoConn::initOptionsAndDeviceServer(pair<string, vector<string>>& dsAndOptions, array<vector<string>, 3> &attrCommPipe)
    {
        _deviceName = dsAndOptions.first;
        vector<string> gettedOptions = dsAndOptions.second;

        for (auto& opt : gettedOptions) {
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
                    else if (gettedIdentOpt[1] == "rndid3")
                        typeOfIdent = TYPE_OF_IDENT::RANDIDENT3;
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
        string jsonStrOut;
        bool wasExc = false;
        try {
            if (!_isInitDs)
            {
                jsonStrOut = _errorMessage;
                wasExc = true;
            }
            else
                jsonStrOut = groupOrDevice->generateJsonForUpdate();
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
            if (_wsds->get_status() != "The listening server is running")
                _wsds->set_status("The listening server is running");
        }
        if (isServerMode())
            // Только если используется серверный режим
            wsThread->send_all(jsonStrOut);
        return jsonStrOut;
    }

    string WSTangoConn::sendRequest(const ParsedInputJson& inputReq, bool& isBinary, ConnectionData& connData)
    {
        isBinary = false;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        if (typeWsReq == TYPE_WS_REQ::UNKNOWN) {
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported", inputReq.type_req);
        }

        if (typeWsReq == TYPE_WS_REQ::COMMAND) {
            if (!isServerMode()) {
                return StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
            }
            if (!_isInitDs)
                return StringProc::exceptionStringOut(inputReq.id, NONE, _errorMessage, inputReq.type_req);
            return sendRequest_Command(inputReq, connData, isBinary);
        }

        if (typeWsReq == TYPE_WS_REQ::PIPE_COMM) {
            if (!isServerMode()) {
                return StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
            }
            if (!_isInitDs)
                return StringProc::exceptionStringOut(inputReq.id, NONE, _errorMessage, inputReq.type_req);
            return sendRequest_PipeComm(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::RIDENT_REQ) {
            return sendRequest_RidentReq(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::RIDENT_ANS) {
            return sendRequest_RidentAns(inputReq, connData);
        }
        
        if (typeWsReq == TYPE_WS_REQ::RIDENT) {
            return sendRequest_Rident(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT) {
            if (ws_mode == MODE::SERVER
                || ws_mode == MODE::CLIENT_ALIAS_RO
                || ws_mode == MODE::CLIENT_ALL_RO
                || ws_mode == MODE::SERVNCLIENT_ALIAS_RO
                || ws_mode == MODE::SERVNCLIENT_ALL_RO)
                return StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported in the current mode", inputReq.type_req);
            return sendRequest_Command_DevClient(inputReq, connData, isBinary);
        }

        if (typeWsReq == TYPE_WS_REQ::ATTR_DEV_CLIENT) {
            if (ws_mode == MODE::SERVER)
                return StringProc::exceptionStringOut(inputReq.id, NONE, "This request type is not supported in the current mode", inputReq.type_req);
            return sendRequest_AttrClient(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
            if (!isServerMode()) {
                return StringProc::exceptionStringOut(inputReq.id, NONE, "This mode does not support commands of this type", inputReq.type_req);
            }
            if (!_isInitDs)
                return StringProc::exceptionStringOut(inputReq.id, NONE, _errorMessage, inputReq.type_req);
            {                
                string resp_json;
                string attrName;


                TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

                string errorMessage = checkPermissionForRequest(inputReq, connData, attrName, _wsds->deviceServer, typeWsReq);

                if (errorMessage.size())
                    return errorMessage;
                
                resp_json = "[12]";
                bool statusAttr;
                resp_json = groupOrDevice->sendAttrWr(inputReq, statusAttr);
                return resp_json;
            }
        }

        // В обычном случае не возвращается никогда
        return "{\"error\": \"Unknown Request\"}";
    }

    void WSTangoConn::checkUser(ConnectionData& connData)
    {
        connData.userCheckStatus = uc->getInformationFromCheckingUser(connData);
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
                groupOrDevice = unique_ptr<GroupForWs>(new GroupForWs(_deviceName, attrCommPipe));
            }
            else
                groupOrDevice = unique_ptr<DeviceForWs>(new DeviceForWs(_deviceName, attrCommPipe));
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

    bool WSTangoConn::initDeviceServer()
    {
        _errorMessage.clear();
        bool isInit = false;

        try {
            if (_isGroup) {
                groupOrDevice = unique_ptr<GroupForWs>(new GroupForWs(_deviceName));
            }
            else
                groupOrDevice = unique_ptr<DeviceForWs>(new DeviceForWs(_deviceName));
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


    string WSTangoConn::sendRequest_Command(const ParsedInputJson &inputReq, ConnectionData &connData, bool &isBinary)
    {
        string resp_json;
        string commandName;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        string errorMessage = checkPermissionForRequest(inputReq, connData, commandName, _wsds->deviceServer, typeWsReq);

        if (errorMessage.size())
            return errorMessage;

        if (_isGroup) {
            if (inputReq.type_req != "command_device" && inputReq.type_req != "command_group")
                return StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("command_name"), "type_req must be command_device or command_group", "command");
        }
        else {
            if (inputReq.type_req != "command")
                return StringProc::exceptionStringOut(inputReq.id, inputReq.otherInpStr.at("command_name"), "type_req must be command", "command");
        }

        bool statusComm;
        OUTPUT_DATA_TYPE odt = groupOrDevice->checkDataType(commandName);
        if (odt == OUTPUT_DATA_TYPE::JSON)
            resp_json = groupOrDevice->sendCommand(inputReq,statusComm);
        if (odt == OUTPUT_DATA_TYPE::BINARY) {
            resp_json = groupOrDevice->sendCommandBin(inputReq,statusComm);
            isBinary = true;
        }
        bool isSent = uc->sendLogCommand(inputReq, connData.remoteConf, _wsds->deviceServer, _isGroup, statusComm, typeWsReq);
        if (isSent)
            INFO_STREAM << "Information was sent to the log" << endl;
        return resp_json;
    }

    string WSTangoConn::sendRequest_Command_DevClient(const ParsedInputJson& inputReq, ConnectionData& connData, bool& isBinary)
    {
        string errorMessage;
        string device_name = checkDeviceNameKey(inputReq, errorMessage);

        if (errorMessage.size())
            return errorMessage;

        string commandName;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        errorMessage = checkPermissionForRequest(inputReq, connData, commandName, device_name, typeWsReq);

        if (errorMessage.size())
            return errorMessage;

        string resp_json;

        vector<string> commands{ commandName };
        try {
            DeviceForWs deviceForWs(device_name, commands);
            bool statusComm;
            OUTPUT_DATA_TYPE odt = deviceForWs.checkDataType(commands[0]);
            if (odt == OUTPUT_DATA_TYPE::JSON)
                resp_json = deviceForWs.sendCommand(inputReq, statusComm);
            if (odt == OUTPUT_DATA_TYPE::BINARY) {
                resp_json = deviceForWs.sendCommandBin(inputReq, statusComm);
                if (statusComm)
                    isBinary = true;
            }
            bool isSent = uc->sendLogCommand(inputReq, connData.remoteConf, device_name, _isGroup, statusComm, typeWsReq);
            if (isSent)
                INFO_STREAM << "Information was sent to the log" << endl;

            return resp_json;
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            return StringProc::exceptionStringOut(inputReq.id, NONE, errors, inputReq.type_req);
        }
    }

    string WSTangoConn::sendRequest_PipeComm(const ParsedInputJson &inputReq, ConnectionData &connData)
    {
        string resp_json;
        if (inputReq.check_key("pipe_name") != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key pipe_name or pipe_name is not value", inputReq.type_req);
        resp_json = groupOrDevice->sendPipeCommand(inputReq);
        removeSymbolsForString(resp_json);
        return resp_json;
    }

    string WSTangoConn::sendRequest_RidentReq(const ParsedInputJson &inputReq, ConnectionData &connData)
    {
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT2)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is RANDIDENT2", inputReq.type_req);

        if (connData.forRandIdent2.identState)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "You have already been authorized ", inputReq.type_req);

        std::uniform_int_distribution<int> dis(1000000, 9999999);
        stringstream json;
        int rand_ident = dis(generator);
        connData.forRandIdent2.login = "";

        if (inputReq.check_key("login") == TYPE_OF_VAL::VALUE)
            connData.forRandIdent2.login = inputReq.otherInpStr.at("login");

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
        connData.forRandIdent2.rand_ident = rand_ident;
        connData.forRandIdent2.isRandSended = true;
        return json.str();
    }

    string WSTangoConn::sendRequest_RidentAns(const ParsedInputJson &inputReq, ConnectionData &connData)
    {
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT2)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is RANDIDENT2", inputReq.type_req);

        if (connData.forRandIdent2.identState)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "You have already been authorized ", inputReq.type_req);

        if (!connData.forRandIdent2.isRandSended)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);

        if (inputReq.check_key("rident_hash") != TYPE_OF_VAL::VALUE){
            if (connData.forRandIdent2.isRandSended)
                connData.forRandIdent2.isRandSended = false;

            connData.forRandIdent2.rand_ident_hash = "";
            connData.forRandIdent2.rand_ident = 0;

            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }

        connData.forRandIdent2.rand_ident_hash = inputReq.otherInpStr.at("rident_hash");

        if (inputReq.check_key("login") == TYPE_OF_VAL::VALUE)
            connData.forRandIdent2.login = inputReq.otherInpStr.at("login");
        else if (!connData.forRandIdent2.login.size()) {
            connData.forRandIdent2.rand_ident_hash = "";
            connData.forRandIdent2.isRandSended = false;
            connData.forRandIdent2.rand_ident = 0;

            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }
        checkUser(connData);
        if (!connData.userCheckStatus.first){
            connData.forRandIdent2.rand_ident_hash = "";
            connData.forRandIdent2.isRandSended = false;
            connData.forRandIdent2.rand_ident = 0;
            return StringProc::exceptionStringOut(inputReq.id, NONE, connData.userCheckStatus.second + " Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }
        connData.forRandIdent2.identState = true;
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

    string WSTangoConn::sendRequest_Rident(const ParsedInputJson& inputReq, ConnectionData& connData) 
    {
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT3)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "This type_req is only used when TYPE_OF_IDENT is RANDIDENT3", inputReq.type_req);

        if (connData.forRandIdent2.identState)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "You have already been authorized ", inputReq.type_req);

        if (inputReq.check_keys({ "rident_hash", "login", "rident" }) != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Send request for rand_identification. For details read the manual.", inputReq.type_req);

        connData.forRandIdent2.rand_ident_hash = inputReq.otherInpStr.at("rident_hash");
        connData.forRandIdent2.login = inputReq.otherInpStr.at("login");
        connData.forRandIdent2.rand_ident_str = inputReq.otherInpStr.at("rident");
        checkUser(connData);

        if (!connData.userCheckStatus.first)
            return StringProc::exceptionStringOut(inputReq.id, NONE, connData.userCheckStatus.second + " Send request for rand_identification. For details read the manual.", inputReq.type_req);

        connData.forRandIdent2.identState = true;

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

    string WSTangoConn::sendRequest_AttrClient(const ParsedInputJson& inputReq, ConnectionData& connData)
    {
        string errorMessage;
        string device_name = checkDeviceNameKey(inputReq, errorMessage);

        if (errorMessage.size())
            return errorMessage;

        bool isAttrNotFound = false;
        bool isPipeNotFound = false;
        
        if (inputReq.check_key("attributes") != TYPE_OF_VAL::VALUE && inputReq.check_key("attributes") != TYPE_OF_VAL::ARRAY)
            isAttrNotFound = true;

        if (inputReq.check_key("pipes") != TYPE_OF_VAL::VALUE && inputReq.check_key("pipes") != TYPE_OF_VAL::ARRAY)
            isPipeNotFound = true;

        if (isAttrNotFound && isPipeNotFound)
            return StringProc::exceptionStringOut(inputReq.id, NONE, "Not found keys attributes / pipes or attributes / pipes is not value or array", inputReq.type_req);

        vector<string> attributes;
        vector<string> pipes;

        if (inputReq.check_key("attributes") == TYPE_OF_VAL::ARRAY)
            attributes = inputReq.otherInpVec.at("attributes");
        if (inputReq.check_key("attributes") == TYPE_OF_VAL::VALUE)
            attributes.push_back(inputReq.otherInpStr.at("attributes"));

        if (inputReq.check_key("pipes") == TYPE_OF_VAL::ARRAY)
            pipes = inputReq.otherInpVec.at("pipes");
        if (inputReq.check_key("pipes") == TYPE_OF_VAL::VALUE)
            pipes.push_back(inputReq.otherInpStr.at("pipes"));

        std::pair<vector<string>, vector<string>> attr_pipes;
        attr_pipes.first = attributes;
        attr_pipes.second  = pipes;
        try {
            DeviceForWs deviceForWs(device_name, attr_pipes);
            return deviceForWs.generateJsonForAttrReadCl(inputReq);
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            return StringProc::exceptionStringOut(inputReq.id, NONE, errors, inputReq.type_req);
        }        
    }

    string WSTangoConn::checkPermissionForRequest(const ParsedInputJson &inputReq, ConnectionData &connData, string &commandName, string device_name, TYPE_WS_REQ typeWsReq)
    {
        if (typeWsReq == TYPE_WS_REQ::COMMAND || typeWsReq == TYPE_WS_REQ::COMMAND_DEV_CLIENT) {

            if (inputReq.check_key("command_name") != TYPE_OF_VAL::VALUE)
                return StringProc::exceptionStringOut(inputReq.id, NONE, "Not found key command_name or command_name is not value", inputReq.type_req);
            commandName = inputReq.otherInpStr.at("command_name");
        }
        else if (typeWsReq == TYPE_WS_REQ::ATTRIBUTE_WRITE) {
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
        // дописать дополнительно проверку режима проверки.
        if (!connData.userCheckStatus.first && typeOfIdent != TYPE_OF_IDENT::RANDIDENT2 && typeOfIdent != TYPE_OF_IDENT::RANDIDENT3) {
            mess = connData.userCheckStatus.second;
            INFO_STREAM << mess << endl;
            return StringProc::exceptionStringOut(inputReq.id, commandName, mess, inputReq.type_req);
        }
        else if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT2 || typeOfIdent == TYPE_OF_IDENT::RANDIDENT3) {
            if (!connData.forRandIdent2.identState) {
                if (connData.forRandIdent2.isRandSended)
                    connData.forRandIdent2.isRandSended = false;
                if (connData.forRandIdent2.rand_ident)
                    connData.forRandIdent2.rand_ident = 0;
                connData.forRandIdent2.login = "";
                return StringProc::exceptionStringOut(inputReq.id, commandName, "Send request for rand_identification. For details read the manual.", inputReq.type_req);
            }
        }

        bool permission = uc->check_permission(inputReq, connData.remoteConf, device_name, _isGroup, mess, typeWsReq);

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
