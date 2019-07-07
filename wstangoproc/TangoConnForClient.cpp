#include "TangoConnForClient.h"

#include <sstream>

#include "DeviceForWs.h"
#include "StringProc.h"

// for debug
//int itt = 0;

WebSocketDS_ns::TangoConnForClient::TangoConnForClient(const json_arr_map& listDeviceWithAttr, bool isObjData)
{
    _isObjData = isObjData;
    for (const auto& deviceAndAttrList : listDeviceWithAttr) {
        try {
            devices.insert(make_pair(deviceAndAttrList.first, unique_ptr<DeviceForWs>(new DeviceForWs(deviceAndAttrList.first, _isObjData))));
            //devices.at(deviceAndAttrList.first)->initAttrCommPipe()
        }
        catch (...) {}
    }
}

WebSocketDS_ns::TangoConnForClient::TangoConnForClient(dev_attr_pipe_map& listDeviceWithAttrNPipes, bool isObjData)
{
    _isObjData = isObjData;
    addDevicesToUpdateList(listDeviceWithAttrNPipes);
}

pair<string, string> WebSocketDS_ns::TangoConnForClient::addDevicesToUpdateList(dev_attr_pipe_map& listDeviceWithAttrNPipes)
{
    pair<string, string> errMessages;
    // [0] - Errors if devices already added
    // [1] - Messages from DevFailed
    
    string devmess;
    string tangoErrors;

    // Для проверки имеющихся девайсов
    if (devices.size() != 0) {
        vector<string> tmpdevs;
        for (auto& deviceAndAttrList : listDeviceWithAttrNPipes) {
            if (devices.find(deviceAndAttrList.first) != devices.end()) {
                tmpdevs.push_back(deviceAndAttrList.first);
            }
        }
        if (tmpdevs.size()) {
            devmess = "The listed devices ( ";
            for (auto& devnmame : tmpdevs) {
                devmess += devnmame + " ";
                listDeviceWithAttrNPipes.erase(devnmame);
            }
            devmess += ") are already in the list";
        }
    }


    for (auto& deviceAndAttrList : listDeviceWithAttrNPipes) {
        try {
            devices.insert(make_pair(deviceAndAttrList.first, unique_ptr<DeviceForWs>(new DeviceForWs(deviceAndAttrList.first, deviceAndAttrList.second, _isObjData))));
        }
        catch (Tango::DevFailed &e) {
            for (unsigned int i = 0; i < e.errors.length(); i++) {
                if (i > 0)
                    tangoErrors += " ||| ";
                tangoErrors += (string)e.errors[i].desc;
            }
        }
    }
    if (devmess.size())
        errMessages.first = devmess;
    if (tangoErrors.size())
        errMessages.second = tangoErrors;

    return errMessages;
}

vector<string> WebSocketDS_ns::TangoConnForClient::addAttrToDevicesFromUpdatelist(dev_attr_pipe_map &listDeviceWithAttrNPipes)
{
    // Добавление списка атрибутов в уже имеющиеся девайсы из списка для обновлений
    vector<string> messages;
    for (auto& deviceAndAttrList : listDeviceWithAttrNPipes) {
        if (devices.find(deviceAndAttrList.first) != devices.end()) {
            // devices is std::unordered_map<string, std::unique_ptr<GroupOrDeviceForWs>>
            // insertAttrToList is method of GroupOrDeviceForWs for adding attributes to list
            // This method returns a message
            messages.push_back(devices.at(deviceAndAttrList.first)->insertAttrToList(deviceAndAttrList.second.first));
        }
        else
            messages.push_back("Device " + deviceAndAttrList.first + " was not found in the update list");
    }
    return messages;
}

vector<string> WebSocketDS_ns::TangoConnForClient::remAttrToDevicesFromUpdatelist(dev_attr_pipe_map &listDeviceWithAttrNPipes)
{
    // Удаления списка атрибутов из уже имеющихся девайсов из списка для обновлений
    vector<string> messages;
    for (auto& deviceAndAttrList : listDeviceWithAttrNPipes) {
        if (devices.find(deviceAndAttrList.first) != devices.end()) {
            // devices is std::unordered_map<string, std::unique_ptr<GroupOrDeviceForWs>>
            // eraseAttrFromList is method of GroupOrDeviceForWs for removing attributes from list
            // This method returns a message
            // deviceAndAttrList second - pair<vector<string>, vector<string>> attr&pipe
            // deviceAndAttrList first - string - device_name
            string inpPipeName = "";
            if (deviceAndAttrList.second.second.size())
                inpPipeName = deviceAndAttrList.second.second.at(0);

            auto messfromerasing = devices.at(deviceAndAttrList.first)->eraseAttrFromList(deviceAndAttrList.second.first, inpPipeName, deviceAndAttrList.first);
            messages.insert(messages.begin(), messfromerasing.begin(), messfromerasing.end());
        }
        else {
            messages.push_back(deviceAndAttrList.first);
            messages.push_back("Device was not found in the update list");
        }
            
    }
    return messages;
}

string WebSocketDS_ns::TangoConnForClient::removeDevicesFromUpdateList(string dev)
{
    if (devices.find(dev) == devices.end())
        return "Device " + dev + " is not included in the list of updates.";

    devices.erase(dev);
    return "";
}

string WebSocketDS_ns::TangoConnForClient::removeDevicesFromUpdateList(vector<string> devs)
{
    string mess;
    string frontMess = "Devices ";
    for (auto& dev : devs) {
        string out = removeDevicesFromUpdateList(dev);
        if (out.size() != 0) {
            mess += (dev + " ");
        }
    }
    if (mess.size())
        mess = frontMess + mess + " is not included in the list of updates.";

    return mess;
}

WebSocketDS_ns::TangoConnForClient::~TangoConnForClient()
{
}

bool WebSocketDS_ns::TangoConnForClient::removeAllDevices()
{
    devices.clear();
    return true;
}

int WebSocketDS_ns::TangoConnForClient::numOfListeningDevices()
{
    return devices.size();
}
