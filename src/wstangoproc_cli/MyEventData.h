#ifndef MY_EVENT_DATA_H
#define MY_EVENT_DATA_H

#include <string>
#include <vector>
#include <tango.h>

using std::string;
using std::vector;

namespace WebSocketDS_ns
{
    struct MyEventData {
        bool err;
        string eventType;
        Tango::DevErrorList errors;
        long tv_sec;
        string attrName;
        string deviceName;
        Tango::DeviceAttribute attr_value;
    };
}

#endif
