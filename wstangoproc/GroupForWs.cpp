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

    GroupForWs::GroupForWs(string pattern, std::pair<vector<string>, vector<string> > &attr_pipes)
        :GroupForWs(pattern)
    {
        initAttr(attr_pipes.first);
        initPipe(attr_pipes.second);
    }

    GroupForWs::GroupForWs(string pattern, array<vector<string>,3> &attrCommPipe)
        :GroupForWs(pattern)
    {
        initAttr(attrCommPipe[0]);
        initComm(attrCommPipe[1]);
        initPipe(attrCommPipe[2]);
    }

    GroupForWs::GroupForWs(string pattern, vector<string> &commands)
        :GroupForWs(pattern)
    {
        initComm(commands);
    }

    GroupForWs::~GroupForWs()
    {
        if (group != nullptr)
            delete group;
    }

    string GroupForWs::generateJsonForUpdate()
    {
        std::stringstream json;

        json << "{\"event\": \"read\", \"type_req\": \"group_attribute\", \"data\": {";
        int it = 0;
        for (long i = 0; i < group_length; i++)
        {

            if (it != 0)
                json << ", ";
            std::vector<Tango::DeviceAttribute> *attrList = nullptr;
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
            if (attrList != nullptr) {
                delete attrList;
            }
        }
        json << "}";

        
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

    void GroupForWs::generateJsonForUpdate(stringstream &json)
    {
        //  Должен использоваться при запуске update_data со стороны клиента
        // ??? !!! Пока только для девайсов, а не для групп
    }

    void GroupForWs::generateJsonForAttrReadCl(const ParsedInputJson& parsedInput, std::stringstream& json)
    {
        // ??? !!! Пока только для девайсов, а не для групп
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
            return StringProc::exceptionStringOut(parsedInput.id, commandName, "type_req must be command_device", "command");


        if (parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE)
            return StringProc::exceptionStringOut(parsedInput.id, commandName, "This request (command_device) must contain a key device_name", parsedInput.type_req);

        string deviceName = parsedInput.otherInpStr.at("device_name");

        Tango::DeviceProxy *dp;

            try
            {
                // Получение девайса по имени. Если данный девайс не входит в группу
                // или нет доступа к нему, высылается сообщение об ошибке
                dp = group->get_device(deviceName);
                if (dp == 0)
                {
                    return StringProc::exceptionStringOut(parsedInput.id, commandName, deviceName + " does not belongs to the group", parsedInput.type_req);
                }
            }
            catch (const Tango::DevFailed &df)
            {
                return StringProc::exceptionStringOut(parsedInput.id, commandName, deviceName + "  belongs to the group but can’t be reached", parsedInput.type_req);
            }

            return sendCommandBinForDevice(dp, parsedInput, statusComm);
    }

    string GroupForWs::sendAttrWr(const ParsedInputJson& parsedInput, bool& statusComm)
    {
        statusComm = false;

        try {
            string attr_name = parsedInput.otherInpStr.at("attr_name");

            if (isWrtAttribute.find(attr_name) == isWrtAttribute.end())
                return StringProc::exceptionStringOut(parsedInput.id, NONE, "Attribute " + attr_name + " is not included in the list of writable attributes, Or it is not writable. Read README.md for information", parsedInput.type_req);

            if (parsedInput.type_req == "write_attr_gr") {
                auto devices_names = group->get_device_list();

                if (!devices_names.size()) {
                    return StringProc::exceptionStringOut(parsedInput.id, attr_name, "Group does not contain any devices", parsedInput.type_req);
                }

                Tango::DeviceAttribute attr;

                // Getting information about attribute
                for (auto& dev : devices_names) {
                    try {
                        Tango::AttributeInfoEx attr_info = group->get_device(dev)->attribute_query(attr_name);
                        vector<string> errors;
                        attr = processor->getDeviceAttributeDataFromJson(parsedInput, attr_info, errors);
                        break;
                    }
                    catch (...) {}
                } 

                Tango::GroupReplyList replyList = group->write_attribute(attr, true);

                bool hasFailed = false;
                vector<string> devices_failed;

                for (auto& reply : replyList) {
                    if (reply.has_failed()) {
                        hasFailed = true;
                        devices_failed.push_back("Device: " + reply.dev_name() + " is not available");
                    }
                }

                if (hasFailed) {
                    if (devices_failed.size() == devices_names.size())
                        return StringProc::exceptionStringOut(parsedInput.id, NONE, "All devices from group are not available", parsedInput.type_req);
                    else 
                        return StringProc::responseStringOut(parsedInput.id, devices_failed, parsedInput.type_req);
                }
            }

            if (parsedInput.type_req == "write_attr_dev") {
                if (parsedInput.check_key("device_name") != TYPE_OF_VAL::VALUE)
                    return StringProc::exceptionStringOut(parsedInput.id, NONE, "Not found key device_name or device_name is not value", parsedInput.type_req);

                string device_name = parsedInput.otherInpStr.at("device_name");

                Tango::DeviceProxy *dp;
                try {
                    dp = group->get_device(device_name);
                    if (dp == 0) {
                        return StringProc::exceptionStringOut(parsedInput.id, attr_name, device_name + " does not belongs to the group", parsedInput.type_req);
                    }
                }
                catch (const Tango::DevFailed &df) {
                    return StringProc::exceptionStringOut(parsedInput.id, attr_name, device_name + "  belongs to the group but can’t be reached", parsedInput.type_req);
                }

                Tango::AttributeInfoEx attr_info = dp->attribute_query(attr_name);
                vector<string> errors;
                Tango::DeviceAttribute attr = processor->getDeviceAttributeDataFromJson(parsedInput, attr_info, errors);

                if (errors.size())
                    return StringProc::exceptionStringOut(parsedInput.id, NONE, errors, parsedInput.type_req);

                dp->write_attribute(attr);
                
            }
            statusComm = true;
        }
        catch (Tango::DevFailed& e) {
            vector<string> errors;
            for (int i = 0; i < e.errors.length(); i++) {
                errors.push_back((string)e.errors[i].desc);
            }
            return StringProc::exceptionStringOut(parsedInput.id, NONE, errors, parsedInput.type_req);
        }
        catch (std::exception &exc) {
            return StringProc::exceptionStringOut(parsedInput.id, NONE, exc.what(), parsedInput.type_req);
        }

        return StringProc::responseStringOut(parsedInput.id, "Was written to the attribute", parsedInput.type_req);
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

    bool GroupForWs::checkIsAttributeWriteble(const string& attr_name)
    {
        vector<string> device_list = group->get_device_list(true);

        if (!device_list.size())
            return false;

        // Здесь проверяется writable 
        // Должны совпадать  для всех атрибутов

        for (auto& device : device_list) {
            Tango::DeviceProxy *dp;
            dp = group->get_device(device);

            if (dp == 0)
                return false;

            try {
                Tango::AttributeInfoEx attr_info = dp->get_attribute_config(attr_name);
                
                if (attr_info.writable != Tango::AttrWriteType::READ_WRITE &&
                    attr_info.writable != Tango::AttrWriteType::WRITE)
                    return false;
            }
            catch (Tango::DevFailed)
            {
                return false;
            }
        }

        return true;
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
            vector<string> tangoErrors;

            for (unsigned int i = 0; i < e.errors.length(); i++) {
                tangoErrors.push_back((string)e.errors[i].desc);
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
        catch (Tango::DevFailed &e) {}

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
            for (unsigned int i = 0; i < e.errors.length(); i++)
                errors.push_back((string)e.errors[i].desc);

            return StringProc::exceptionStringOut(parsedInput.id, pipeName, errors, parsedInput.type_req);
        }

        return json.str();
    }

    bool GroupForWs::initAllAttrs() {
        return false;
    }
}
