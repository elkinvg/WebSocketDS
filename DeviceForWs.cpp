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
        json << "]";
        if (ds->pipeName.size()) {
            json << ", \"pipe\": ";
            try {
                Tango::DevicePipe devicePipe = device->read_pipe(ds->pipeName[0]);
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

        iterator++;
        if (attrList != nullptr)
            delete attrList;
        return json.str();
    }

    string DeviceForWs::generateJsonFromPipeComm(const std::map<string, string> &pipeConf)
    {
        // Вызов generateJsonFromPipeComm происходит из WSThread.cpp
        // Там происходит проверка ключей read_pipe_gr read_pipe_dev если ds->isGroup() == true
        // И read_pipe если  ds->isGroup() == false

        string pipeName = pipeConf.at("read_pipe");

        try {
            DevicePipe devicePipe = device->read_pipe(pipeName);

            std::stringstream json;
            generateJsonHeadForPipeComm(pipeConf,json,pipeName);
            json << processor->processPipe(devicePipe, TYPE_WS_REQ::PIPE_COMM);
            json << "}";
            return json.str();
        }
        catch (Tango::DevFailed &e) {
            string errM = "Pipe " + pipeName + " not found in " + device->name();
            return StringProc::exceptionStringOut(pipeConf.at("id"),pipeName,errM,"read_pipe");
        }
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
