#include "DeviceForWs.h"
#include "StringProc.h"

#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    DeviceForWs::DeviceForWs(string deviceName)
    {
        device = new Tango::DeviceProxy(deviceName);
        device->set_timeout_millis(3000);
    }

    DeviceForWs::DeviceForWs(string deviceName, std::pair<vector<string>, vector<string> > &attr_pipes)
        :DeviceForWs(deviceName)
    {
        initAttr(attr_pipes.first);
        initPipe(attr_pipes.second);
    }

    DeviceForWs::DeviceForWs(string deviceName, array<vector<string>, 3> &attrCommPipe)
        :DeviceForWs(deviceName)
    {
        initAttr(attrCommPipe[0]);
        initComm(attrCommPipe[1]);
        initPipe(attrCommPipe[2]);
    }

    DeviceForWs::DeviceForWs(string deviceName, vector<string> &commands)
        :DeviceForWs(deviceName)
    {
        initComm(commands);
    }

    DeviceForWs::~DeviceForWs() 
    {
        if (device != nullptr)
            delete device;
    }

    string DeviceForWs::generateJsonForUpdate() {
        device->ping();

        std::stringstream json;

        json << "{\"event\": \"read\", \"type_req\": \"attribute\", \"data\":[";
        forGenerateJsonForUpdate(json);
        json << "}";

        iterator++;
        return json.str();
    }

    void DeviceForWs::generateJsonForUpdate(stringstream &json)
    {
        forGenerateJsonForUpdate(json);
        iterator++;
    }

    string DeviceForWs::sendPipeCommand(const ParsedInputJson& parsedInput)
    {
        string pipeName = parsedInput.otherInpStr.at("pipe_name");

        if (parsedInput.type_req != "read_pipe")
            return StringProc::exceptionStringOut(parsedInput.id, pipeName, "type_req must be read_pipe", parsedInput.type_req);

        try {
            DevicePipe devicePipe = device->read_pipe(pipeName);

            std::stringstream json;
            generateJsonHeadForPipeComm(parsedInput, json);
            json << processor->processPipe(devicePipe, TYPE_WS_REQ::PIPE_COMM);
            json << "}";
            return json.str();
        }
        catch (Tango::DevFailed &e) {
            string errM = "Pipe " + pipeName + " not found in " + device->name();
            return StringProc::exceptionStringOut(parsedInput.id, pipeName, errM, "read_pipe");
        }
    }

    string DeviceForWs::sendCommand(const ParsedInputJson& parsedInput, bool& statusComm) {
        statusComm = false;
        if (parsedInput.type_req != "command")
            return StringProc::exceptionStringOut(parsedInput.id, parsedInput.otherInpStr.at("command_name"),"type_req must be command","command");

        string errorMess;
        Tango::DeviceData outDeviceData = tangoCommandInoutForDevice(device, parsedInput, errorMess);

        if (errorMess.size())
            return errorMess;

        statusComm = true;
        return processor->getJsonStrFromDevData(outDeviceData, parsedInput);
    }

    string DeviceForWs::sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm)
    {
        return sendCommandBinForDevice(device, parsedInput, statusComm);
    }

    Tango::CommandInfo DeviceForWs::getCommandInfo(const string &command_name)
    {
        return device->command_query(command_name);
    }

    void DeviceForWs::forGenerateJsonForUpdate(stringstream &json)
    {
        std::vector<Tango::DeviceAttribute> *attrList = nullptr;

        attrList = device->read_attributes(_attributes);
        generateAttrJson(json, attrList);
        json << "]";

        if (_pipeAttr.size()) {
            json << ", \"pipe\": ";
            try {
                Tango::DevicePipe devicePipe = device->read_pipe(_pipeAttr);
                json << processor->processPipe(devicePipe, TYPE_WS_REQ::PIPE);
            }
            catch (Tango::DevFailed &e) {
                json << "[";
                for (unsigned int i = 0; i < e.errors.length(); i++) {
                    if (i > 0)
                        json << ", ";
                    json << "\"" << e.errors[i].desc << "\"";
                }
                json << "]";
            }
        }

        if (attrList != nullptr)
            delete attrList;
    }

    bool DeviceForWs::initAllAttrs() {
        auto attrList = device->attribute_list_query();
        for (auto& attrFromList : *attrList) {
            _attributes.push_back(attrFromList.name);
        }
        return true;
    }
}
