#include "GroupForWs.h"
#include "StringProc.h"

namespace WebSocketDS_ns
{
	GroupForWs::GroupForWs(string pattern):
        Group("forws")
    {
        add(pattern);
        auto hostAndDevice = StringProc::splitDeviceName(pattern);
        _deviceList = get_device_list(true);

        // Проверяется, удаляется ли танго-хост из имени девайса.
        // При чтении с удалённых танго-хостов, в deviceList записывается 
        //      только имена девайсов, без имени хоста.
        if (_deviceList.size() && hostAndDevice.first.size() && !StringProc::splitDeviceName(_deviceList[0]).first.size()) {
            remove_all();

            for (auto& dev : _deviceList) {
                add(hostAndDevice.first + dev);
            }

            _deviceList = get_device_list(true);
        }
    }

    GroupForWs::GroupForWs(string pattern, vector<string>& attributes, vector<string>& pipes)
        :GroupForWs(pattern)
    {
        initAttr(attributes);
        initPipe(pipes);
    }

    GroupForWs::~GroupForWs()
    {
    }

    std::vector<std::string> GroupForWs::getDeviceList()
    {
        return _deviceList;
    }

    std::vector<Tango::DeviceAttribute>* GroupForWs::getDeviceAttributeList(const string& device_name_i)
    {
        std::vector<Tango::DeviceAttribute>* devAttrList = nullptr;
        Tango::DeviceProxy *dp = get_device(device_name_i);
        
        //if (dp != 0)
        // TODO: CHECK if has not
        devAttrList = dp->read_attributes(_attributes);

        return devAttrList;
    }

    Tango::DevicePipe GroupForWs::getDevicePipe(const string & device_name_i)
    {
        Tango::DevicePipe devicePipe;
        Tango::DeviceProxy *dp = get_device(device_name_i);
        //if (dp != 0)
        // TODO: CHECK if has not
        devicePipe = dp->read_pipe(_pipeName);

        return devicePipe;
    }

    /*
    Берётся список атрибутов первого из группы девайса.
    Поэтому использовать опцию стоит, только если девайсы однородные (одинаковый список атрибутов)
    */
    vector<string> GroupForWs::getListOfAllAttributes()
    {
        vector<string> _attrs;

        try {
            auto dl = get_device_list();

            if (!dl.size()) {
                return _attrs;
            }

            string devNameFromGroup = dl[0];

            auto attrList = get_device(devNameFromGroup)->attribute_list_query();
            for (const auto& attrFromList : *attrList) {
                _attrs.push_back(attrFromList.name);
            }
        }
        catch (...) {
        }
        return _attrs;
    }

}
