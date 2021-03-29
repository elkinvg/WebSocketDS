#ifndef ERROR_TYPE_H
#define ERROR_TYPE_H

namespace WebSocketDS_ns
{
    enum class ERROR_TYPE {
        INIT_FAILED,            // Ошибка инициализации прослушиваемого девайса
        AUTH_CHECK,             // Ошибка аутентификации. Неверный логин или пароль TODO: \|/ Объединить?
        AUTH_PERM,              // Ошибка аутентификации. Нет доступа
        AUTH_SERVER_ERR,        // Ошибка аутентификации. Сервер недоступен
        IS_NOT_VALID,           // Неправильный запрос
        NOT_SUPP,               // Нет поддержки
        NOT_SUPP_IN_CURR,       // Нет поддержки в текущем режиме
        UNKNOWN_REQ_TYPE,       // Неизвестный типа запроса
        CHECK_REQUEST,          // Проверить запрос. Неправильный формат, или не найден необходимый ключ
        TANGO_EXCEPTION,        // TANGO exception
        UNAVAILABLE_DEVS,       // All device unavailable
        EVENT_ERR,              // Исключение из событийных данных
        EXC_FROM_EVENT_DEV,     // Исключение из прослушиваемого устройства
        CHECK_CODE,             // Данный тип ошибки не должен выдаваться при нормальной работе
        SUBSCR_NOT_FOUND,       // Не найден подписчик с данным id
        COMMUNICATION_FAILED,   // Tango::CommunicationFailed
        CONNECTION_FAILED,      // Tango::ConnectionFailed
        UNKNOWN_EXC,            // Исключение неизвестного типа
        NOT_SUBSCR_YET,         // Ещё нет подписчиков
        FROM_EVENT_SUBSCR,      // Ошибка возникшая при подписке на события
        DEVICE_NOT_IN_GROUP     // Этого девайса нет в группе
    };
}

#endif
