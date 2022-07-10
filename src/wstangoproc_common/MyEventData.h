#ifndef MY_EVENT_DATA_H
#define MY_EVENT_DATA_H

#include <string>
#include <vector>
#include <tango.h>

#include "MyAttributeData.h"

using std::string;
using std::vector;

namespace WebSocketDS_ns
{
    class MyEventData {
    public:
        MyEventData(Tango::EventData * dt);
        ~MyEventData();
    public:
        bool err;
        string eventType;
        Tango::DevErrorList errors;
        long tv_sec;
        string attrName;
        string deviceName;
        MyAttributeData *attrData = nullptr;
    };
}

#endif
