#ifndef DEVICEFORWS_H
#define DEVICEFORWS_H

#include "GroupOrDeviceForWs.h"

namespace WebSocketDS_ns
{
    class DeviceForWs: public GroupOrDeviceForWs, public Tango::DeviceProxy
    {
    public:
        DeviceForWs(string deviceName);
        // TODO: ONLY FOR SERVER MODE
        DeviceForWs(string deviceName, vector<string>& attributes, vector<string>& pipes);
        ~DeviceForWs();

        // TODO: ONLY FOR SERVER MODE
        std::vector<Tango::DeviceAttribute>* getDeviceAttributeList();
        // TODO: ONLY FOR SERVER MODE
        Tango::DevicePipe getDevicePipe();

    private:
        vector<string> getListOfAllAttributes();
    
    private:
        string _deviceName;
    };
}

#endif
