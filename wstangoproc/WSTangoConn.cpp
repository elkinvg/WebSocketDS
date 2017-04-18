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
    WSTangoConn::WSTangoConn(WebSocketDS *dev, pair<string, string> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber)
    :Tango::LogAdapter(dev)
    {
        groupOrDevice = nullptr;
        wsThread = new WSThread_plain(this, portNumber);
        init_wstc(dev,dsAndOptions,attrCommPipe);
    }

    WSTangoConn::WSTangoConn(WebSocketDS *dev, pair<string, string> dsAndOptions, array<vector<string>, 3> attrCommPipe, int portNumber, string cert, string key)
    :Tango::LogAdapter(dev)
    {
        groupOrDevice = nullptr;
        wsThread = new WSThread_tls(this, portNumber, cert, key);
        init_wstc(dev,dsAndOptions,attrCommPipe);
    }

    void WSTangoConn::init_wstc(WebSocketDS *dev, pair<string, string> &dsAndOptions, array<vector<string>, 3> &attrCommPipe)
    {
        // ??? !!! IF _isInitDs FALSE ???
        initOptionsAndDeviceServer(dsAndOptions);
        if (_isInitDs) {
            groupOrDevice->initAttrCommPipe(attrCommPipe);

            _wsds = dev;
            uc = unique_ptr<UserControl> (new UserControl(dev->authDS, typeOfIdent, _isLogActive));
        }
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
            // State отсаётся ON, но в STATUS выводится сообщение
            _wsds->set_status(jsonStrOut);

            removeSymbolsForString(jsonStrOut);
            jsonStrOut = StringProc::exceptionStringOut(jsonStrOut);
        }
        else {
            if (_wsds->get_status() != "The listening server is running")
                _wsds->set_status("The listening server is running");
        }
        wsThread->send_all(jsonStrOut);
        return jsonStrOut;
    }

    string WSTangoConn::sendRequest(const ParsedInputJson& inputReq, bool& isBinary, ConnectionData& connData)
    {
        isBinary = false;
        TYPE_WS_REQ typeWsReq = getTypeWsReq(inputReq.type_req);

        if (typeWsReq == TYPE_WS_REQ::UNKNOWN) {
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "This request type is not supported", inputReq.type_req);
        }

        if (typeWsReq == TYPE_WS_REQ::COMMAND) {
            return sendRequest_Command(inputReq, connData, isBinary);
        }

        if (typeWsReq == TYPE_WS_REQ::PIPE_COMM) {
            return sendRequest_PipeComm(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::RIDENT_REQ) {
            return sendRequest_RidentReq(inputReq, connData);
        }

        if (typeWsReq == TYPE_WS_REQ::RIDENT_ANS) {
            return sendRequest_RidentAns(inputReq, connData);
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

    void WSTangoConn::initOptionsAndDeviceServer(pair<string, string>& dsAndOptions)
    {
        string options = dsAndOptions.second;
        _deviceName = dsAndOptions.first;

        if (options.size()) {
            std::vector<std::string> gettedOptions = StringProc::parseInputString(options, ";", true);
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
                        else
                            typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                    }
                }
                if (opt == "notshrtatt") {
                    _isShortAttr = false;
                }
            }
        }
        _isInitDs = initDeviceServer();
    }

    bool WSTangoConn::initDeviceServer()
    {
        errorMessage.clear();
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
            errorMessage = fromException(e, "WSTangoConn::initDeviceServer()");
        }
        return isInit;
    }

    string WSTangoConn::fromException(Tango::DevFailed &e, string func)
    {
        string outErrMess;
        auto lnh = e.errors.length();
        for (int i=0;i<lnh;i++) {
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
        return TYPE_WS_REQ::UNKNOWN;
    }

    string WSTangoConn::sendRequest_Command(const ParsedInputJson &inputReq, ConnectionData &connData, bool &isBinary)
    {
        string resp_json;
        if (inputReq.check_key("command_name") != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "Not found key command_name or command_name is not value", inputReq.type_req);
        string commandName = inputReq.otherInpStr.at("command_name");
        string mess;

        // Пока логин пароль (и прочие аут-данные) вводятся при запросе
        // В дальнейшем, когда будет режим проверки во время сессии
        // дописать дополнительно проверку режима проверки.
        if (!connData.userCheckStatus.first && typeOfIdent != TYPE_OF_IDENT::RANDIDENT2 ) {
            mess = connData.userCheckStatus.second;
            INFO_STREAM << mess << endl;
            return StringProc::exceptionStringOut(inputReq.id, commandName, mess, inputReq.type_req);
        }
        else if (typeOfIdent == TYPE_OF_IDENT::RANDIDENT2) {
            if (!connData.forRandIdent2.identState) {
                if (connData.forRandIdent2.isRandSended)
                    connData.forRandIdent2.isRandSended = false;
                if (connData.forRandIdent2.rand_ident)
                    connData.forRandIdent2.rand_ident = 0;
                connData.forRandIdent2.login = "";
                return StringProc::exceptionStringOut(inputReq.id, commandName, "Send request for rand_identification. For details read the manual.", inputReq.type_req);
            }
        }

        bool permission = uc->check_permission(inputReq, connData.remoteConf, _wsds->deviceServer, _isGroup, mess);

        if (mess.size())
            INFO_STREAM << mess << endl;

        if (!permission)
        {
            if (mess.size())
                return StringProc::exceptionStringOut(inputReq.id, commandName, mess, inputReq.type_req);
            else
                return StringProc::exceptionStringOut(inputReq.id, commandName, "Permission denied", inputReq.type_req);
        }

        bool statusComm;
        OUTPUT_DATA_TYPE odt = groupOrDevice->checkDataType(commandName);
        if (odt == OUTPUT_DATA_TYPE::JSON)
            resp_json = groupOrDevice->sendCommand(inputReq,statusComm);
        if (odt == OUTPUT_DATA_TYPE::BINARY) {
            resp_json = groupOrDevice->sendCommandBin(inputReq,statusComm);
            isBinary = true;
        }
        bool isSent = uc->sendLogCommand(inputReq, connData.remoteConf, _wsds->deviceServer, _isGroup, statusComm);
        if (isSent)
            INFO_STREAM << "Information was sent to the log" << endl;
        return resp_json;
    }

    string WSTangoConn::sendRequest_PipeComm(const ParsedInputJson &inputReq, ConnectionData &connData)
    {
        string resp_json;
        if (inputReq.check_key("pipe_name") != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "Not found key pipe_name or pipe_name is not value", inputReq.type_req);
        resp_json = groupOrDevice->sendPipeCommand(inputReq);
        removeSymbolsForString(resp_json);
        return resp_json;
    }

    string WSTangoConn::sendRequest_RidentReq(const ParsedInputJson &inputReq, ConnectionData &connData)
    {
        if (typeOfIdent != TYPE_OF_IDENT::RANDIDENT2)
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "This type_req is only used when TYPE_OF_IDENT is RANDIDENT2", inputReq.type_req);

        if (connData.forRandIdent2.identState)
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "You have already been authorized ", inputReq.type_req);

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
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "This type_req is only used when TYPE_OF_IDENT is RANDIDENT2", inputReq.type_req);

        if (connData.forRandIdent2.identState)
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "You have already been authorized ", inputReq.type_req);

        if (!connData.forRandIdent2.isRandSended)
            return StringProc::exceptionStringOut(inputReq.id, "unknown", "Send request for rand_identification. For details read the manual.", inputReq.type_req);

        if (inputReq.check_key("rident_hash") != TYPE_OF_VAL::VALUE){
            if (connData.forRandIdent2.isRandSended)
                connData.forRandIdent2.isRandSended = false;

            connData.forRandIdent2.rand_ident_hash = "";
            connData.forRandIdent2.rand_ident = 0;

            return StringProc::exceptionStringOut(inputReq.id, "unknown", "Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }

        connData.forRandIdent2.rand_ident_hash = inputReq.otherInpStr.at("rident_hash");

        if (inputReq.check_key("login") == TYPE_OF_VAL::VALUE)
            connData.forRandIdent2.login = inputReq.otherInpStr.at("login");
        else if (!connData.forRandIdent2.login.size()) {
            connData.forRandIdent2.rand_ident_hash = "";
            connData.forRandIdent2.isRandSended = false;
            connData.forRandIdent2.rand_ident = 0;

            return StringProc::exceptionStringOut(inputReq.id, "unknown", "Send request for rand_identification. For details read the manual.", inputReq.type_req);
        }
        checkUser(connData);
        if (!connData.userCheckStatus.first){
            connData.forRandIdent2.rand_ident_hash = "";
            connData.forRandIdent2.isRandSended = false;
            connData.forRandIdent2.rand_ident = 0;
            return StringProc::exceptionStringOut(inputReq.id, "unknown", connData.userCheckStatus.second + " Send request for rand_identification. For details read the manual.", inputReq.type_req);
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
}
