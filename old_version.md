## Reading data from attributes

data single device:

```json
{
  "event": "read",
  "type_req": "read_attr",
  "id_req": 6,
  "data": {
    "double_scalar": {
      "data": -12075.9,
      "set": 12345.7
    }
  }
}
```

data from group:

```json
{
  "event": "read",
  "type_req": "read_attr_gr",
  "id_req": 2,
  "data": {
    "sys/tg_test/1": {
      "double_scalar": {
        "data": -244.814,
        "set": 0
      }
    },
    "sys/tg_test/2": {
      "double_scalar": {
        "data": -128.001,
        "set": 0
      }
    }
  }
}
```

## output from command

Data from response. Command to device in single mode

```json
{
  "event": "read",
  "type_req": "command",
  "id_req": 4,
  "data": {
    "command_name": "DevDouble",
    "argout": 123.456
  }
}
```

Data from response. Command to device in group mode

```json
{
  "event": "read",
  "type_req": "command",
  "id_req": 23,
  "data": {
    "command_name": "DevLong",
    "device_name": "sys/tg_test/1",
    "argout": 123456
  }
}
```

Data from response. Command to group in group mode

```json
{
  "event": "read",
  "type_req": "command_group",
  "id_req": 2,
  "data": {
    "command_name": "DevDouble",
    "argout": {
      "sys/tg_test/1": 1e6,
      "sys/tg_test/2": 1e6
    }
  }
}
```

## data from pipe

Data from device in group mode

```json
{
  "event": "read",
  "type_req": "read_pipe_dev",
  "pipe_name": "string_long_short_ro",
  "device_name": "sys/tg_test/1",
  "id_req": 14,
  "data": {
    "FirstDE": "The string",
    "SecondDE": 666,
    "ThirdDE": 12,
    "DOUBLE_1": 123.457,
    "DOUBLE_2": 1.65432e-15
  }
}
```

Data from group in group mode

```json
{
  "event": "read",
  "type_req": "read_pipe_gr",
  "pipe_name": "string_long_short_ro",
  "id_req": 0,
  "data": {
    "sys/tg_test/1": {
      "FirstDE": "The string",
      "SecondDE": 666,
      "ThirdDE": 12,
      "DOUBLE_1": 123.457,
      "DOUBLE_2": 1.65432e-15
    },
    "sys/tg_test/2": {
      "FirstDE": "The string",
      "SecondDE": 666,
      "ThirdDE": 12,
      "DOUBLE_1": 123.457,
      "DOUBLE_2": 1.65432e-15
    }
  }
}
```

Data from device in single mode

```json
{
  "event": "read",
  "type_req": "read_pipe",
  "pipe_name": "string_long_short_ro",
  "id_req": 1,
  "data": {
    "FirstDE": "The string",
    "SecondDE": 666,
    "ThirdDE": 12,
    "DOUBLE_1": 123.457,
    "DOUBLE_2": 1.65432e-15
  }
}
```
