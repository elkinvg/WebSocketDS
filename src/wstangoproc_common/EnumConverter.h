#ifndef ENUM_CONVERTER_H
#define ENUM_CONVERTER_H

#include "common.h"
#include "ErrorType.h"
#include <map>
#include <tango.h>

using std::map;

namespace WebSocketDS_ns
{
    class EnumConverter {
    public:
        static TYPE_WS_REQ typeWsReqFromString(const string& value) {
            static const map<string, TYPE_WS_REQ> typeWsReqMap = {
                {"command", TYPE_WS_REQ::COMMAND},
                {"command_device", TYPE_WS_REQ::COMMAND_DEV},
                {"command_group", TYPE_WS_REQ::COMMAND_GR},

                {"read_pipe", TYPE_WS_REQ::PIPE_COMM},
                {"read_pipe_dev", TYPE_WS_REQ::PIPE_COMM_DEV},
                {"read_pipe_gr", TYPE_WS_REQ::PIPE_COMM_GR},

                {"command_device_cl", TYPE_WS_REQ::COMMAND_DEV_CLIENT},

                {"attr_device_cl", TYPE_WS_REQ::ATTR_DEV_CLIENT},
                {"write_attr", TYPE_WS_REQ::ATTRIBUTE_WRITE},
                {"write_attr_dev", TYPE_WS_REQ::ATTRIBUTE_WRITE_DEV},
                {"write_attr_gr", TYPE_WS_REQ::ATTRIBUTE_WRITE_GR},
                {"write_attr_dev_cl", TYPE_WS_REQ::ATTR_DEV_CLIENT_WR},
                {"attr_group_cl", TYPE_WS_REQ::ATTR_GR_CLIENT},

                {"user_status", TYPE_WS_REQ::USER_CHECK_STATUS},
                {"change_user_smpl", TYPE_WS_REQ::CHANGE_USER},

                {"read_attr", TYPE_WS_REQ::ATTRIBUTE_READ},
                {"read_attr_dev", TYPE_WS_REQ::ATTRIBUTE_READ_DEV},
                {"read_attr_gr", TYPE_WS_REQ::ATTRIBUTE_READ_GR},

                {"eventreq_add_dev", TYPE_WS_REQ::EVENT_REQ_ADD_DEV},
                {"eventreq_rem_dev", TYPE_WS_REQ::EVENT_REQ_REM_DEV},
                {"eventreq_off", TYPE_WS_REQ::EVENT_REQ_OFF},
                {"eventreq_check_dev", TYPE_WS_REQ::EVENT_REQ_CHECK_DEV},

                {"", TYPE_WS_REQ::UNKNOWN}
            };

            auto it = typeWsReqMap.find(value);
            if (it == typeWsReqMap.end()) {
                return TYPE_WS_REQ::UNKNOWN;
            }
            return it->second;
        };

        static string errorTypeToString(const ERROR_TYPE& errorType) {
            static const map<ERROR_TYPE, string> errTypeMap = {
                {ERROR_TYPE::INIT_FAILED, "init_failed"},
                {ERROR_TYPE::AUTH_CHECK, "auth_check"},
                {ERROR_TYPE::AUTH_PERM, "auth_perm"},
                {ERROR_TYPE::AUTH_SERVER_ERR, "auth_server_err"},
                {ERROR_TYPE::IS_NOT_VALID, "is_not_valid"},
                {ERROR_TYPE::NOT_SUPP, "not_supp"},
                {ERROR_TYPE::NOT_SUPP_IN_CURR, "not_supp_in_curr"},
                {ERROR_TYPE::UNKNOWN_REQ_TYPE, "unknown_req_type"},
                {ERROR_TYPE::CHECK_REQUEST, "check_request"},
                {ERROR_TYPE::TANGO_EXCEPTION, "tango_exc"},
                {ERROR_TYPE::UNAVAILABLE_DEVS, "unavailable_devs"},
                {ERROR_TYPE::EVENT_ERR, "event_err"},
                {ERROR_TYPE::EXC_FROM_EVENT_DEV, "event_dev_err"},
                {ERROR_TYPE::CHECK_CODE, "check_code"},
                {ERROR_TYPE::SUBSCR_NOT_FOUND, "subscr_not_found"},
                {ERROR_TYPE::COMMUNICATION_FAILED, "commun_failed"},
                {ERROR_TYPE::CONNECTION_FAILED, "conn_failed"},
                {ERROR_TYPE::UNKNOWN_EXC, "unknown_exc"},
                {ERROR_TYPE::NOT_SUBSCR_YET, "not_subscr_yet"},
                {ERROR_TYPE::FROM_EVENT_SUBSCR, "from_event_sub"},
                {ERROR_TYPE::DEVICE_NOT_IN_GROUP, "device_not_in_group"}
            };

            auto it = errTypeMap.find(errorType);
            if (it == errTypeMap.end()) {
                return "check_code";
            }
            return it->second;
        }

        static string eventTypeToString(const Tango::EventType& eventType) {
            static const map<Tango::EventType, string> errTypeMap = {
                {Tango::EventType::ARCHIVE_EVENT, "archive"},
                {Tango::EventType::PERIODIC_EVENT, "periodic"},
                {Tango::EventType::USER_EVENT, "user"},
                {Tango::EventType::CHANGE_EVENT, "change"},
            };

            auto it = errTypeMap.find(eventType);
            return it->second;
        }
    };
}

#endif // ENUM_CONVERTER_H
