#include "GroupForWs.h"
#include "StringProc.h"

#include "TangoProcessor.h"
#include "WebSocketDS.h"

namespace WebSocketDS_ns
{
    GroupForWs::GroupForWs(string pattern)
    {
        group = new Tango::Group("forws");
        group->add(pattern);
        group_length = group->get_size(true);
        group->set_timeout_millis(3000);
        deviceList = group->get_device_list(true);
    }

    GroupForWs::~GroupForWs()
    {
        if (group != nullptr)
            delete group;
    }

    string GroupForWs::generateJsonForUpdate()
    {
        std::stringstream json;
        std::vector<Tango::DeviceAttribute> *attrList = nullptr;

        json << "{\"event\": \"read\", \"type_req\": \"group_attribute\", \"data\": {";
        int it = 0;
        for (long i = 0; i < group_length; i++)
        {
            if (it != 0)
                json << ", ";

            attrList = getAttributeList(deviceList[i], _attributes);
            it++;

            json << "\"" << deviceList[i] << "\": ";

            if (attrList == nullptr) {
                json << "\"Device unavailable. Check the status of corresponding tango-server\"";
                continue;
            }

            json << "[";
            generateAttrJson(json, attrList);
            json << "]";
        }
        json << "}";

        if (attrList != nullptr)
            delete attrList;
        
        if (_pipeAttr.size()) {
            it = 0;
            json << ", \"pipe\": {";
            for (long i = 0; i < group_length; i++)
            {
                if (it != 0)
                    json << ", ";
                Tango::DeviceProxy *dp;

                try {
                    dp = group->get_device(deviceList[i]);
                    
                    if (dp == 0)
                        continue;
                    it++;
                    
                    json << "\"" << deviceList[i] << "\": ";
                    Tango::DevicePipe devicePipe = dp->read_pipe(_pipeAttr);
                    json << processor->processPipe(devicePipe, TYPE_WS_REQ::PIPE);
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
            json << "}";
        }

        iterator++;

        json << "}";
        return json.str();
    }

    string GroupForWs::sendPipeCommand(const ParsedInputJson& parsedInput)
    {
        string output;
        string pipeName = parsedInput.otherInpStr.at("pipe_name");
        if (parsedInput.type_req != "read_pipe_gr" && parsedInput.type_req != "read_pipe_dev")
            return StringProc::exceptionStringOut(parsedInput.id, pipeName, "type_req must be read_pipe_gr or read_pipe_dev", parsedInput.type_req);

        if (parsedInput.type_req == "read_pipe_dev") {
            if (parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE)
                return StringProc::exceptionStringOut(parsedInput.id, pipeName, "This request (command_device) must contain a key device_name", parsedInput.type_req);
            output = generateJsonFromPipeCommForDeviceFromGroup(parsedInput);
        }

        if (parsedInput.type_req == "read_pipe_gr") 
            output = generateJsonFromPipeCommForGroup(parsedInput);

        return output;
    }

    string GroupForWs::sendCommand(const ParsedInputJson& parsedInput, bool& statusComm)
    {
        statusComm = false;
        if (parsedInput.type_req != "command_device" && parsedInput.type_req != "command_group")
            return StringProc::exceptionStringOut(parsedInput.id, parsedInput.otherInpStr.at("command_name"), "type_req must be command_device or command_group", "command");

        // command_device or command_group
        string resp;
        string commandName = parsedInput.otherInpStr.at("command_name");

        if (parsedInput.type_req == "command_device") {
            
            if (parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE)
                return StringProc::exceptionStringOut(parsedInput.id, commandName, "This request (command_device) must contain a key device_name", parsedInput.type_req);
            string deviceName = parsedInput.otherInpStr.at("device_name");
            Tango::DeviceProxy *dp;
            try {
                dp = group->get_device(deviceName);
                if (dp == 0) {
                    return StringProc::exceptionStringOut(parsedInput.id, commandName, deviceName + " does not belongs to the group" , parsedInput.type_req);
                }
            }
            catch (const Tango::DevFailed &df) {
                return StringProc::exceptionStringOut(parsedInput.id, commandName, deviceName + "  belongs to the group but can’t be reached" , parsedInput.type_req);
            }

            string errorMessInJson;
            Tango::DeviceData dd = tangoCommandInoutForDevice(dp,parsedInput,errorMessInJson);
            if (errorMessInJson.size())
                return errorMessInJson;

            resp =  processor->getJsonStrFromDevData(dd, parsedInput);
            statusComm = true;
        }
        
        if (parsedInput.type_req == "command_group") {
            string errorMessInJson;
            Tango::GroupCmdReplyList replyList = tangoCommandInoutForGroup(parsedInput, errorMessInJson);

            if (errorMessInJson.size())
                return errorMessInJson;

            resp = processor->getJsonStrFromGroupCmdReplyList(replyList, parsedInput);
            statusComm = true;
        }

        return resp;
    }

    string GroupForWs::sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm)
    {
        statusComm = false;
        // Отправление команды производится только отдельным девайсам из группы
        // Для всей группы команда не выполняется
        string commandName = parsedInput.otherInpStr.at("command_name");

        if (parsedInput.type_req != "command_device" )
            return StringProc::exceptionStringOut(parsedInput.id, commandName, "type_req must be command_device", "command").insert(0, ERR_PRED);


        if (parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(parsedInput.id, commandName, "This request (command_device) must contain a key device_name", parsedInput.type_req).insert(0, ERR_PRED);

        string deviceName = parsedInput.otherInpStr.at("device_name");

        Tango::DeviceProxy *dp;

            try
            {
                // Получение девайса по имени. Если данный девайс не входит в группу
                // или нет доступа к нему, высылается сообщение об ошибке
                dp = group->get_device(deviceName);
                if (dp == 0)
                {
                    return StringProc::exceptionStringOut(parsedInput.id, commandName, deviceName + " does not belongs to the group", parsedInput.type_req).insert(0, ERR_PRED);
                }
            }
            catch (const Tango::DevFailed &df)
            {
                return StringProc::exceptionStringOut(parsedInput.id, commandName, deviceName + "  belongs to the group but can’t be reached", parsedInput.type_req).insert(0, ERR_PRED);
            }

            return sendCommandBinForDevice(dp, parsedInput, statusComm);
    }

    Tango::CommandInfo GroupForWs::getCommandInfo(const string& command_name)
    {
        Tango::CommandInfo ci_out;
        // Здесь проверяется идентичность команд для для девайсов из группы
        // Должны совпадать  CommandInfo команд для всех девайсов

        vector<string> device_list = group->get_device_list(true);
        bool first_iter = true;
        for (auto& device : device_list) {
            Tango::DeviceProxy *dp;
            dp = group->get_device(device);

            if (dp == 0)
                continue;

            Tango::CommandInfo ci = dp->command_query(command_name);
            if (first_iter) {
                ci_out = ci;
                first_iter = false;
            }
            else {
                if (ci == ci_out)
                    continue;
                else
                    throw GroupForWsException();
            }
        }
        return ci_out;
    }

    Tango::GroupCmdReplyList GroupForWs::tangoCommandInoutForGroup(const ParsedInputJson& dataFromJson, string& errorMessInJson)
    {
        Tango::GroupCmdReplyList deviceDataList;
        string commandName = dataFromJson.otherInpStr.at("command_name");

        if (accessibleCommandInfo.find(commandName) == accessibleCommandInfo.end()) {
            errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, "This Command not found in the list of available commands or not found on DeviceServer", dataFromJson.type_req);
            return deviceDataList;
        }

        Tango::CommandInfo comInfo = accessibleCommandInfo[commandName];
        int type = comInfo.in_type;

        try{
            if (type == Tango::DEV_VOID)
                deviceDataList = group->command_inout(commandName, true);
            else {
                if (dataFromJson.check_key("argin") == TYPE_OF_VAL::NONE) {
                    errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, "argin not found", dataFromJson.type_req);
                    return deviceDataList;
                }

                // если argin - массив
                // и если требуемый type не является массивом
                if (dataFromJson.check_key("argin") == TYPE_OF_VAL::ARRAY && !processor->isMassive(type)) {
                    errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, "The input data should not be an array", dataFromJson.type_req);
                    return deviceDataList;
                }

                Tango::DeviceData inDeviceData;
                inDeviceData = processor->getDeviceDataFromParsedJson(dataFromJson, type);

                deviceDataList = group->command_inout(commandName, inDeviceData, true);
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
            errorMessInJson = StringProc::exceptionStringOut(dataFromJson.id, commandName, exc.what(), dataFromJson.type_req);
        }

        return deviceDataList;
    }

