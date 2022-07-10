# WebSocketDS client mode

Translation is not finished yet

If you find errors, write to me

Перевод ещё не закончен Русская версия [здесь](./README_RUS.md)

## Property list for defining Tango device

- **Port** — Listening port at connection; (DevShort)

- **AuthDS** — Tango web authentication device server name.; (string)

- **Secure** — Shall we use SSL encryption?
  Set true, for secure wss connection, otherwise false; (bool)

- **Certificate** — Full path to the certificate in use (if Secure == true); (string)

- **Key** - Full path to the file used with Private key (if Secure == true); (string)

- **Options** - additional options. A list of additional options for the device. The list of available options is [listed here.](../README.md#list-of-possible-values-in-property-options); (array of string)

## Work with events

The following event types are currently supported: `CHANGE_EVENT`, `PERIODIC_EVENT`, `ARCHIVE_EVENT`, `USER_EVENT`

Event data comes in the following format:

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

### Subscribing to events

To subscribe to events, you need to send a message in this format

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

The answer in case of success:

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

Here `"event_sub_id"` is the event subscription id. It can be used to further unsubscribe from the event.

### Full unsubscription from events

To unsubscribe from all events subscribed to, you need to send a message:

```json
{
  "type_req": "eventreq_off",
  "id": "id"
}
```

Answer:

```json
{
  "event": "read",
  "type_req": "eventreq_off",
  "id_req": "Request id",
  "success": true
}
```

### Partial unsubscription from events

To unsubscribe from individual events, you need to send:

```json
{
  "type_req": "eventreq_rem_dev",
  "id": "Request id",
  "event_sub_id": "Event Subscriber ID"
}
```

### Get the subscriber id

If you want to get the subscriber id, send:

```json
{
  "type_req": "eventreq_check_dev",
  "id": "id",
  "device": "name/of/device",
  "attribute": "attribute name",
  "event_type": "Event type"
}
```

The answer in case of success:

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

## Reading data from attributes

Input message:

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

**The `group_request: true` key is specified only if the request is group. The `device_name` should contain a pattern. The pattern parameter can be a simple device name or a device name pattern (for example, `domain_*/family/member_*`)**

---

---

**The `precision` key is specified only if you want to format the output data (float double). If there are several attributes, either one is indicated for all, or an array with a separate format for each ["precf=10","precs=3"] [For more detail about precision options read here](../README.md#precision-options)**

---

Output message:

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

## running of commands

Input message:

```json
{
  "type_req": "command",
  "id": "ID request",
  "device_name": "name of device",
  "command_name": "name of command",
  "precision": "precf=10",
  "group_request": true,
  "argin": "value or array if needed"
}
```

---

**The `group_request: true` key is specified only if the request is group. The `device_name` should contain a pattern. The pattern parameter can be a simple device name or a device name pattern (for example, `domain_*/family/member_*`)**

---

---

**The `precision` key is specified only if you want to format the output data (float double). [For more detail about precision options read here](../README.md#precision-options)**

---

Output message:

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

## Write attribute

Input JSON:

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

**The `group_request: true` key is specified only if the request is group. The `device_name` should contain a pattern. The pattern parameter can be a simple device name or a device name pattern (for example, `domain_*/family/member_*`)**

---

`" dimX "` and `" dimY "` are only used for arrays of type `Image`. For `Spectrum` `"dimX"` is set automatically based on the sent data.

Output message (if succesfull):

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

## Reading data from pipe

Input message:

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

**The `group_request: true` key is specified only if the request is group. The `device_name` should contain a pattern. The pattern parameter can be a simple device name or a device name pattern (for example, `domain_*/family/member_*`)**

---

---

**The `precision` key is specified only if you want to format the output data (float double). It uses the object format, where key is the name of the attribute, value is the output format. [For more detail about precision options read here](../README.md#precision-options)**

---

Output message for group:

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
    "name/tango/other_device_from_group": "Possible error message, or [array of messages]"
  }
}
```

Output message for device:

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
