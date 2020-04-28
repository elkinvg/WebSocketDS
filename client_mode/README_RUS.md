# WebSocketDS client mode

- **[перечень property для определения Tango устройства](#перечень-property-для-определения-tango-устройства)**

- **[работа с событиями](#работа-с-событиями)**
  - **[данные с событий](#данные-с-событий)**
  - **[подписка на события](#подписка-на-события)**
  - **[полная отписка от событий](#полная-отписка-от-событий)**
  - **[частичная отписка от событий](#частичная-отписка-от-событий)**
  - **[получение id подписки](#получение-id-подписки)**

- **[чтение атрибутов по запросу](#чтение-атрибутов-по-запросу)**
  - **[формат выводимимых данных чтения атрибутов по запросу](#формат-выводимимых-данных-чтения-атрибутов-по-запросу)**

- **[запуск команд](#запуск-команд)**
  - **[формат выводимых данных по командному запросу](#формат-выводимых-данных-по-командному-запросу)**

- **[запись в атрибуты](#запись-в-атрибуты)**

- **[чтение с pipe](#чтение-с-pipe)**
  - **[чтение с pipe. ответ для группы](#чтение-с-pipe.-ответ-для-группы)**
  - **[чтение с pipe. ответ для девайса](#чтение-с-pipe.-ответ-для-девайса)**

## перечень property для определения Tango устройства

- **Port** — Прослушиваемый порт при соединении; (DevShort)

- **AuthDS** — танго сервер, отвечающий за аутентификацию пользователя, при наличии исполняемых команд; (string)

- **Secure** — установите true, для использования защищённого wss соединения, иначе false; (bool)

- **Certificate** — полный путь к используемому сертификату (if Secure = true); (string)

- **Key** - полный путь к используемому файлу с Private key (if Secure = true); (string)

- **Options** - дополнительные опции. Перечень дополнительных опций для девайса. Перечень доступных опций [указан здесь](../README_RUS.md#список-возможных-значений-в-property-options); (array of string)

## работа с событиями

Возможна подписка на следующие типы событий: `CHANGE_EVENT`, `PERIODIC_EVENT`, `ARCHIVE_EVENT`, `USER_EVENT`

### данные с событий

```json
{
  "event": "read",
  "type_req": "from_event",
  "event_type": "Event type",
  "timestamp": 1501324867,
  "attr": "attribute name",
  "data": "value or array",
  "set": "value or array if writtable",
  "dimX": "only for Image or Spectrum type",
  "dimY": "only for Image type"
}
```

### подписка на события

Для подписки на события нужно отправить сообщение в данном формате:

```json
{
  "type_req": "eventreq_add_dev",
  "id": "id",
  "change": {
    "name_of_device": "atrribute_name or array of names",
    "name_of_other_device": "atrribute_name"
  },
  "periodic": {
    "name_of_device": "atrribute_name or array of names",
    "name_of_other_device": "atrribute_name"
  },
  "user": {
    "name_of_device": "atrribute_name or array of names",
    "name_of_other_device": "atrribute_name"
  },
  "archive": {
    "name_of_device": "atrribute_name or array of names",
    "name_of_other_device": "atrribute_name"
  }
}
```

Ответ:

```json
{
  "event": "read",
  "type_req": "eventreq_add_dev",
  "id_req": "Request id",
  "resp": [
    {
      "device": "sys/tg_test/1",
      "attribute": "double_scalar",
      "event_type": "periodic",
      "event_sub_id": 7
    },
    {
      "device": "sys/tg_test/1",
      "attribute": "float_scalar",
      "event_type": "periodic",
      "event_sub_id": 8
    }
  ],
  "errors": [
    {
      "device": "sys/tg_test/1",
      "event_type": "periodic",
      "attribute": "attribute",
      "data": ["attribute attribute not found"]
    }
  ]
}
```

### полная отписка от событий

Для отписки от всех событий, на которые подписаны, нужно отправить сообщение:

```json
{
  "type_req": "eventreq_off",
  "id": "id"
}
```

Ответ:

```json
{
  "event": "read",
  "type_req": "eventreq_off",
  "id_req": "Request id",
  "success": true
}
```

### частичная отписка от событий

Для отписки от отдельных событий, нужно отправить:

```json
{
  "type_req": "eventreq_rem_dev",
  "id": "Request id",
  "event_sub_id": "Event Subscriber ID"
}
```

### получение id подписки

Для получения id подписки, нужно отправить:

```json
{
  "type_req": "eventreq_check_dev",
  "id": "id",
  "device": "name/of/device",
  "attribute": "attribute name",
  "event_type": "Event type"
}
```

Ответ будет следующим:

```json
{
  "event": "read",
  "type_req": "eventreq_check_dev",
  "id_req": "Request id",
  "data": {
    "device": "name/of/device",
    "attribute": "attribute name",
    "event_type": "Event type",
    "event_sub_id": 1234
  }
}
```

## чтение атрибутов по запросу

Для чтения атрибутов с прослушиваемого устройства следует отправить команду

```json
{
  "type_req": "read_attr",
  "id": "ID request",
  "attr_name": "name of attribute or array of names",
  "precision": "precf=10",
  "device_name": "name_of_device",
  "group_request": true
}
```

---

**Ключ `group_request: true` указывается, только если запрос групповой. В `device_name` следует указать шаблон. Параметром шаблона может быть простое имя устройства или шаблон имени устройства (например, `domain_*/family/member_*`)**

---

---

**Ключ `precision` указывается, только если требуется форматировать выводимые данные (float double). Если несколько атрибутов, указывается либо один для всех, либо массив с отдельным форматом для каждого ["precf=10","precs=3"][подробнее про precision опции](../README_RUS.md#precision-options)**

---

### формат выводимимых данных чтения атрибутов по запросу

Формат данных для единичных и групповых запросов одинаковый.

```json
{
  "event": "read",
  "type_req": "read_attr",
  "id_req": "ID request",
  "device_name": "name_of_device",
  "data": {
    "attribute_name": {
      "data": "data",
      "set": "if writable attribute"
    }
  }
}
```

---

**Так как запросы асинхронные, данные для каждого девайса приходят отдельно**

---

## запуск команд

Формат команд:

```json
{
  "type_req": "command",
  "id": "ID request",
  "device_name": "name of device",
  "command_name": "name of command",
  "precision": "precf=10",
  "group_request": true
}
```

---

**Ключ `group_request: true` указывается, только если запрос групповой. В `device_name` следует указать шаблон. Параметром шаблона может быть простое имя устройства или шаблон имени устройства (например, `domain_*/family/member_*`)**

---

---

**Ключ `precision` указывается, только если требуется форматировать выводимые данные (float double). [Подробнее про precision опции](../README_RUS.md#precision-options)**

---

### формат выводимых данных по командному запросу

---

**Так как запросы асинхронные, данные для каждого девайса приходят отдельно**

---

```json
{
  "event": "read",
  "type_req": "command",
  "device_name": "name of device",
  "command_name": "name of command",
  "id_req": "ID request",
  "data": "data"
}
```

## запись в атрибуты

Формат команд для записи в атрибуты:

```json
{
  "type_req": "write_attr",
  "device_name": "Device name",
  "attr_name": "Attribute name",
  "id": "Request id",
  "argin": "value or [array]",
  "dimX": "only for Image type",
  "dimY": "only for Image type",
  "group_request": true
}
```

---

**Ключ `group_request: true` указывается, только если запрос групповой. В `device_name` следует указать шаблон. Параметром шаблона может быть простое имя устройства или шаблон имени устройства (например, `domain_*/family/member_*`)**

---

Здесь `"dimX"` и `"dimY"` используются только для массивов типа `Image`. Для `Spectrum` `"dimX"` выставляется автоматически, исходя из присланных данных.

Ответ, в успешном случае, будет таким:

```json
{
  "event": "read",
  "type_req": "write_attr",
  "device_name": "Device name",
  "attr_name": "Attribute name",
  "id_req": "Request id",
  "resp": "OK"
}
```

## чтение с pipe

Для чтения с pipe на сервер отправляется JSON сообщение в следующем формате:

```json
{
  "type_req": "read_pipe",
  "pipe_name": "PipeName",
  "precision": {
    "AttrName": "precopt",
    "AttrName2": "precopt"
  },
  "device_name": "name_of_device",
  "id": "Request id",
  "group_request": true
}
```

---

**Ключ `group_request: true` указывается, только если запрос групповой. В `device_name` следует указать шаблон. Параметром шаблона может быть простое имя устройства или шаблон имени устройства (например, `domain_*/family/member_*`)**

---

---

**Ключ `precision` указывается, только если требуется форматировать выводимые данные (float double). Здесь используется формат объекта, где ключ - это имя атрибута, значение - формат вывода. [Подробнее про precision опции](../README_RUS.md#precision-options)**

---

Формат ответа варьируется в зависимости от режима и типа команды

### чтение с pipe. ответ для группы

```json
{
  "event": "read",
  "type_req": "read_pipe",
  "id_req": "Request id",
  "data": {
    "name/tango/device_from_group": {
      "AttrName": "data",
      "AttrName2": ["data", "data"]
    },
    "name/tango/other_device_from_group": "Возможное сообщение об ошибке, либо [массив сообщений]"
  }
}
```

### чтение с pipe. ответ для девайса

```json
{
  "event": "read",
  "type_req": "read_pipe",
  "device_name": "name/tango/device",
  "id_req": "Request id",
  "data": {
    "AttrName": "data",
    "AttrName2": ["data", "data"]
  }
}
```
