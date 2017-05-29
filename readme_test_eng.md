WebSocketDS
===========

- **[Installation](#installation)**
- **[List of properties for device](#list-of-properties-for-device)**
- **[List of Attributes](#list-of-attributes)**
- **[Description of the "Options" property](#description-of-the-options-property)**
- **[Modes of operation of the tango module](#modes-of-operation-of-the-tango-module)**
- **[Simple use only with reading attributes](#simple-use-only-with-reading-attributes)**
  - **[Reading attributes from the Tango Device](reading-attributes-from-the-tango-device)**
  - **[Reading attributes from the TangoDevice group](reading-attributes-from-the-tangodevice-group)**
- **[Reading from the pipe of connected devices](reading-from-the-pipe-of-connected-devices)**
  - **[Reading with pipe in attribute mode](reading-with-pipe-in-attribute-mode)**
  - **[Reading from the pipe in command mode](reading-from-the-pipe-in-command-mode)**

## Installation

  For successful compilation, TANGO libraries and a compiler with support for c ++ 11 should be installed. Also should be __installed libboost-system libboost-system-dev libssl__

## List of properties for device

  - **Port** — port to listen incoming ws connections; (DevShort)
  - **DeviceServer** - Using DeviceServer name (`name/of/device`) or a device name pattern (e.g. `domain_* / family/ member_*`) for communicate with a group of devices; (string)
  - **Attributes** — List of device attributes that you want to read, if you want to read all the attributes, add `__all_attrs__` (In group mode is not valid); (array of string)
  - **Commands** — List of device commands that you want to perform WS; (array of string)
  - **PipeName** - PipeName for pipe of device. You can also add a list of attributes with options for output; (array of string)
  - **AuthDS** — The tango-module that is responsible for user authentication, if there are executable commands; (string)
  - **Secure** — Set to true, to use a secure wss connection, otherwise false; (bool)
  - **Certificate** — Full path to the certificate in use (if Secure == true); (string)
  - **Key** - Full path to the file used with Private key (if Secure == true); (string)
  - **MaxNumberOfConnections** - Maximum number of connections. If the limit is reached, subsequent connections will be interrupted by an error `400 Bad Request`. If the value is 0, the number of connections will not be limited.; (DevUShort)
  - **MaximumBufferSize** - The maximum buffer size for each connection in the KiB. The default value is 1000. Possible values from 1 to 10000. If the specified maximum buffer size is exceeded, the connection will be interrupted by the server; (DevULong)
  - **ResetTimestampDifference** - The difference between the timestamps (in seconds) after which the server WS will be rebooted. Calculation of the difference is made in the CheckPoll method between the time-updateable in the UpdateData method and the current time-stamping. Minimum value is 60.; (DevUShort)
  - **Options** - additional options. A list of additional options for the device in the format `opt1;opt2=val;opt3`.More information about the list of additional options will be [given below](#description-of-the-options-property)??? !!!. (string)

  <h2><b><font color="red"> The CheckPoll method checks how many seconds have elapsed since the last time the UpdateData method was run. If the value exceeds the value specified in the property, the WS-device will reboot.  Do not set the polling value for the UpdateData near to the value of ResetTimestampDifference </font></b></h2>

## List of Attributes

  - **JSON** - current JSON-output; (DevString)
  - **TimestampDiff** - Current timestamps difference (Updated in the CheckPoll method. The default value of Polling for CheckPoll method is 10 seconds); (DevULong)
  - **NumberOfConnections** - Current number of clients (connections). (DevULong)

## Description of the "Options" property

  The Options property for the tango device has the format `opt1; opt2 = val; opt3`, where` opt` is the name of the option, `val` is the value, if present. The options are separated by a semicolon `;`, it needs to be set only between options.

  List of available options:

  - **group** - Without additional values. Use of groups of devices. The `DeviceServer` property must be specified in the appropriate format for groups.
  - **uselog** - Without additional values. Use the logging when executing commands.  [Подробнее ниже](#Использование-записи-в-журнал-при-выполении-команд)??? !!!.
  - **tident** - With additional values. Type of authorization that is used. There are three types `rndid` - RANDIDENT, `rndid2` - RANDIDENT2 и `rndid3` RANDIDENT3 ([подробности ниже](#Использование-userandident-для-авторизации))??? !!!.
  - **mode** - With additional values. Used mode of the module. More information about modes types is [described below](#modes-of-operation-of-the-tango-module)??? !!!.
  - **tm100ms** - Without additional values. By default, the minimum value for the update period set by the client should be greater than or equal to 1000 ms. When this option is set in Options, the minimum value is 100 ms.

## Modes of operation of the tango module

Tango module works in one of the following modes:

  - **SERVER** - Only server control of the output of information read from the attributes and pipe. And also the launch of commands in the tango-module (or group), registered in the Property. In this mode, the module is started by default.
  - **SERVNCLIENT_ALL_RO** - Server and client management of the output of information read from the attributes and pipe. And also the launch of commands in the tango-module (or group), registered in the Property; option:  `mode=ser_cli_all_ro`.
  - **SERVNCLIENT_ALL** - Server and client management of the output of information read from the attributes and pipe. Running the commands in the tango module (or group) specified in the Property. Also launching commands and reading attributes from any tango modules; option: `mode=ser_cli_all`
  - **SERVNCLIENT_ALIAS_RO** - Server and client management of the output of information read from the attributes and pipe. Running the commands in the tango module (or group) specified in the Property. It is forbidden to run commands in the modules specified by the client. Reading attributes only from tango modules that have alias; option: `mode=ser_cli_ali_ro`
  - **SERVNCLIENT_ALIAS** - Server and client management of the output of information read from the attributes and pipe. Running the commands in the tango module (or group) specified in the Property. Reading attributes, as well as launching commands only from tango modules that have alias; option: `mode=ser_cli_ali`
  - **CLIENT_ALL_RO** - Client control of the output of information read from the attributes and pipe. When using this mode, only the user-defined data comes from the server. Also it is forbidden to run commands; option: `mode=cli_all_ro`
  - **CLIENT_ALL** - Client control of the output of information read from the attributes and pipe. When using this mode, only the user-defined data comes from the server. It is possible to start commands on any tango-modules; option: `mode=cli_all`
  - **CLIENT_ALIAS_RO** - Client control of the output of information read from attributes and pipe only from tango modules that have alias. When using this mode, only the user-defined data comes from the server.; option: `mode=cli_ali_ro`
  - **CLIENT_ALIAS** - Client control of the output of information read from attributes and pipe, only from tango modules that have alias. When using this mode, only the user-defined data comes from the server. It is also possible to run commands only from tango modules that have alias; option: `mode=cli_ali`

To enable a mode other than SERVER, you need to add the value specified in the mode to the Property `Options`. ([More about the options format here](#description-of-the-options-property)). Format: `mode=Selected_mode`. ??? !!!
[More information about the output control](#Клиентное-управление-обновляемым-выводом-информации)??? !!!

## Simple use only with reading attributes

To use WS only for reading attributes, you must define `Port`,` DeviceServer` (or Group), and the list of `Attributes` to be read. If you use a secure wss connection, you need to set the `Secure` property to true, and also specify` Certificate` and `Key`.

[More information about JSON can be found here](http://www.json.org/)

#### Reading attributes from the Tango Device

By default the format of the received data will be as follows:
```json
{
	"event": "read",
	"type_req":"attribute",
	"data": [
		{
			"attr": "attribute_name",
			"data": "data"
		},
		{
			"attr": "attribute_name2",
			"data": ["data", "array"]
		},
	]
}
```

Also, values UNIX_TIMESTAMP `" time "` and `Tango :: AttrQuality`` "qual" `can be added. By default, `Tango :: AttrQuality` is displayed only if it is not equal to `VALID`.

```json
{
	"attr": "attribute_name",
	"qual": "VALID",
	"time": 1475580424,
	"data": 128
}
```

To add these values, add the value `notshrtatt` in the Property` "Options" ` ([More about the options format here](#description-of-the-options-property)).

  * **"event"** - Type of event, in this case it is `reading`
  * **"type_req"** - The type of data received, in this case it is attribute. Data from the listened attributes.
  * **"data"** - Gotten data. (Attributes)

    * **"attr"** — Name of the listened attribute.
    * **"qual"** — `Tango::AttrQuality` Possible Values: `"VALID`", `"INVALID"`,`"ALARM"`, `"CHANGING"`, `"WARNING"`;
    * **"time"** — UNIX_TIMESTAMP
    * **"data"** — Data. Depending on the type of attributes being read (Scalar, Spectrum, Image), the output format changes. For Scalar, this is the only value in the form of a string, a number, or a Boolean value. For Spectrum, it is an array of the form [...], and the value of `"dimX"` the dimension of the spectrum is also defined. For Image- this is an array of the form [...], also the value of `"dimX"` and `"dimY"` is the dimension of Image.

  If errors occur while reading the attributes, an error message is displayed in JSON format. The format is described below in the section `"[Формат возвращаемых ошибок](#Формат-возвращаемых-ошибок)"` ??? !!!

#### Reading attributes from the TangoDevice group

An example of the format of the received data from the group of devices is as follows:

```json
{
	"event": "read",
	"type_req":"group_attribute",
	"data":
	{
		"name/tango/device_from_group" :
		[
			{
				"attr": "attribute_name",
				"data": "data"
			},
			{
				"attr": "attribute_name2",
				"data": ["data", "array"]
			},
		]
	}
}
```

If an error occurs when reading from a separate device from the group, the data format for it will be:

```json
{
	"name/tango/device_from_group" : "Error message"
}
```

## Reading from the pipe of connected devices

Reading is possible in the attribute and command modes:
  - In the attribute mode, the data from the pipe is read when the data is updated with the attributes and added to the corresponding JSON-output.
  - In the command mode, the data from the pipe is read at the request. The JSON-input is sent in the required format (described below).

#### Reading with pipe in attribute mode

PipeName has type `array of string`. To read data from the pipe of the connected devices in the attribute mode, the `pipe_name` - pipe name of the readable device must be added to the `PipeName[0]` property.
To format the output of individual pipe segments, you need to add lines with the names of the segment to the property, and the necessary types of formatting. For more information, see "[Дополнительные параметры для атрибутов, pipe и команд](#Дополнительные-параметры-для-атрибутов-pipe-и-команд)"

In the attribute mode, you can connect to a single pipe from the device.

If there is a PipeName in the property, JSON adds values in the following format:

__In case of success:__

  a) __for device:__

  ```json
	{ "attribute_data" : "data",
		"pipe":
		{
			"attrName1" : "Data in format dependent on type",
			"attrName2" : ["Data in format", "dependent on type"]
		}
	}
  ```
  b) __for group:__

 ```json
	{ "attribute_data" : "data",
		"pipe":
		{
			"nameof/tango/device":
			{
				"attrName1" : "Data in format dependent on type",
				"attrName2" : ["Data in format", "dependent on type"]
			}
		}
	}
 ```

__In case of error:__

  a) __for device:__

   ```json
  { "attribute_data" : "data",
    "pipe" : "Error message"
  }
   ```

   b) __for group:__

   ```json
  { "attribute_data" : "data",
    "pipe":
    {
      "nameof/tango/device": "Error message"
    }
  }
   ```

 `"Error message"` - Can also be an array of messages in the format `["Message1", "Message2"]`

#### Reading from the pipe in command mode

To read from the pipe in command mode, send a JSON message in the following format:

```json
{     
	"type_req": "read_pipe or read_pipe_dev or read_pipe_gr",
	"pipe_name": "PipeName",
	"device_name": "!!!Only when reading from a specific device in a group mode!!!",
	"id": "Id to identify the request"
}
```

  * __"read_pipe or read_pipe_dev or read_pipe_gr"__: Name of reading pipe. Used `"read_pipe"` in the mode of one device. `"read_pipe_dev"` in the mode `group`, for reading from a specific device from the group. `"read_pipe_gr"` in the mode `group`, for reading from all devices from the group.
  * __"device_name"__: Name of device in mode `group`, for reading from a specific device from the group.
  * __"id"__: Identification of the request.  The value for id can be either a number or a string. In case the id is not specified, is returned `"id": "None"`.

To format the output of individual pipe segments, you need to add lines with the names of the segment to the property, and the necessary types of formatting. For more information, see "[Дополнительные параметры для атрибутов, pipe и команд](#Дополнительные-параметры-для-атрибутов-pipe-и-команд)"

  - The answer for `"read_pipe"` in single device mode:

  ```json
  {
  	"event": "read",
  	"type_req": "read_pipe",
  	"id_req": 1,
  	"data": {
  		"AttrName": "data",
  		"AttrName2": ["data", "data"]
  	}
  }
  ```

  - For `"read_pipe_dev"` in mode `group`:

  ```json
  {
  	"event": "read",
  	"type_req": "read_pipe_dev",
  	"device_name": "name/tango/device_from_group",
  	"id_req": "id",
  	"data": {
  		"AttrName": "data",
  		"AttrName2": ["data", "data"]
  	}
  }
  ```

  - For reading from all devices from the group `"read_pipe_gr"` in mode `group`:

  ```json
  {
  	"event": "read",
  	"type_req": "read_pipe_gr",
  	"device_name": "name/tango/device_from_group",
  	"id_req": "id",
  	"data" : {
  		"name/tango/device_from_group": {
  			"AttrName": "data",
  			"AttrName2": ["data", "data"]
  		},
  		"name/tango/other_device_from_group": "error messageо [or Message array]"
  	}
  }
  ```

 If the error occurs, depending on the operation mode and command type, the message is either added to the JSON-output for the group in the device partition where the error occurred, or the general error message is displayed in the format described below in the section "[Формат возвращаемых ошибок](#Формат-возвращаемых-ошибок)"
