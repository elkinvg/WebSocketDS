#include "DeviceForWs.h"
#include "StringProc.h"

#include "TangoProcessor.h"
#include "WebSocketDS.h"

namespace WebSocketDS_ns
{
    DeviceForWs::DeviceForWs(WebSocketDS *dev, string deviceName):
        GroupOrDeviceForWs(dev)
    {
        device = new Tango::DeviceProxy(deviceName);
        device->set_timeout_millis(3000);
    }

    DeviceForWs::~DeviceForWs() 
    {
        if (device != nullptr)
            delete device;
    }

    string DeviceForWs::generateJsonForUpdate() {
        device->ping();

        std::stringstream json;
        std::vector<Tango::DeviceAttribute> *attrList = nullptr; 

        json << "{\"event\": \"read\", \"type_req\": \"attribute\", \"data\":[";

        attrList = device->read_attributes(_attributes);
        generateAttrJson(json, attrList);
        json << "]}";

        iterator++;
        if (attrList != nullptr)
            delete attrList;
        return json.str();
    }

    Tango::DevString DeviceForWs::sendCommand(Tango::DevString &argin) {
        string errorMess;
        std::map<std::string, std::string> jsonArgs = StringProc::parseJsonFromCommand(argin, ds->isGroup());
        Tango::DeviceData outDeviceData = tangoCommandInout(argin, jsonArgs, errorMess);

        if (errorMess.size())
            return CORBA::string_dup(errorMess.c_str());
        else
            // Преобразование полученных данных в Json-формат
            return CORBA::string_dup(processor->gettingJsonStrFromDevData(outDeviceData, jsonArgs).c_str());
    }

    Tango::DevVarCharArray* DeviceForWs::sendCommandBin(Tango::DevString &argin)
    {
        std::map<std::string, std::string> jsonArgs = StringProc::parseJsonFromCommand(argin, ds->isGroup());
        return sendCommandBinForDevice(device, argin, jsonArgs);
    }

    Tango::CommandInfo DeviceForWs::getCommandInfo(const string &command_name)
    {
        return device->command_query(command_name);
    }

    Tango::DeviceData DeviceForWs::tangoCommandInout(Tango::DevString &argin, const std::map<std::string, std::string> &jsonArgs, string& errorMess)
    {
        string commandName = jsonArgs.at("command");
        string arginStr = jsonArgs.at("argin");
        string idStr = jsonArgs.at("id");

        errorMess.clear();

        return tangoCommandInoutForDevice(device,argin,commandName,arginStr,idStr,errorMess);
    }
}
