# WebSocketDS
### Установка.
Для успешной компиляции должны быть установлены библиотеки TANGO и компилятор с поддержкой c++11.
В директории есть Makefile-exmpl для gcc. Данный Makefile был генерирован POGO, и при стандартных сборках TANGO подходит в большинстве случаев. Для успешного запуска данного Makefile, должна присутствовать директория определённая в  `MAKE_ENV = /usr/local/share/pogo/preferences` (по умолчанию, при необходимости нужно поправить). В скриптах, содержащихся в директории, определены необходимые переменные окружения для сборки проекта.

Для возможности использования записи событий запуска команд в журналы, должен быть определён макрос `USELOG` (в gcc  `-DUSELOG`).  Подробнее об использовании смотрите ниже.

Для использования метода идентификации пользователя случайным выражением, должен быть определён макрос `USERANDIDENT` (в gcc  `-DUSERANDIDENT`).  Подробнее об использовании смотрите ниже.

### Перечень property для определения Tango устройства.
 - **Port** — Прослушиваемый порт при соединении; (DevShort)
 - **DeviceServer** - tango id используемого устройства. Пример: `test/psw/group` ; (string)
 - **Attributes** — список аттрибутов устройства, которые вы хотите считывать; (array of string)
 - **Commands** — список команд устройства, которые вы хотите выполнять через WS; (array of string)
 - **AuthDS** — танго сервер, отвечающий за аутентификацию пользователя, при наличии исполняемых команд; (string)
 - **Secure** — установите true, для использования защищённого wss соединения, иначе false; (bool)
 - **Certificate** — полный путь к используемому сертификату (if Secure = true); (string)
 - **Key** - полный путь к используемому файлу с  Private key (if Secure = true); (string)

### Простое использование только с чтением атрибутов.
Для использования  WS только для чтения атрибутов, следует определить  Port,  DeviceServer и  Attributes. При использовании защищённого wss соединения, нужно установить свойство Secure в true, а также определить  Certificate  и Key

Пример формата получаемых данных будет следующим:
```json
{
  "event": "read",
  "type_req":"attribute",
	"data": [
		{"attr": "имя_атрибута", "qual": "VALID", "time": 1475580424, "data": 128},
		{…}
	]
}
```
 * **"event"** - тип события, в данном случае чтение
 * **"type_req"** - тип полученных данных, в данном случае attribute. Данные с прослушиваемых атрибутов.
 * **"data"** - Получаемые данные. (Атрибуты)
   * **"attr"** — имя прослушиваемого атрибута.
   * **"qual"** — `Tango::AttrQuality` Возможные значения: `"VALID`", `"INVALID"`,`"ALARM"`, `"CHANGING"`, `"WARNING"`;
   * **"time"** — UNIX_TIMESTAMP
   * **"data"** — Данные. В зависимости от читаемых атрибутов (Scalar, Spectrum,  Image) меняется выводимый формат. Для Scalar — это единственное значение в виде строки, числа или булевого значения. Для  Spectrum — это массив вида [ … ], также определено значение "dimX" размерность спектра. Для  Image— это это массив вида [ … ], также определено значение "dimX" и "dimY" -  размерность Image.

В случае ошибок выводится сообщение вида
```json
{
  "event": "error",
  "data": [
            {
		        "error" : «Сообщение ошибки»,
                "type_req": "attribute",
	        }
  ]
}
```

В данном случае, type_req - говорит о том, что ошибка произошла при чтении атрибута.

### Изменение точности и форматирование выводимых значений атрибутов и команд типа с плавающей запятой

По умолчанию, для вывода атрибутов стоит setprecision(5). Для выставления других значений точности, к имени аттрибута в свойстве следует добавить `;prec=N`, где N - это требуемая точность. Пример: `AttrDevDouble;prec=10`

