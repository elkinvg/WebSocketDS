# WebSocketDS server mode

Translation is not finished yet

If you find errors, write to me

Перевод ещё не закончен Русская версия [здесь](./README_RUS.md)

## Property list for defining Tango device

- **Port** — Listening port at connection; (DevShort)

- **DeviceServer** - tango id of the used device. As template parameter may serve simple name of a device or name pattern for group (e.g., `domain_*/family/member_*`).; (string)

- **Attributes** — a list of device attributes you want to read, if reading all attributes is required, add `__all_attrs__` (not operational in group mode).; (array of string)

- **PipeName** - Name of DevicePipe for reading.; If you want to set properties for specific attributes, add them in the format `NameAttr;property` (array of string)

- **AuthDS** — Tango web authentication device server name.; (string)

- **Secure** — Shall we use SSL encryption?
  Set true, for secure wss connection, otherwise false; (bool)

- **Certificate** — Full path to the certificate in use (if Secure == true); (string)

- **Key** - Full path to the file used with Private key (if Secure == true); (string)

- **Options** - additional options. A list of additional options for the device. The list of available options is [listed here.](../README.md#list-of-possible-values-in-property-options); (array of string)

- **list_subscr_event_change** - List of subscriptions to change events. ; (array of string)

- **list_subscr_event_periodic** - List of subscriptions to periodic events.; (array of string)

- **list_subscr_event_user** - List of subscriptions to user events.; (array of string)

- **list_subscr_event_archive** - List of subscriptions to archive events.; (array of string)

## periodically reading attributes from a listening TangoDevice

The listening device must be written in the property `DeviceServer`

The list of readable attributes must be listed in the property `Attributes`

If you want to listen to a group of devices, the "Options" property must contain a string `group`. `DeviceServer` must contain name pattern for group (e.g., domain*\*/family/member*\*)

### Data from device for periodic reading

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

### Data from group for periodic reading

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

## data from events

Event data comes in the following format:

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

## Reading data from attributes

Input message:

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

**The key `device_name` is specified only if group mode is used, and you need to read the attributes from a specific device**

---

TODO: translate

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
  "precision": "precf=10"
}
```

---

**The key `device_name` is specified only if group mode is used, and you need to read the attributes from a specific device**

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
  "dimY": "only for Image type"
}
```

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
  "id": "Request id"
}
```

---

**The key `device_name` is specified only if group mode is used, and you need to read the attributes from a specific device**

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
    "name/tango/other_device_from_group": "Возможное сообщение об ошибке, либо [массив сообщений]"
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
