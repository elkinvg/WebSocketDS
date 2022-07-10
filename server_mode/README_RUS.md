# WebSocketDS server mode

- **[перечень property для определения Tango устройства](#перечень-property-для-определения-tango-устройства)**

- **[периодическое чтение атрибутов с прослушиваемого TangoDevice](#периодическое-чтение-атрибутов-с-прослушиваемого-tangodevice)**

  - **[формат данных периодического чтения для единичного девайса](#формат-данных-периодического-чтения-для-единичного-девайса)**
  - **[формат данных периодического чтения для группы девайсов](#формат-данных-периодического-чтения-для-группы-девайсов)**

- **[данные с событий](#данные-с-событий)**

- **[чтение атрибутов с прослушиваемого TangoDevice по запросу](#чтение-атрибутов-с-прослушиваемого-tangodevice-по-запросу)**
  - **[формат выводимимых данных чтения атрибутов по запросу](#формат-выводимимых-данных-чтения-атрибутов-по-запросу)**

- **[запуск команд](#запуск-команд)**
  - **[формат выводимых данных по командному запросу](#формат-выводимых-данных-по-командному-запросу)**

- **[запись в атрибуты](#запись-в-атрибуты)**

- **[чтение с pipe](#чтение-с-pipe)**
  - **[чтение с pipe. ответ для группы](#чтение-с-pipe.-ответ-для-группы)**
  - **[чтение с pipe. ответ для девайса](#чтение-с-pipe.-ответ-для-девайса)**

## перечень property для определения Tango устройства

- **Port** — Прослушиваемый порт при соединении; (DevShort)

- **DeviceServer** - tango id используемого устройства. Пример: `test/psw/group`. В случае использования групп устройств, используется шаблон. Параметром шаблона может быть простое имя устройства или шаблон имени устройства (например, `domain_*/family/member_*`); (string)

- **Attributes** — список атрибутов устройства, которые вы хотите считывать, если требуется считывать все атрибуты добавьте `__all_attrs__` (в групповом режиме не действует) (array of string)

- **PipeName** - PipeName для pipe устройства, также список атрибутов с опциями для вывода.; (array of string)

- **AuthDS** — танго сервер, отвечающий за аутентификацию пользователя, при наличии исполняемых команд; (string)

- **Secure** — установите true, для использования защищённого wss соединения, иначе false; (bool)

- **Certificate** — полный путь к используемому сертификату (if Secure = true); (string)

- **Key** - полный путь к используемому файлу с Private key (if Secure = true); (string)

- **Options** - дополнительные опции. Перечень дополнительных опций для девайса. Перечень доступных опций [указан здесь](../README_RUS.md#список-возможных-значений-в-property-options); (array of string)

- **list_subscr_event_change** - Перечень атрибутов с подпиской на события `CHANGE_EVENT`. ; (array of string)

- **list_subscr_event_periodic** - Перечень атрибутов с подпиской на события `PERIODIC_EVENT`.; (array of string)

- **list_subscr_event_user** - Перечень атрибутов с подпиской на события `USER_EVENT`. ; (array of string)

- **list_subscr_event_archive** - Перечень атрибутов с подпиской на события `ARCHIVE_EVENT`. ; (array of string)

## периодическое чтение атрибутов с прослушиваемого TangoDevice

Для получения данных с необходимых атрибутов нужно определить свойства `DeviceServer` и список `Attributes` которые будут читаться.

Если требуется читать с группы устройств, в свойство `Options` нужно добавить ключ `group`

### формат данных периодического чтения для единичного девайса

```json
{
  "event": "read",
  "type_req": "attribute",
  "data": {
    "attribute_name": {
      "data": "data",
      "set": "if writable attribute"
    }
  },
  "pipe": {
    "attrName1": "Data in format dependent on type",
    "attrName2": ["Data in format", "dependent on type"]
  }
}
```

### формат данных периодического чтения для группы девайсов

```json
{
  "event": "read",
  "type_req": "group_attribute",
  "data": {
    "name/tango/device_from_group": {
      "attribute_name": {
        "data": "data",
        "set": "if writable attribute"
      }
    }
  },
  "pipe": {
    "name/tango/device_from_group": {
      "attrName": "Data in format dependent on type"
    }
  }
}
```

## данные с событий

Данные из подписанных событий приходят в следующем формате:

```json
{
  "event": "read",
  "type_req": "from_event",
  "event_type": "Event type",
  "timestamp": 1501324867,
  "attr": "attribute name",
  "data": "value or array",
  "set": "value or array if writable"
}
```

## чтение атрибутов с прослушиваемого TangoDevice по запросу

Для чтения атрибутов с прослушиваемого устройства следует отправить команду

```json
{
  "type_req": "read_attr",
  "id": "ID request",
  "attr_name": "name of attribute or array of names",
  "precision": "precf=10",
  "device_name": "name_of_device"
}
```

---

**Ключ `device_name` указывается, только если используется групповой режим, и требуется считать атрибуты с конкретного девайса**

---

---

**Ключ `precision` указывается, только если требуется форматировать выводимые данные (float double). Если несколько атрибутов, указывается либо один для всех, либо массив с отдельным форматом для каждого  ["precf=10","precs=3"] [Подробнее про precision опции](../README_RUS.md#precision-options)**

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
  "argin": "value or array if needed"
}
```

---

**Ключ `device_name` указывается, только если используется групповой режим, и требуется считать атрибуты с конкретного девайса**

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
  "dimX": "only for Spectrum type",
  "dimY": "only for Image type"
}
```

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
  "id": "Request id"
}
```

---

**Ключ `device_name` необходим, только если требуется данные с конкретного устройства при использовании группового режима.**

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
