#include "DeviceForWs.h"
#include "StringProc.h"
#include "ParsingInputJson.h"

namespace WebSocketDS_ns
{
    DeviceForWs::DeviceForWs(string deviceName)
        :DeviceProxy(deviceName), _deviceName(deviceName)
    {}

    DeviceForWs::DeviceForWs(string deviceName, vector<string>& attributes, vector<string>& pipes)
        :DeviceForWs(deviceName)
    {
        initAttr(attributes);
        initPipe(pipes);
    }

    DeviceForWs::~DeviceForWs() 
    {
    }

    std::vector<Tango::DeviceAttribute>* DeviceForWs::getDeviceAttributeList()
    {
        return read_attributes(_attributes);
    }

    Tango::DevicePipe DeviceForWs::getDevicePipe()
    {
        return read_pipe(_pipeName);
    }

    vector<string> DeviceForWs::getListOfAllAttributes()
    {
        vector<string> _attrs;
        try {
            auto attrList = attribute_list_query();
            for (auto& attrFromList : *attrList) {
                _attrs.push_back(attrFromList.name);
            }
        }
        catch (...) {
        }
        return _attrs;
    }
}