# История изменений
# 0.9.2 (02.11.2016)
 - Добавлена возможность выставления флагов std::fixed std::scientific для выходных значений команд типа с плавающей запятой
 - поправлен выводимый json-формат ошибок, атрибутов и результатов запуска команд
 - Добавлена возможность изменения точности (precision) для читаемых атрибутов типа с плавающей запятой. По умолчанию значение точности 5
 - Добавлена возможность выставления флагов std::fixed std::scientific для читаемых атрибутов типа с плавающей запятой
 - Перенесён под #ifdef TESTFAIL метод  sendLogToFile(), который запускался при принудительной перезагрузке девайс-сервера
 - Исправлено выставление state и status при инициализации сервера на ON или FAULT. До этого высавлялось UNKNOWN.
 - Добавлена реинициализация списка команд, если размер списка равен нулю, при имеющихся в property командах.
 - Добавлено свойство девайса MaxNumberOfConnections, задабщее максиальное число подключённых клиентов
 - Добавлено свойство девайса MaximumBufferSize, задающее максимальный размер буфера для каждого соединения в КиБ
 - Добавлено свойство девайса ResetTimestampDifference
 - Добавлены скалярные атрибуты TimestampDiff и NumberOfConnections
 - Исправлено закрытие соединения со стороны сервера, при исключениях.
 - Отключён запуск метода CheckPoll, при выставленном device state = OFF. До этого, после минуты нахождения в статусе OFF, происходила перезагрузка девайс сервера.

### 0.9.1 (05.10.2016)
 - Добавлена возможность использования метода USERANDIDENT для идентификации пользователя
 - Удалена проверка пользователя при чтении атрибутов

### 0.9.0 (08.09.2016)
 - Добавлена возможность выполнения методов на прослушиваемом танго девайс-сервере
 - Добавлена возможность использования защищённого wss соединения
 - Добавлена возможность записи в журналы при запуске методов на прослушиваемом сервере
 - Добавлена принудительная перезагрузка WS танго-сервера, если poll sleeping превышает 60 секунд
 - Добавлены перехваты исключений при обрыве соединения и ошибках отправления