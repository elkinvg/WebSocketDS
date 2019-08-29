#ifndef TANGOCONNFORCLIENT_H
#define TANGOCONNFORCLIENT_H

#include <string>

//#include "GroupOrDeviceForWs.h"
#include "DeviceForWs.h"
using std::string;

namespace WebSocketDS_ns
{
    class TangoConnForClient
    {
    public:
		TangoConnForClient(const json_arr_map& listDeviceWithAttr, bool isObjData);
		TangoConnForClient(dev_attr_pipe_map& listDeviceWithAttrNPipes, bool isObjData);

        pair<string, string> addDevicesToUpdateList(dev_attr_pipe_map& listDeviceWithAttrNPipes);

        vector<string> addAttrToDevicesFromUpdatelist(dev_attr_pipe_map& listDeviceWithAttrNPipes);
        vector<string> remAttrToDevicesFromUpdatelist(dev_attr_pipe_map& listDeviceWithAttrNPipes);

        string removeDevicesFromUpdateList(string dev);
        string removeDevicesFromUpdateList(vector<string> devs);
        
        ~TangoConnForClient();
        bool removeAllDevices();
        int numOfListeningDevices();


    private:
        std::unordered_map<string, std::unique_ptr<DeviceForWs>> devices;
		bool _isObjData;
    };

}

#endif // TANGOCONNFORCLIENT_H
