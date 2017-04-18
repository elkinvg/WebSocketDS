#include "GroupOrDeviceForWs.h"

#include "WebSocketDS.h"
#include "StringProc.h"
#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    GroupOrDeviceForWs::GroupOrDeviceForWs()
    {
        processor = unique_ptr<TangoProcessor>(new TangoProcessor());
    }

    GroupOrDeviceForWs::~GroupOrDeviceForWs() {}

    void GroupOrDeviceForWs::initAttrCommPipe(array<vector<string>, 3>& attrCommPipe)
    {
        initAttrAndPipe(attrCommPipe[0], attrCommPipe[2]);
        initComm(attrCommPipe[1]);
    }

    //--------------------------------------------------------
    /**
    *	Method      : checkDataType()
    *	Description : Checking type of returned data for command
    *
    *	@param commandName Name of command
    *	@returns Type of returned data. (JSON or binary)
    *
    */
    //--------------------------------------------------------

    OUTPUT_DATA_TYPE GroupOrDeviceForWs::checkDataType(string commandName)
    {
        pair<bool, string> optionBinData = processor->checkOption(commandName,"bindata",TYPE_WS_REQ::COMMAND);
        if (optionBinData.first)
            return OUTPUT_DATA_TYPE::BINARY;
        else
            return OUTPUT_DATA_TYPE::JSON;
    }

    void GroupOrDeviceForWs::initAttrAndPipe(vector<string> &attributes, vector<string>&pipeName)
    {
        //DEBUG_STREAM << "Attributes: " << endl;
        // Method gettingAttrUserConf added for Searhing of additional options for attributes
        // Now it is options "prec", "precf", "precs" for precision
        // And "niter"
        nIters.clear();
        nIters.reserve(attributes.size());

        // Лямда функция для парсинга niter. Формат niter=N niter=N/M
        // N - периодичность (unsigned short) вывода
        // M (unsigned short) - смещение относительно первой итерации периода
        // M < N
        auto getPairOfParams = [](string inp_param) {
            std::pair<unsigned short, unsigned short> outPair;
            outPair = std::make_pair(0, 0);
            size_t pos = 0;
            string delimiter = "/";

            if ((pos = inp_param.find(delimiter)) != std::string::npos) {
                string first = inp_param.substr(0, pos);
                inp_param.erase(0, pos + delimiter.length());
                try {
                    unsigned short tmp1 = stoi(first);
                    unsigned short tmp2 = stoi(inp_param);
                    if (tmp1 > tmp2) {
                        outPair.first = tmp1;
                        outPair.second = tmp2;
                    }
                }
                catch (...) {
                }
            }
            else {
                unsigned short tmpsz = 0;
                try {
                    tmpsz = stoi(inp_param);
                    outPair.first = tmpsz;
                }
                catch (...) {
                }
            }
            return outPair;
        };

        for (auto& attr : attributes) {
            string tmpAttrName = attr;
            std::transform(tmpAttrName.begin(), tmpAttrName.end(), tmpAttrName.begin(), ::tolower);
            isJsonAttribute.push_back(tmpAttrName.find("json") != std::string::npos);

            //DEBUG_STREAM << attr << endl;

            vector<string> gettedOptions = StringProc::parseInputString(attr, ";");
            processor->initOptionsForAttrOrComm(attr,gettedOptions, TYPE_WS_REQ::ATTRIBUTE);

            std::pair<unsigned short, unsigned short> tmpsz;
            tmpsz = std::make_pair(0, 0);
            // Если задан niter, производится парсинг, иначе (0,0)

            pair<bool, string> niterOpt = processor->checkOption(attr,"niter",TYPE_WS_REQ::ATTRIBUTE);
            if (niterOpt.first)
                tmpsz = getPairOfParams(niterOpt.second);
            nIters.push_back(tmpsz);
        }
        _attributes = attributes;
        nAttributes = _attributes.size();
        
        if (pipeName.size())
            _pipeAttr = pipeName[0];
        if (pipeName.size() > 1) {
            for (int i=1; i<pipeName.size(); i++) {
                string attrName = pipeName[i];
                vector<string> gettedOptions = StringProc::parseInputString(attrName, ";");
                processor->initOptionsForAttrOrComm(attrName,gettedOptions, TYPE_WS_REQ::PIPE);
            }
        }
    }

    void GroupOrDeviceForWs::initComm(vector<string> &commands)
    {
        //DEBUG_STREAM << "Commands: " << endl;

        // Список команд, доступных для выполения
        // При каждой попытке запуска команды, проверяются права на выполение,
        // а также наличие данной команды в accessibleCommandInfo

        accessibleCommandInfo.clear();

        for (auto& com : commands) {
            try {
                vector<string> gettedOptions = StringProc::parseInputString(com, ";");

                bool isPipeComm = false;
                for (auto &it: gettedOptions)
                    if (it == "pipecomm") {
                        isPipeComm = true;
                        break;
                    }

                if (isPipeComm) {
                    processor->initOptionsForAttrOrComm(com,gettedOptions, TYPE_WS_REQ::PIPE_COMM);
                }
                else {
                    processor->initOptionsForAttrOrComm(com,gettedOptions, TYPE_WS_REQ::COMMAND);

                    // Getting CommandInfo
                    // cmd_name , cmd_tag, in_type, in_type_desc, out_type, out_type_desc
                    Tango::CommandInfo info = getCommandInfo(com);

                    accessibleCommandInfo.insert(std::pair<std::string, Tango::CommandInfo>(com, info));
                }
                //DEBUG_STREAM << "Init " << com << endl;
            }
            catch (Tango::DevFailed &e)
            {
                //ERROR_STREAM << "command " << com << " not found" << endl;
            }
            catch (exception &e)
            {
                //ERROR_STREAM << "Command " << com << ": " << e.what() << endl;
            }
        }
    }

    void GroupOrDeviceForWs::generateAttrJson(std::stringstream& json, std::vector<Tango::DeviceAttribute> *attrList) {
        int it = 0;
        for (int i = 0; i < nAttributes; i++)
        {
            // Если задан niter для данного атрибута
            // Вывод будет только если iterator кратно nIters
            if (nIters[i].first != 0) {
                if ((iterator + (nIters[i].first - nIters[i].second)) % nIters[i].first != 0)
                    continue;
            }
            if (it != 0) json << ", ";
            it++;
            Tango::DeviceAttribute att = attrList->at(i);
            if (isJsonAttribute.at(i))
                json << processor->process_device_attribute_json(att);
            else
                json << processor->process_attribute_t(att, _isShortAttr);
        }
    }

    Tango::DeviceData GroupOrDeviceForWs::tangoCommandInoutForDevice(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& dataFromJson, string& errorMessInJson)
    {
        Tango::DeviceData outDeviceData;
        string commandName = dataFromJson.otherInpStr.at("command_name");

        if (accessibleCommandInfo.find(commandName) == accessibleCommandInfo.end()) {
            errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, "This Command not found in the list of available commands or not found on DeviceServer", dataFromJson.type_req);
            return outDeviceData;
        }

        Tango::CommandInfo comInfo = accessibleCommandInfo[commandName];
        int type = comInfo.in_type;

        // Вызов правильного метода  command_inout
        // Проверка типа входных аргументов Void, Array, Data
        try {
            if (type == Tango::DEV_VOID) {
                outDeviceData = deviceProxy->command_inout(commandName);
            }
            else {
                if (dataFromJson.check_key("argin") == TYPE_OF_VAL::NONE) {
                    errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, "argin not found", dataFromJson.type_req);
                    return outDeviceData;
                }

                // если argin - массив
                // и если требуемый type не является массивом
                if (dataFromJson.check_key("argin") == TYPE_OF_VAL::ARRAY && !processor->isMassive(type)) {
                    errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, "The input data should not be an array", dataFromJson.type_req);
                    return outDeviceData;
                }

                Tango::DeviceData inDeviceData;
                inDeviceData = processor->getDeviceDataFromParsedJson(dataFromJson, type);

                outDeviceData = deviceProxy->command_inout(commandName, inDeviceData);
            }
        }
        catch (Tango::DevFailed &e) {
            string tangoErrors;

            for (int i = 0; i < e.errors.length(); i++) {
                if (i > 0)
                    tangoErrors += " ||| ";
                tangoErrors += (string)e.errors[i].desc;
            }

            errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, tangoErrors, dataFromJson.type_req);
        }
        catch (std::exception &exc) {
            // if cannot convert input str
            errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, exc.what(), dataFromJson.type_req);
        }

        return outDeviceData;
    }

    void GroupOrDeviceForWs::generateJsonHeadForPipeComm(const ParsedInputJson& parsedInput, stringstream &json)
    {        
        json << "{\"event\": \"read\", \"type_req\": \"" << parsedInput.type_req << "\", ";
        json << "\"pipe_name\": \"" << parsedInput.otherInpStr.at("pipe_name") << "\", ";

        if (parsedInput.type_req == "read_pipe_dev")
            json << "\"device_name\": " << "\"" << parsedInput.otherInpStr.at("device_name") << "\", ";

        try {
            auto idTmp = stoi(parsedInput.id);
            json << "\"id_req\": "  << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (parsedInput.id == NONE)
                json << "\"id_req\": " << parsedInput.id << ", ";
            else
                json << "\"id_req\": \"" << parsedInput.id << "\", ";
        }

        json << "\"data\": ";
    }

    string GroupOrDeviceForWs::sendCommandBinForDevice(Tango::DeviceProxy *deviceProxy, const ParsedInputJson& parsedInput, bool& statusComm)
    {
        statusComm = false;
        string argout;
        string commandName = parsedInput.otherInpStr.at("command_name");
        string typeReq = parsedInput.type_req;


        string errorMess;

        Tango::DeviceData outDeviceData = tangoCommandInoutForDevice(deviceProxy, parsedInput, errorMess);

        // Если при отправлении команды на девайс выявлена ошибка
        if (errorMess.size())
            return errorMess.insert(0,ERR_PRED);

        statusComm = true;

        int type = outDeviceData.get_type();

        if (type == Tango::DEVVAR_CHARARRAY) {
            const Tango::DevVarCharArray *vcharr;
            try {
                outDeviceData >> vcharr;
                for (int i = 0; i < vcharr->length(); i++) {
                    argout.push_back((*vcharr)[i]);
                }
            }
            catch (Tango::DevFailed &e) {
                argout = StringProc::exceptionStringOut(parsedInput.id, commandName, "Exception From sendCommandBin", typeReq).insert(0, ERR_PRED);
            }
        }
        else
        {
            argout = StringProc::exceptionStringOut(parsedInput.id, commandName, "Output data must be Tango::DevVarCharArray", typeReq).insert(0, ERR_PRED);
        }

        return argout;
    }
}
