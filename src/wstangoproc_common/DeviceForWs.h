#ifndef DEVICEFORWS_H
#define DEVICEFORWS_H

#include "GroupOrDeviceForWs.h"

namespace WebSocketDS_ns
{
    // Используется только в server_mode
    class DeviceForWs: public GroupOrDeviceForWs, public Tango::DeviceProxy
    {
    public:
        DeviceForWs(string deviceName);
        DeviceForWs(string deviceName, vector<string>& attributes, vector<string>& pipes);
        ~DeviceForWs();

        std::vector<Tango::DeviceAttribute>* getDeviceAttributeList();
        Tango::DevicePipe getDevicePipe();

    private:
        vector<string> getListOfAllAttributes();
    
    private:
        string _deviceName;
    };
}

#endif
