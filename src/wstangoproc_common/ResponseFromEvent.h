#ifndef RESPONSE_FROM_EVENT_H
#define RESPONSE_FROM_EVENT_H

#include "tango.h"
#include <vector>
#include <string>

using std::string;
using std::vector;

namespace WebSocketDS_ns
{
    enum class RESPONSE_TYPE
    {
        ERROR_M,
        SUCCESS,
        ADDED_EARLY
    };

    struct ResponseFromEventReq {
        RESPONSE_TYPE respType;
        Tango::EventType eventType;
        string eventTypeStr;
        string deviceName;
        string attrName;
        vector<string> errorMessages;
        int eventSubId;
    };
}

#endif // RESPONSE_FROM_EVENT_H