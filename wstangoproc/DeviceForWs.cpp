#include "DeviceForWs.h"
#include "StringProc.h"

#include "TangoProcessor.h"

namespace WebSocketDS_ns
{
    DeviceForWs::DeviceForWs(string deviceName)
    {
        StringProc::isNameAlias(deviceName);
        getDeviceNameFromAlias(deviceName);
        _deviceName = deviceName;

        device = new Tango::DeviceProxy(deviceName);
        // ??? !!!
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

    DeviceForWs::DeviceForWs(string deviceName, const string& commandOrAttrName, TYPE_WS_REQ type_req)
        :DeviceForWs(deviceName)
    {
        vector<string> inpVec = { commandOrAttrName + ";wrt" };
        if (type_req == TYPE_WS_REQ::COMMAND_DEV_CLIENT)
            initComm(inpVec);
        if (type_req == TYPE_WS_REQ::ATTR_DEV_CLIENT_WR)
            initAttr(inpVec);
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

    void DeviceForWs::generateJsonForAttrReadCl(const ParsedInputJson& parsedInput, std::stringstream& json)
    {
        json << "{\"event\": \"read\", \"type_req\": \"" << parsedInput.type_req << "\", ";
        if (parsedInput.check_key("device_name") == TYPE_OF_VAL::VALUE)
            json << "\"device_name\": \"" << parsedInput.otherInpStr.at("device_name") << "\", ";
        
        try {
            auto idTmp = stoi(parsedInput.id);
            json << "\"id_req\": " << idTmp << ", ";
        }
        catch (...) {
            // id_req может быть числом, либо случайной строкой
            if (parsedInput.id == NONE)
                json << "\"id_req\": " << parsedInput.id << ", ";
            else
                json << "\"id_req\": \"" << parsedInput.id << "\", ";
        }

        json << "\"data\": {";
        json << "\"attrs\": [";

        forGenerateJsonForUpdate(json);
        json << "}";
        json << "}";
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

        string errorMess;
        Tango::DeviceData outDeviceData;
        if (parsedInput.type_req == "command")
            outDeviceData = tangoCommandInoutForDevice(device, parsedInput, errorMess);
        else if (parsedInput.type_req == "command_device_cl")
            outDeviceData = tangoCommandInoutForDeviceCl(device, parsedInput, errorMess);

        if (errorMess.size())
            return errorMess;

        statusComm = true;
        return processor->getJsonStrFromDevData(outDeviceData, parsedInput);
    }

    string DeviceForWs::sendCommandBin(const ParsedInputJson& parsedInput, bool& statusComm)
    {
        return sendCommandBinForDevice(device, parsedInput, statusComm);
    }

    string DeviceForWs::sendAttrWr(const ParsedInputJson& parsedInput, bool& statusComm)
    {
        statusComm = false;
        try {
            
            string attr_name = parsedInput.otherInpStr.at("attr_name");

            if (isWrtAttribute.find(attr_name) == isWrtAttribute.end())
                return StringProc::exceptionStringOut(parsedInput.id, NONE, "Attribute " + attr_name + " is not included in the list of writable attributes, Or it is not writable. Read README.md for information", parsedInput.type_req);

            Tango::AttributeInfoEx attr_info =  device->attribute_query(attr_name);
            vector<string> errors;
            Tango::DeviceAttribute attr = processor->getDeviceAttributeDataFromJson(parsedInput, attr_info, errors);

            if (errors.size())
                return StringProc::exceptionStringOut(parsedInput.id, NONE, errors, parsedInput.type_req);
            
            device->write_attribute(attr);
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

    vector<string> DeviceForWs::getListOfDevicesNames() 
    {
        return vector<string>({_deviceName});
    }

    Tango::CommandInfo DeviceForWs::getCommandInfo(const string &command_name)
    {
        return device->command_query(command_name);
    }

    bool DeviceForWs::checkIsAttributeWriteble(const string& attr_name)
    {
        try {
            Tango::AttributeInfoEx attr_info = device->get_attribute_config(attr_name);
        
            // ??? !!! if Tango::AttrWriteType::READ_WITH_WRITE
        
            if (attr_info.writable == Tango::AttrWriteType::READ_WRITE ||
                attr_info.writable == Tango::AttrWriteType::WRITE)
                return true;
        }
        catch (...){}
        
        return false;
    }

    bool DeviceForWs::pingDevice(string& errorMess)
    {
        errorMess.clear();
        try {
            device->ping();
            return true;
        }
        catch (Tango::DevFailed &e) {
            stringstream json;
            json << "[";
            for (unsigned int i = 0; i < e.errors.length(); i++) {
                if (i > 0)
                    json << ", ";
                json << "\"" << e.errors[i].desc << "\"";
            }
            json << "]";
            errorMess = json.str();
            return false;
        }
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

    void DeviceForWs::getDeviceNameFromAlias(string& alias) {
        string device_name_from_alias;
        try {
            Tango::Database *db = Tango::Util::instance()->get_database();
            db->get_device_alias(alias, device_name_from_alias);
            alias = device_name_from_alias;
        }
        catch (Tango::DevFailed& e) {}
    }
}