    std::vector<Tango::DeviceAttribute>* GroupForWs::getAttributeList(const  string& device_name_i, vector<string> &attributes)
    {
        std::vector<Tango::DeviceAttribute>* devAttrList = nullptr;
        Tango::DeviceProxy *dp;

        try {
            dp = group->get_device(device_name_i);

            if (dp != 0) 
                devAttrList = dp->read_attributes(attributes);
        }
        catch (Tango::DevFailed &e) {
        }

        return devAttrList;
    }

    string GroupForWs::generateJsonFromPipeCommForGroup(const ParsedInputJson& parsedInput)
    {
        string pipeName = parsedInput.otherInpStr.at("pipe_name");
        vector<string> errorsMess;

        std::stringstream json;
        generateJsonHeadForPipeComm(parsedInput, json);
        json << "{";
        vector<string> device_list;
        try {
            device_list = group->get_device_list(true);
        }
        catch(Tango::DevFailed &e) {
            return StringProc::exceptionStringOut(parsedInput.id, pipeName, "Device list not received", parsedInput.type_req);
        }

        int it = 0;
        bool hasActDev = false;
        bool hasPipe = false;
        for (auto& deviceFromGroup : device_list) {
            Tango::DeviceProxy *dp = group->get_device(deviceFromGroup);
            if (it != 0)
                json << ", ";
            it++;

            json << "\"" << deviceFromGroup << "\": ";

            if (dp == 0) {
                string tmpErrMess = "Device unavailable. Check the status of corresponding tango-server";
                json << "\"" << tmpErrMess << "\"";
                errorsMess.push_back(deviceFromGroup + ": " + tmpErrMess);
                continue;
            }
            hasActDev = true;

            try {
                DevicePipe devicePipe = dp->read_pipe(pipeName);
                json << processor->processPipe(devicePipe, TYPE_WS_REQ::PIPE_COMM);
                hasPipe = true;
            }
            catch (Tango::DevFailed &e){
                json << "[";
                string tmpErrMess;
                for (int i = 0; i < e.errors.length(); i++) {
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
            return StringProc::exceptionStringOut(parsedInput.id, pipeName, "All device unavailable. Check the status of corresponding tango-server", parsedInput.type_req);

        return StringProc::exceptionStringOut(parsedInput.id, pipeName, errorsMess, parsedInput.type_req);

    }

    string GroupForWs::generateJsonFromPipeCommForDeviceFromGroup(const ParsedInputJson& parsedInput)
    {
        string pipeName = parsedInput.otherInpStr.at("pipe_name");
        string device_name = parsedInput.otherInpStr.at("device_name");

        std::stringstream json;
        try{
            Tango::DeviceProxy *dp = group->get_device(device_name);

            if (dp == 0)
                return StringProc::exceptionStringOut(parsedInput.id, pipeName, "Device " + device_name + " unavailable.Check the status of corresponding tango - server", parsedInput.type_req);

            generateJsonHeadForPipeComm(parsedInput, json);
            DevicePipe devicePipe = dp->read_pipe(pipeName);
            json << processor->processPipe(devicePipe, TYPE_WS_REQ::PIPE_COMM);
            json << "}";
        }
        catch (Tango::DevFailed &e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++)
                errors.push_back((string)e.errors[i].desc);

            return StringProc::exceptionStringOut(parsedInput.id, pipeName, errors, parsedInput.type_req);
        }

        return json.str();
    }
}