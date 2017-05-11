#ifndef TANGOCONNFORCLIENT_H
#define TANGOCONNFORCLIENT_H

#include <string>

#include "GroupOrDeviceForWs.h"

using std::string;

namespace WebSocketDS_ns
{
    class TangoConnForClient
    {
    public:
        TangoConnForClient(const json_arr_map& listDeviceWithAttr);
        TangoConnForClient(dev_attr_pipe_map& listDeviceWithAttrNPipes);

        pair<string, string> addDevicesToUpdateList(dev_attr_pipe_map& listDeviceWithAttrNPipes);

        vector<string> addAttrToDevicesFromUpdatelist(dev_attr_pipe_map& listDeviceWithAttrNPipes);
        vector<string> remAttrToDevicesFromUpdatelist(dev_attr_pipe_map& listDeviceWithAttrNPipes);

        string removeDevicesFromUpdateList(string dev);
        string removeDevicesFromUpdateList(vector<string> devs);
        
        ~TangoConnForClient();
        string getJsonForAttribute();
        bool removeAllDevices();
        int numOfListeningDevices();


    private:
        std::unordered_map<string, std::unique_ptr<GroupOrDeviceForWs>> devices;
    };

}

#endif // TANGOCONNFORCLIENT_H
