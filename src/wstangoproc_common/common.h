#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/property_tree/json_parser.hpp>

using std::string;
using std::vector;
using std::unordered_map;

namespace WebSocketDS_ns
{
    typedef boost::property_tree::ptree ptree;
    typedef unordered_map<string, vector<string>> json_arr_map;
    typedef unordered_map<string, string> json_val_map;
    typedef unordered_map<string, ptree> json_obj_map;
    //typedef unordered_map< string, pair<vector<string>, vector<string>>> dev_attr_pipe_map; // TODO: DELETE OR IN CLIENT MODE
    // TODO: OR DELETE OR array
    // тип запроса. 
    enum class TYPE_WS_REQ 
    {
        ATTRIBUTE,
        ATTRIBUTE_WRITE,
        ATTRIBUTE_WRITE_DEV,
        ATTRIBUTE_WRITE_GR,

        COMMAND,
        COMMAND_DEV,
        COMMAND_GR,

        PIPE, 
        PIPE_COMM,
        PIPE_COMM_DEV,
        PIPE_COMM_GR,

        COMMAND_DEV_CLIENT, // команда любому устройству command_device_cl

        ATTR_DEV_CLIENT,    // чтение атрибута любого устройства attr_device_cl
        ATTR_DEV_CLIENT_WR,    // write атрибута любого устройства write_attr_dev_cl
        ATTR_GR_CLIENT,

        USER_CHECK_STATUS,

        CHANGE_USER,

        ATTRIBUTE_READ, // чтение атрибута в серверном режиме с девайса
        ATTRIBUTE_READ_DEV, // чтение атрибута в серверном режиме с девайса
        ATTRIBUTE_READ_GR, // чтение атрибута в серверном режиме с девайса

        EVENT_REQ_ADD_DEV, // добавить подписку на события для девайса
        EVENT_REQ_REM_DEV, // удалить конкретную подписку на события для девайса
        EVENT_REQ_OFF, // полностью отписаться от событий
        EVENT_REQ_CHECK_DEV, // проверить подписку, получить id

        UNKNOWN};
    // форматы для IOS
    enum class TYPE_IOS_OPT { PREC, PRECF, PRECS };
    // тип 
    // OLD enum class OUTPUT_DATA_TYPE { JSON, BINARY };

    enum class TYPE_OF_IDENT { SIMPLE, PERMISSION_WWW };
    enum class TYPE_OF_VAL { VALUE, ARRAY, OBJECT, NONE };

    enum class SINGLE_OR_GROUP { SINGLE, GROUP, SINGLE_FROM_GROUP};

    enum class TYPE_OF_DEVICE_DATA { VOID_D = 0, DATA = 1, ARRAY = 2, NOTSUPPORTED = 3 };

    const std::string NONE = "null";

}

#endif // COMMON_H
