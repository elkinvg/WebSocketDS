#ifndef COMMON_H
#define COMMON_H

#include <string>

namespace WebSocketDS_ns
{
    // тип запроса. 
    enum class TYPE_WS_REQ { ATTRIBUTE, COMMAND, PIPE, PIPE_COMM,
                             RIDENT_REQ, RIDENT_ANS, RIDENT,
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
        
        // как с севрвера, так и с клиента. Все сервера. 
        SERVNCLIENT_ALL_RO,     // Режим ReadOnly.
        SERVNCLIENT_ALL,        // Режим RW

        // как с севрвера, так и с клиента. Модули прописанные в alias.
        SERVNCLIENT_ALIAS_RO,   // Режим ReadOnly.
        SERVNCLIENT_ALIAS,      // как с севрвера, так и с клиента. Модули прописанные в alias. режим RW

        // Только с клиента. Все сервера.
        CLIENT_ALL_RO,          //  Режим ReadOnly.
        CLIENT_ALL,             //  Режим RW

        // Только с клиента. Модули прописанные в alias
        CLIENT_ALIAS_RO,        //  Режим ReadOnly.
        CLIENT_ALIAS            //  Режим RW
    };

    const std::string NONE = "\"NONE\"";

}

#endif // COMMON_H
