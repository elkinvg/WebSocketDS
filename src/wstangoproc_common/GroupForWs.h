#ifndef GroupForWs_H
#define GroupForWs_H

#include "GroupOrDeviceForWs.h"
#include <exception>

namespace WebSocketDS_ns
{

    class GroupForWs : public GroupOrDeviceForWs, public Tango::Group
    {
    public:
        GroupForWs(string pattern);
        GroupForWs(string pattern, vector<string>& attributes, vector<string>& pipes);
        ~GroupForWs();

        std::vector<std::string> getDeviceList();
        std::vector<Tango::DeviceAttribute>* getDeviceAttributeList(const string& device_name_i);
        Tango::DevicePipe getDevicePipe(const string& device_name_i);

    private:
        vector<string> getListOfAllAttributes();

    private:
        std::vector<std::string> _deviceList;
    };
}

#endif
