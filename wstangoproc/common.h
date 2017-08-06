#ifndef COMMON_H
#define COMMON_H

#include <string>

namespace WebSocketDS_ns
{
    // тип запроса. 
    enum class TYPE_WS_REQ 
    { 
        ATTRIBUTE,
        ATTRIBUTE_WRITE,
        COMMAND, PIPE, PIPE_COMM, 
        RIDENT_REQ, RIDENT_ANS, RIDENT,
        COMMAND_DEV_CLIENT, // команда любому устройству command_device_cl
        ATTR_DEV_CLIENT,    // чтение атрибута любого устройства attr_device_cl
        UNKNOWN};
    // форматы для IOS
    enum class TYPE_IOS_OPT { PREC, PRECF, PRECS };
    // тип 
    enum class OUTPUT_DATA_TYPE { JSON, BINARY };

    enum class TYPE_OF_IDENT { SIMPLE, RANDIDENT, RANDIDENT2, RANDIDENT3 };
    enum class TYPE_OF_VAL { VALUE, ARRAY, OBJECT, NONE };


    enum class MODE
    {
        // только серверное управление выводом информации
        SERVER,
        
        // как с сервера, так и с клиента. Все сервера. 
        SERVNCLIENT_ALL_RO,     // ser_cli_all_ro   Режим ReadOnly. 
        SERVNCLIENT_ALL,        // ser_cli_all      Режим RW 

        // как с сервера, так и с клиента. Модули прописанные в alias.
        SERVNCLIENT_ALIAS_RO,   // ser_cli_ali_ro   Режим ReadOnly.
        SERVNCLIENT_ALIAS,      // ser_cli_ali      Режим RW

        // Только с клиента. Все сервера.
        CLIENT_ALL_RO,          // cli_all_ro       Режим ReadOnly.
        CLIENT_ALL,             // cli_all          Режим RW

        // Только с клиента. Модули прописанные в alias
        CLIENT_ALIAS_RO,        // cli_ali_ro       Режим ReadOnly.
        CLIENT_ALIAS            // cli_ali          Режим RW
    };

    const std::string NONE = "\"NONE\"";

}

#endif // COMMON_H
