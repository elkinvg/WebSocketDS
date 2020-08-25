#ifndef TASK_INFO_H
#define TASK_INFO_H

#include <string>
#include <vector>
#include <unordered_map>
#include "common.h"
#include "CurrentMode.h"

using std::string;
using std::vector;
using std::unordered_map;

namespace WebSocketDS_ns
{
    struct TaskInfo {
        string idReq;
        string deviceName;
#ifdef CLIENT_MODE
        string groupPattern;
#endif
        string typeReqStr;

        string reqName;
        string precision;

        vector<string> reqNames; // if attributes
        unordered_map<string, string> precisions; // if attributes

        SINGLE_OR_GROUP singleOrGroup;
        TYPE_WS_REQ typeAsynqReq;
#ifdef CLIENT_MODE
        Tango::DeviceProxy* deviceForWs = nullptr;
#endif // CLIENT_MODE

    };
}
#endif