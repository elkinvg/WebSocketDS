#include "GroupForWs.h"
#include "StringProc.h"

#include "TangoProcessor.h"
#include "WebSocketDS.h"

namespace WebSocketDS_ns
{
    GroupForWs::GroupForWs(WebSocketDS *dev, string pattern):
        GroupOrDeviceForWs(dev)
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
        
        // if reading from Pipe
        if (ds->pipeName.size()) {
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
                    Tango::DevicePipe devicePipe = dp->read_pipe(ds->pipeName[0]);
                    json << processor->processPipe(devicePipe);
                }
                catch (Tango::DevFailed &e) {
                    json << "\"Exception\"";
                }
            }
            json << "}";
        }

        iterator++;

        json << "}";
        return json.str();
    }

    Tango::DevString GroupForWs::sendCommand(Tango::DevString &argin)
    {
        std::map<std::string, std::string> jsonArgs = StringProc::parseJsonFromCommand(argin, ds->isGroup());

        if (jsonArgs.at("command_group") != NONE)
            return sendCommandToGroup(argin,jsonArgs);

        else if (jsonArgs.at("command_device") != NONE) {
            if (jsonArgs.at("device_name") == NONE)
                return CORBA::string_dup(StringProc::exceptionStringOut(jsonArgs.at("id"
                ), jsonArgs.at("command_device"), "Object device_name not found. Check format of json request.", "command_device").c_str());
            return sendCommandToDevice(argin,jsonArgs);
        }

        return CORBA::string_dup(StringProc::exceptionStringOut(jsonArgs.at("id"
            ), jsonArgs.at("command_group"), "Command not found. Check format of json request.", "command_group").c_str());
    }

    Tango::DevVarCharArray* GroupForWs::sendCommandBin(Tango::DevString &argin)
    {
        // Отправление команды производится только отдельным девайсам из группы
        // Для всей группы команда не выполняется
        
        std::map<std::string, std::string> jsonArgs = StringProc::parseJsonFromCommand(argin, ds->isGroup());
        
        if (jsonArgs.at("command_device") != NONE) {
            if (jsonArgs.at("device_name") == NONE)
                return errorMessageToCharArray(StringProc::exceptionStringOut(jsonArgs.at("id"), jsonArgs.at("command_device"), "Not found object device_name in input JSON", "command_device"));

            Tango::DeviceProxy *dp;
            string deviceName = jsonArgs.at("device_name");
            try
            {
                // Получение девайса по имени. Если данный девайс не входит в группу
                // или нет доступа к нему, высылается сообщение об ошибке
                dp = group->get_device(deviceName);
                if (dp == 0)
                {
                    return errorMessageToCharArray(StringProc::exceptionStringOut(jsonArgs.at("id"
                        ), jsonArgs.at("command_device"), deviceName + " does not belongs to the group", "command_device"));
                }
            }
            catch (const Tango::DevFailed &df)
            {
                return errorMessageToCharArray(StringProc::exceptionStringOut(jsonArgs.at("id"
                    ), jsonArgs.at("command_device"), deviceName + "  belongs to the group but can’t be reached", "command_device"));
            }

            return sendCommandBinForDevice(dp, argin, jsonArgs);
        }

        // Имя команды определено либо в jsonArgs["command_group"] либо 
        // в jsonArgs["command_device"] в зависимости адресата команды
        //
        // Проверка данных ключей производится в методах:
        //       WebSocketDS::send_command_bin
        //       WebSocketDS::send_command
        // вызовом метода StringProc::parseJsonFromCommand

        return errorMessageToCharArray(StringProc::exceptionStringOut(jsonArgs.at("id"
            ), jsonArgs.at("command_group"), "Not found object device_name or command_device in input JSON. And command with binary output is available only for single device from group. ", "command_group"));
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

    Tango::GroupCmdReplyList GroupForWs::tangoCommandInoutForGroup(Tango::DevString &argin, const std::map<string, string> &jsonArgs, string &errorMess)
    {
        Tango::GroupCmdReplyList deviceDataList;
        errorMess.clear();
        string commandName;
        string arginStr = jsonArgs.at("argin");
        string idStr = jsonArgs.at("id");

        if (jsonArgs.at("command_group") != NONE)
            commandName = jsonArgs.at("command_group");
        else {
            errorMess = StringProc::exceptionStringOut(idStr, commandName, "Not found key command_group in JSON", "command_group");
            return deviceDataList;
        }

        if (accessibleCommandInfo.find(commandName) != accessibleCommandInfo.end()) {
            Tango::CommandInfo comInfo = accessibleCommandInfo[commandName];
            int type = comInfo.in_type;
            // Вызов правильного метода  command_inout
            // Проверка типа входных аргументов Void, Array, Data

            try{
                if (type == Tango::DEV_VOID)
                    deviceDataList = group->command_inout(commandName, true);
                else {
                    if (arginStr == NONE) {
                        errorMess = StringProc::exceptionStringOut(idStr, commandName, "argin not found", "command_group");
                        return deviceDataList;
                    }

                    // если argin - массив
                    // и если требуемый type не является массивом
                    if (arginStr == "ARRAY" && !processor->isMassive(type)) {
                        errorMess = StringProc::exceptionStringOut(idStr, commandName, "The input data should not be an array", "command_group");
                        return deviceDataList;
                    }

                    Tango::DeviceData inDeviceData;
                    inDeviceData = processor->gettingDevDataFromJsonStr(argin, type);

                    deviceDataList = group->command_inout(commandName, inDeviceData, true);
                }
            }
            catch (Tango::DevFailed &e) {
                errorMess = StringProc::exceptionStringOut(idStr, commandName, "Exception from command_inout. Check the format of the data", "command_group");
            }
        }
        else {
            errorMess = StringProc::exceptionStringOut(idStr, commandName, "This Command not found in the list of available commands or not found on DeviceServer", "command_group");
        }

        return deviceDataList;
    }

    Tango::DevString GroupForWs::sendCommandToGroup(Tango::DevString &argin, std::map<string, string> &jsonArgs)
    {
        string errorMess;
        Tango::GroupCmdReplyList dataFromGroup =  tangoCommandInoutForGroup(argin,jsonArgs,errorMess);

        if (errorMess.size())
            return CORBA::string_dup(errorMess.c_str());

        // Преобразование полученных данных в Json-формат
        return CORBA::string_dup(processor->gettingJsonStrFromGroupCmdReplyList(dataFromGroup,jsonArgs).c_str());
    }

    Tango::DeviceData GroupForWs::tangoCommandInoutForDeviceFromGroup(Tango::DevString &argin, std::map<string, string> &jsonArgs, string &errorMess)
    {
        string commandName = jsonArgs.at("command_device");
        string deviceName = jsonArgs.at("device_name");
        string arginStr = jsonArgs.at("argin");
        string idStr = jsonArgs.at("id");

        errorMess.clear();

        Tango::DeviceData outDeviceData;
        Tango::DeviceProxy *dp;

        try
        {
            // Получение девайса по имени. Если данный девайс не входит в группу
            // или нет доступа к нему, высылается сообщение об ошибке
            dp = group->get_device(deviceName);
            if (dp == 0)
            {
                errorMess = StringProc::exceptionStringOut(idStr, commandName, deviceName + " does not belongs to the group" , "command_device");
                return outDeviceData;
            }
        }
        catch (const Tango::DevFailed &df)
        {
            errorMess = StringProc::exceptionStringOut(idStr, commandName, deviceName + "  belongs to the group but can’t be reached" , "command_device");
            return outDeviceData;
        }

        return tangoCommandInoutForDevice(dp,argin,commandName,arginStr,idStr,errorMess);
    }

    Tango::DevString GroupForWs::sendCommandToDevice(Tango::DevString &argin, std::map<string, string> &jsonArgs)
    {
        string errorMess;
        Tango::DeviceData outDeviceData = tangoCommandInoutForDeviceFromGroup(argin, jsonArgs, errorMess);

        if (errorMess.size())
            return CORBA::string_dup(errorMess.c_str());
        else
            // Преобразование полученных данных в Json-формат
            return CORBA::string_dup(processor->gettingJsonStrFromDevData(outDeviceData, jsonArgs, true).c_str());
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
            fromException(e, "getAttributeList ");
        }

        return devAttrList;
    }
}
