#include "GroupOrDeviceForWs.h"

#include "WebSocketDS.h"
#include "StringProc.h"
#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    GroupOrDeviceForWs::GroupOrDeviceForWs(WebSocketDS *dev):
        Tango::LogAdapter(dev)
    {
        ds = dev;
        processor = unique_ptr<TangoProcessor>(new TangoProcessor());
    }

    GroupOrDeviceForWs::~GroupOrDeviceForWs() {}

    void GroupOrDeviceForWs::initAttrComm(vector<string> &attributes, vector<string> &commands)
    {
        initAttr(attributes);
        initComm(commands);
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

    void GroupOrDeviceForWs::initAttr(vector<string> &attributes)
    {
        DEBUG_STREAM << "Attributes: " << endl;
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

            DEBUG_STREAM << attr << endl;

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
    }

    void GroupOrDeviceForWs::initComm(vector<string> &commands)
    {
        DEBUG_STREAM << "Commands: " << endl;

        // Список команд, доступных для выполения
        // При каждой попытке запуска команды, проверяются права на выполение,
        // а также наличие данной команды в accessibleCommandInfo

        accessibleCommandInfo.clear();

        for (auto& com : commands) {
            try {
                vector<string> gettedOptions = StringProc::parseInputString(com, ";");
                processor->initOptionsForAttrOrComm(com,gettedOptions, TYPE_WS_REQ::COMMAND);

                // Getting CommandInfo
                // cmd_name , cmd_tag, in_type, in_type_desc, out_type, out_type_desc
                Tango::CommandInfo info = getCommandInfo(com);

                accessibleCommandInfo.insert(std::pair<std::string, Tango::CommandInfo>(com, info));
                DEBUG_STREAM << "Init " << com << endl;
            }
            catch (Tango::DevFailed &e)
            {
                ERROR_STREAM << "command " << com << " not found" << endl;
            }
            catch (exception &e)
            {
                ERROR_STREAM << "Command " << com << ": " << e.what() << endl;
            }
        }
    }

    Tango::DevVarCharArray* GroupOrDeviceForWs::errorMessageToCharArray(const string& errorMessage)
    {
        Tango::DevVarCharArray *errout = new Tango::DevVarCharArray();
        errout->length(3 + errorMessage.size());
        (*errout)[0] = 'e';
        (*errout)[1] = 'r';
        (*errout)[2] = 'r';
        for (unsigned int i = 0; i < errorMessage.size(); i++) {
            (*errout)[i + 3] = errorMessage[i];
        }
        return errout;
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
                json << processor->process_attribute_t(att);
        }
    }

    Tango::DeviceData GroupOrDeviceForWs::tangoCommandInoutForDevice(Tango::DeviceProxy *deviceProxy, Tango::DevString &argin, string &commandName,const string &arginStr, const string idStr, string& errorMess)
    {
        Tango::DeviceData outDeviceData;
        string typeReq = "command_device";

        if (accessibleCommandInfo.find(commandName) != accessibleCommandInfo.end())
        {
            Tango::CommandInfo comInfo = accessibleCommandInfo[commandName];
            int type = comInfo.in_type;

            // Вызов правильного метода  command_inout
            // Проверка типа входных аргументов Void, Array, Data
            try {
                if (type == Tango::DEV_VOID) {
                    outDeviceData = deviceProxy->command_inout(commandName);
                }
                else {
                    if (arginStr == NONE) {
                        errorMess = StringProc::exceptionStringOut(idStr, commandName, "argin not found", typeReq);
                        return outDeviceData;
                    }

                    // если argin - массив
                    // и если требуемый type не является массивом
                    if (arginStr == "ARRAY" && !processor->isMassive(type)) {
                        errorMess = StringProc::exceptionStringOut(idStr, commandName, "The input data should not be an array", typeReq);
                        return outDeviceData;
                    }

                    Tango::DeviceData inDeviceData;
                    inDeviceData = processor->gettingDevDataFromJsonStr(argin, type);

                    outDeviceData = deviceProxy->command_inout(commandName, inDeviceData);

                }
            }
            catch (Tango::DevFailed &e) {
                errorMess = StringProc::exceptionStringOut(idStr, commandName, "Exception from command_inout. Check the format of the data", typeReq);
            }
        }
        else {
            errorMess = StringProc::exceptionStringOut(idStr, commandName, "This Command not found in the list of available commands or not found on DeviceServer", typeReq);
        }

        return outDeviceData;
    }

    Tango::DevVarCharArray* GroupOrDeviceForWs::sendCommandBinForDevice(Tango::DeviceProxy *deviceProxy, Tango::DevString &argin, const std::map<std::string, std::string> &jsonArgs)
    {
        Tango::DevVarCharArray *argout;

        // Имя команды определено либо в jsonArgs["command"] либо
        // в jsonArgs["command_device"] в зависимости от типа
        // Проверка данных ключей производится в методах:
        //       WebSocketDS::send_command_bin
        //       WebSocketDS::send_command
        // вызовом метода StringProc::parseJsonFromCommand

        string commandName;
        string typeReq;
        try {
            commandName = jsonArgs.at("command");
            typeReq = "command";
        }
        catch (const std::out_of_range&) {
            commandName = jsonArgs.at("command_device");
            typeReq = "command_device";
        }


        string errorMess;
        string arginStr = jsonArgs.at("argin");
        string idStr = jsonArgs.at("id");

        Tango::DeviceData outDeviceData = tangoCommandInoutForDevice(deviceProxy, argin, commandName, arginStr, idStr, errorMess);

        // Если при отправлении команды на девайс выявлена ошибка
        if (errorMess.size())
            return errorMessageToCharArray(errorMess);

        int type = outDeviceData.get_type();
        argout = new Tango::DevVarCharArray();

        if (type == Tango::DEVVAR_CHARARRAY) {
            const Tango::DevVarCharArray *vcharr;

            try {
                outDeviceData >> vcharr;
                argout->length(vcharr->length());
                for (int i = 0; i < vcharr->length(); i++) {
                    (*argout)[i] = (*vcharr)[i];
                }
            }
            catch (Tango::DevFailed &e) {
                argout = errorMessageToCharArray(StringProc::exceptionStringOut(idStr, commandName, "Exception From sendCommandBin", typeReq));
            }
        }
        else
        {
            argout = errorMessageToCharArray(StringProc::exceptionStringOut(idStr, commandName, "Output data must be Tango::DevVarCharArray", typeReq));
        }

        return argout;
    }
}
