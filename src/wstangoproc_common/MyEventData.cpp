#include "MyEventData.h"
#include <iostream>
using namespace std;

namespace WebSocketDS_ns {
    MyEventData::MyEventData(Tango::EventData* dt)
    {
        err = dt->err;
        eventType = dt->event;
        errors = dt->errors;
        tv_sec = dt->get_date().tv_sec;
        deviceName = dt->device->dev_name();
        attrName = dt->attr_name;

        if (dt->err) {
            return;
        }
        attrData = new MyAttributeData(dt->attr_value);
    }

    MyEventData::~MyEventData()
    {
        if (attrData != nullptr)
            delete attrData;
    }
}