Следует учитывать максимально возможную точность. Подробнее про double можно посмотреть [здесь](https://ru.wikipedia.org/wiki/%D0%A7%D0%B8%D1%81%D0%BB%D0%BE_%D0%B4%D0%B2%D0%BE%D0%B9%D0%BD%D0%BE%D0%B9_%D1%82%D0%BE%D1%87%D0%BD%D0%BE%D1%81%D1%82%D0%B8), про float [здесь](https://ru.wikipedia.org/wiki/%D0%A7%D0%B8%D1%81%D0%BB%D0%BE_%D1%81_%D0%BF%D0%BB%D0%B0%D0%B2%D0%B0%D1%8E%D1%89%D0%B5%D0%B9_%D0%B7%D0%B0%D0%BF%D1%8F%D1%82%D0%BE%D0%B9).

Также возможно выставление дополнительных флагов форматирования:

- **precf** - выставление флага std::fixed
- **precs** - выставление флага std::scientific

Пример вывода для числа 1476379200 типа double с precision=10

- **;prec=10** - выводится 1476379200
- **;precf=10** - выводится 1476379200.0000000000
- **;precs=10** - выводится 1.4763792000e+009

Пример вывода для числа 1476379200 типа double с precision по умолчанию
- **;precf** - выводится 1476379200.000000
- **;precs** - выводится 1.476379e+009


Выставление точности для возвращаемых значений команд проходит аналогично.  Для выставления других значений точности, к имени команды в свойстве следует добавить `;prec=N`, либо другое, из перечисленного выше.

### Запуск команд
Для запуска команд через  WS, нужно определить список запускаемых команд в свойстве  Commands. Команды, перечисленные в свойстве, должны содержаться в прослушиваемом девайсе. Команды запускаются входящим json сообщением вида `{“command” : “nameOfCommand”, “argin” : [“1”,”2”,”3”]}`.
argin, в зависимости от метода, отправлять в виде массива, значения или не  указывать, если команда не принимает аргументов. Также можно указывать «id»: id, для идентификации запроса. В случае, если id не указан, возвращается «id»: «None».

Ответ в успешном случае будет таким:
```json
{
  "event": "read",
  "type_req": "command",
  "data": {
    "command_name": "имя команды",
    "id_req": "id запроса запуска команды",
    "argout": "Данные. Единственное значение
    или массив в зависимости возвращаемых данных. В случае
    массива [...]"
  }  
}
```

В случае ошибки таким:
```json
{
  "event": "error",
  "data": [
            {
		        "error" : "Сообщение ошибки",
                "command_name" : "Имя команды",
                "type_req": "command",
                "id_req": "id запроса запуска команды"
	        }
  ]
}
```

Также запуск команд возможен только для зарегистрированных в базе пользователей. Соответственно в скрипте должен быть прописан url такого типа: `ws(wss):адрес :порт?'login=логин&password=пароль.` Если логин и пароль не прописан, или не проходит проверку, выкидывается ошибка такого типа: `«WebSocket connection to 'ws://…… ' failed: Error during WebSocket handshake: Unexpected response code: 400»`
Проверка осуществляется через  TANGO сервер, прописанный в свойстве  AuthDS.
1. Этот сервер должен содержать метод `check_user(const Tango::DevVarStringArray (*argin)` и возвращать true или false
   * **\(\*argin)[0]** = «login»,
   * **\(\*argin)[1]** = «password». Обязательно должна быть учтена последовательность и размер.
2. Также сервер должен содержать метод `check_permissions(const Tango::DevVarStringArray (*argin)` , в который передаётся массив, содержащий перечисленные ниже данные, и возвращать true или false
   * **\(\*argin)[0]** — Танго девайс
   * **\(\*argin)[1]** — запускаемая команда
   * **\(\*argin)[2]** — Ip, с которого производится запуск
   * **\(\*argin)[3]** — login

Сама процедура проверки прав пользователей определяется в сервере, заданном в AuthDS. Должны лишь быть сохранены количество и последовательность вводимых параметров.



### Использование  USERANDIDENT для аутентификации.
Также возможна проверка пользователей методом USERANDIDENT (использования случайного выражения).
При авторизации происходит проверка не логина и пароля, а логина и хэша (md5, sha ... и так далее) из присланного пользователю случайного слова/числа плюс идентификатора пользователя, хранящегося у него,к примеру, в localStorage. `md5(rand+ident)`
Как это будет делаться на стороне клиента, дело разработчика. Главное, чтобы в метод check_permission, который вызывается при каждой команде, отправлялись дополнительно четыре значения. Они должны передаваться при открытии вэбсокета ... то есть, как я писал ранее, `ws(wss):адрес :порт?login=логин&id_ri=ххх&rand_ident=xxx&rand_ident_hash=xxx`

 * **login** - ну тут всё понятно
 * **id_ri** - идентификатор случайного числа (тут тоже вроде понятно ...)
 * **rand_ident** - случайное число/слово присланное с сервера
 * **rand_ident_hash** - хэш(rand_ident+клиентское число/слово)

Также в танговском-девайсе, через который проводится авторизация (AuthDS), должен быть определён метод check_permissions_ident, в котором будет проводиться проверка. Он возвращает true или false. Ну и конечно важно, чтоб сохранялась последовательность и количество аргументов. Сейчас последовательность такая

 - **permission_data[0]** = `deviceName`; // имя девайса на котором выполняется команда
 - **permission_data[1]** = `commandName`; // имя команды
 - **permission_data[2]** = `parsedGet["ip"]`; // айпишник пользователя
 - **permission_data[3]** = `parsedGet["login"]`; // логин пользователя
 - **permission_data[4]** = `parsedGet["id_ri"]`; // идентификатор случайного числа
 - **permission_data[5]** = `parsedGet["rand_ident_hash"]`; // случайное число/слово
 - **permission_data[6]** = `parsedGet["rand_ident"]`; // хэш

В общем представлении процесс выглядит так:

1. Пользователь открывает страницу. На сервер при подключении пользователя отправляется его логин, с запросом о получении случайного числа.
2. Сервер считывает из базы идентификатор пользователя. Генерирует случайное число, сохраняет его (к примеру тоже в БД) и создаёт md5(случайное число+идентификатор). Естественно после того, как клиент завершит сеанс, случайное число должно быть удалено, и при следующем подключении должно генерироваться другое. Пользователю отправляется это число, и id этого числа. В качестве id, в принципе, можно использовать timestamp (с миллисекундами).
3. Клиент запускает вэбсокет `ws(wss):адрес :порт?login=логин&id_ri=ххх&rand_ident=xxx&id_ri=xxx&rand_ident_hash=xxx`
4. На сервере проверяются все переданные, при открытии вэбсокета, параметры, и, если всё нормально, открывает вэб-сокет с возможностью отправления сообщений на сервер. Если что-то не так, то при попытке клиента что-то отправить он будет получать `{"error":"Permission denied"}`

Естественно, стоит предусмотреть случаи обрыва только вэбсокетов, потому-что тут, скорее всего, будет открываться новое соединение и выполняться тогда будет только с шага 3 со старыми параметрами.

### Использование записи в журнал при выполении команд
Для активации этой возможности нужно во-первых, определить define `#USELOG`, во-вторых, сервер, отвечающий за авторизацию, должен содержать метод `send_log_command_ex` который записывает в таблицу следующие данные:
 - **id** - autoincrement
 - **argin[0]** = timestamp_string UNIX_TIMESTAMP
 - **argin[1]** = login
 - **argin[2]** = deviceName
 - **argin[3]** = IP
 - **argin[4]** = commandName
 - **argin[5]** = commandJson
 - **argin[6]** = statusBool

Сама процедура записи определяется в (AuthDS). Должны лишь быть сохранены последовательность аргументов и имя метода. Возвращается bool.
