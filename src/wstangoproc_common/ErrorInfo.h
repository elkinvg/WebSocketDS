#ifndef ERROR_INFO_H
#define ERROR_INFO_H

#include <string>
#include <vector>
#include "ErrorType.h"
#ifdef CLIENT_MODE 
#include "ResponseFromEvent.h"
#endif
using std::string;
using std::vector;


namespace WebSocketDS_ns
{
    struct ErrorInfo
    {
        ERROR_TYPE typeofError; // TODO: RENAME error_type
        string typeofReq; // TODO: RENAME req_type
        string id;
        string device_name;
        string attr_name;
        string errorMessage;
        vector<string> errorMessages;
        string event_type;
#ifdef CLIENT_MODE 
        vector<ResponseFromEventReq> errorResponses;
#endif
    };
}
#endif
