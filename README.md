WebSocketDS
===========

- **[Server mode](#server-mode)**
    - **[Data from device](#data-from-device)**
    - **[Data from group of devices](#data-from-group-of-devices)**
    - **[Reading data from pipe](#reading-data-from-pipe)**
    - **[Running commands](#running-commands)**
    - **[Write attribute](#write-attribute)**
    
- **[Client mode](#client-mode)**
    - **[Timer control](#timer-control)**
    - **[Reading data from attributes and pipe](#reading-data-from-attributes-and-pipe)**
    - **[Client running of commands](#client-running-of-commands)**
    - **[Client attribute writing](#client-attribute-writing)**

- **[Work with events](#work-with-events)**
    - **[Subscribing to events](#subscribing-to-events)**
    - **[Full unsubscription from events](#full-unsubscription-from-events)**
    - **[Partial unsubscription from events](#partial-unsubscription-from-events)**
    - **[Get the subscriber id](#get-the-subscriber-id)** 

- **[Authorization and authentication](#authorization-and-authentication)**
    - **[Method SIMPLE](#method-simple)**
    - **[Method USERANDIDENT](#method-userandident)**
    - **[Method USERANDIDENT2](#method-userandident)**
    - **[Method USERANDIDENT3](#method-userandident3)**
    
- **[Logging](#logging)**

- **[Additional parameters for attributes, pipe and commands](#additional-parameters-for-attributes-pipe-and-commands)**
    - **[Changing the  precision and formatting the output values of attributes, pipe, and commands](#changing-the-precision-and-formatting-the-output-values-of-attributes-pipe-and-commands)**
    - **[Setting the periodicity of the output of values for attributes](#setting-the-periodicity-of-the-output-of-values-for-attributes)**

- **[The format of the returned error messages](#the-format-of-the-returned-error-messages)**
- **[Property list for defining Tango device](#property-list-for-defining-tango-device)**

- **[Device operating mode](#device-operating-mode)**


## Server mode

Server mode is `ser` or `ser_cli_all_ro` or `ser_cli_all` or `ser_cli_ali` or  `ser_cli_ali_ro`

The listening device must be written in the property `DeviceServer`

The list of readable attributes must be listed in the property `Attributes`

If you want to listen to a group of devices, the "Options" property must contain a string `group`. `DeviceServer` must contain name pattern for group (e.g., domain_*/family/member_*)

#### Data from device

If the device (output):

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
	],
	"pipe":
		{
			"attrName1" : "Data in format dependent on type",
			"attrName2" : ["Data in format", "dependent on type"]
		}
}
```


#### Data from group of devices

If you want to listen to a group of devices, the "Options" property must contain a string `group`.

If a group of devices (output):

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
	},
	"pipe":
		{
			"nameof/tango/device_from_group":
			{
				"attrName1" : "Data in format dependent on type",
				"attrName2" : ["Data in format", "dependent on type"]
			}
		}
}
```

#### Reading data from pipe

Input message:

```json
{     
	"type_req": "read_pipe or read_pipe_dev or read_pipe_gr",
	"pipe_name": "PipeName",
	"device_name": "!!!Only when reading from a specific device in a group mode!!!",
	"id": "Id to identify the request"
}
```

 - `"read_pipe"` - Single device. If the property `DeviceServer` is device.
 - `"read_pipe_dev"` - Single device. If the property `DeviceServer` is group of devices.
 - `"read_pipe_gr"` - Group of devices. If the property `DeviceServer` is group of devices.
 
Output message:

For `"read_pipe"`

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
  
For `"read_pipe_dev"` in mode `group`:

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
  
#### Running commands

The list of available commands must be listed in the property `Commands`

```json
{
	"type_req": "command or command_device or command_group",
	"id": "Request id",
	"device_name": "!!!Only when reading from a specific device in a group mode!!!",
	"command_name": "Command name",
	"argin" : "value or array"
}
```

 - `"command"` - Single device. If the property `DeviceServer` is device.
 - `"command_device"` - Single device. If the property `DeviceServer` is group of devices.
 - `"command_group"` - Group of devices. If the property `DeviceServer` is group of devices.
 
 Output message:
 
 For `"command"`
 
 ```json
{
	"event": "read",
	"type_req": "command",
	"id_req": "Request id",
	"data": {
		"command_name": "Command name",
		"argout": "value or array"
	}
}
```

 For `"command_device"`
 
 ```json
  {
  	"event": "read",
  	"type_req": "command_device",
  	"id_req": "Request id",
  	"data": {
  		"command_name": "Command name",
  		"device_name": "name/tango/device_from_group",
  		"argout": "value or array"
  	}
  }
  ```
  
  For `"command_group"`
  
  ```json
  {
  	"event": "read",
  	"type_req": "command_group",
  	"id_req": "Request id",
  	"data": {
  		"command_name": "Command name",
  		"argout": {
  			"name/tango/device_from_group" : "value or array",
  			"name/tango/other_device_from_group": {
  				"errors": "Possible error message, or [message array]"
  			}
  		}
  	}
  }
  ```

#### Write attribute

The list of writable attributes must be listed in the property `Attributes` with postfix `;wrt` or `;onlywrt` (if only for writing)

```text
AttributeName;wrt
or
AttributeName;onlywrt
```

Input JSON:

```json
{
	"type_req": "write_attr or write_attr_gr or write_attr_dev",
	"attr_name": "Attribute name",
	"device_name": "Device name if write_attr_dev",
	"id": "Request id",
	"argin": "value or [array]",
	"dimX": "only for Image type",
	"dimY": "only for Image type"
}
```

Request types:
 
 - `"write_attr"` - Single device. If the property `DeviceServer` is device.
 - `"write_attr_dev"` - Single device. If the property `DeviceServer` is group of devices.
 - `"write_attr_gr"` - Group of devices. If the property `DeviceServer` is group of devices.
 
 Output message (if succesfull):
 
```json
{
	"event":"read", 
	"type_req": "write_attr", 
	"id_req": 0, 
	"resp": "Was written to the attribute."
}
```
 
 `"resp"` Can contain devices for which the write operation is not successful (If using the group mode)
  
## Client mode

Client mode is everything except `ser`. (`ser_cli_all_ro`, `ser_cli_all`, `ser_cli_ali_ro`, `ser_cli_ali`, `cli_all_ro`, `cli_all`, `cli_ali_ro`, `cli_ali`)

List of modes [here](#device-operating-mode) 

#### Timer control

Input message for timer control

```json
{
  "type_req" : "(always) Type request",
  "id_req": "(always) Request id",
  "msec": "(Depending on the type) Values for the timer in milliseconds (int). minimum 1000",
  "devices" : "(Depending on the type) object or value or array"
}
```

Types of requests:

 - **timer_start** - Start the timer. Also should be sent `"msec": value` and `devices` (object)
 - **timer_stop** - Stop the timer. Request type only.
 - **timer_change** - Changes in the data update period. Also should be sent `"msec": value`
 - **timer_add_devs** - Adding devices to the list of listeners. Also should be sent `devices` (object)
 - **timer_remove_devs** - Removal of devices from the list of listeners. Also should be sent `devices` (value or array)
 - **timer_upd_devs_add** - Adding attributes or pipe. If the device is already in the list. Also should be sent `devices` (object)
 - **timer_upd_devs_rem** - Removing attributes or pipe. If the device is already in the list. Also should be sent `devices` (object)
 - **timer_check** - Check the status of the timer.
 
For devices of object type:

```json
{
	"other" : "data",
	"devices": {
		"name/of/device": {
			"attr": "attribute name or array of names",
			"pipe": "PipeName and (If it is needed) Properties for attributes"
		}
	}
}
```

You can specify the output frequency (Once in N iterations) and the iteration in which the output is made. [Read more here](#setting-the-periodicity-of-the-output-of-values-for-attributes) 

For devices a value type or an array

```json
{
  ...
  "devices" : "name/of/device or Array of names"
}
```

Output message:

```json
{
	"event": "read",
	"type_req": "attribute_client",
	"data": {
		"name/of/tangodevice": {
			"attrs": [{
				"attr": "Attribute name",
				"data": "value or array"
			}, {
				"attr": "Name of another attribute",
				"data": "value or array"
			}],
			"pipe": {
				"Attribute name": "value or array",
				"Name of another attribute": "value or array"
			}
		}
	}
}
```



#### Reading data from attributes and pipe

Input message:

```json
{
	"type_req": "attr_device_cl",
	"id": "Request id",
	"device_name": "Name of the device or alias",
	"attributes": "attribute name or array of names",
	"pipes": ["pipe_name","attr_name;param"]
}
```

In order to read data from all attributes of the tango module.

```
"attributes": "__all_attrs__"
```

Output message:

```json
{
	"event": "read",
	"type_req": "attr_device_cl",
	"device_name": "имя девайса или alias",
	"id_req": "id запроса",
	"data": [{
		"attr": "Attribute name",
		"data": "value or array"
	}],
	"pipe": {
		"attr_name": "value or array",
		"attr_name_2": "value or array"
	}
}
```

#### Client running of commands

If the mode is not "read-only"

Input message:

```json
{
	"type_req": "command_device_cl",
	"id": "Request id",
	"device_name": "name/of/device or alias",
	"command_name": "Command name"
}
```

Output message:

```json
{
	"event": "read",
	"type_req": "command_device_cl",
	"id_req": "Request id",
	"data": {
		"command_name": "Command name",
		"device_name": "name/of/device or alias",
		"argout": "value or array"
	}
}
```

#### Client attribute writing

If the mode is not "read-only"

Input JSON:

```json
{
	"type_req": "write_attr_dev_cl",
	"attr_name": "Attribute name",
	"device_name": "Device name",
	"id": "Request id",
	"argin": "value or [array]",
	"dimX": "only for Image type",
	"dimY": "only for Image type"
}
```

Output message (if succesfull):

```json
{
	"event": "read", 
	"type_req": "write_attr_dev_cl", 
	"id_req": 0, 
	"resp": "Was written to the attribute."
}
```

## Work with events

Subscription to events is possible in all modes except `Server`.

#### Subscribing to events

The following event types are currently supported: `CHANGE_EVENT`, `PERIODIC_EVENT`, `ARCHIVE_EVENT`, `USER_EVENT`

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
	"event":"read",
	"type_req": "eventreq_add_dev", 
	"id_req": 2, 
	"resp": [
		{
			"device": "name/of/device", 
			"attribute": "attribute name", 
			"event_type": "Event type", 
			"event_id": 2
		}
	]
}
```

Here `"event_id"` is the event subscription id. It can be used to further unsubscribe from the event.

#### Full unsubscription from events

To unsubscribe from all events subscribed to, you need to send a message:

```json
{
	"type_req": "eventreq_off",
	"id": "id"
}
```

#### Partial unsubscription from events

To unsubscribe from individual events, you need to send:

```json
{
	"type_req": "eventreq_rem_dev",
	"id": "id",
	"event_id": 12345
}
```

Here `"event_id"` is the event subscription id.

#### Get the subscriber id

If you want to get the subscriber id, send

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
	"event":"read",
	"type_req": "eventreq_check_dev", 
	"id_req": 2, 
	"resp": [
		{
			"device": "name/of/device", 
			"attribute": "attribute name", 
			"event_type": "Event type", 
			"event_id": -1
		}
	]
}
```


## Authorization and authentication

To execute the command, the client must be authenticated.

Authorization and authentication are performed in the Device `AuthDS`.

#### Method SIMPLE

`ws(wss)://ip_or_hostname:port?login=zzz&password=zzz`

Device `AuthDS` must contain method `check_user(const Tango::DevVarStringArray (*argin)` and return true or false.

   * **\(\*argin)[0]** = login
   * **\(\*argin)[1]** = password
   
Also the server should contain the method `check_permissions(const Tango::DevVarStringArray *argin)`, used for authentication. return true or false.
 
   * **\(\*argin)[0]** — Device name
   * **\(\*argin)[1]** — Running command
   * **\(\*argin)[2]** — Ip
   * **\(\*argin)[3]** — login 

#### Method USERANDIDENT

`ws(wss)://ip_or_hostname:port?login=yyy&rand_ident=yyy&rand_ident_hash=yyy`

To activate this feature, you need to define `tident=rndid` in Property "Options"

Device `AuthDS` must contain method `check_user_ident(const Tango::DevVarStringArray *argin)` and return true or false.

   * **\(\*argin)[0]** — login
   * **\(\*argin)[1]** — rand_ident. Random number/word stored/generated by the client
   * **\(\*argin)[2]** — rand_ident_hash. For example MD5(rand_ident+login)
   
#### Method USERANDIDENT2

__2-step verification__

To activate this feature, you need to define `tident=rndid2` in Property "Options"

The same as in [Method USERANDIDENT](#method-userandident), but authentication will be after connecting

Authentication is executed in several stages.

Requests must be made sequentially

First. To start the authorization process, the user must send a message requesting a random number.

   ```json
   {
    "type_req": "rident_req",
    "id": "Request id",
    "login": "login"
   }
   ```
   
Second. The server generates a random number, stores it. Sends the following type of response to the client:

  ```json
  {
  	"event": "read",
  	"type_req": "rident_req",
  	"id_req": "Request id",
  	"rident": "random number"
  }
  ```
  
Third. The client calculates (depending on the requirements of the authorization module) the required response, and sends it to the server.

  ```json
  {
  	"type_req": "rident_ans",
  	"id": "Request id",
  	"rident_hash": "For example MD5(random_number_from_request+login)"
  }
  ```
  
Finish. The server will send a response:

  ```json
  {
  	"event": "read",
  	"type_req": "rident_ans",
  	"id_req": "Request id",
  	"success": "true or false"
  }
  ```
   
#### Method USERANDIDENT3

The same as in [Method USERANDIDENT](#method-userandident), but authentication will be after connecting

To activate this feature, you need to define `tident=rndid3` in Property "Options"

To authorize, send:

  ```json
  {
  	"type_req": "rident",
  	"id": "Request id",
  	"login": "login",
  	"rident": "rand_ident",
  	"rident_hash": "rand_ident_hash"
  }
  ```
  
The server will send a response:
  
  ```json
  {
  	"event": "read",
  	"type_req": "rident",
  	"id_req": "Request id",
  	"success": "true or false"
  }
  ```  

## Logging

To activate this feature, you need to define `uselog` in Property "Options"

Device `AuthDS` must contain method `send_log_command_ex(const Tango::DevVarStringArray *argin)`

 - **id** - autoincrement
 - **argin[0]** = timestamp_string (UNIX_TIMESTAMP)
 - **argin[1]** = login
 - **argin[2]** = deviceName (or group)
 - **argin[3]** = IP
 - **argin[4]** = commandName 
 - **argin[5]** = command in Json (вводимая команда в json формате)
 - **argin[6]** = statusBool (true if successful, otherwise false)
 - **argin[7]** = isGroup (true if group, otherwise false)

## Additional parameters for attributes, pipe and commands

Used in server mode

To add additional parameters, add in the postfix property

```
CommandOrAttrName;par1=val
```

or for several parameters

```
CommandOrAttrName;par1=val;par2;par3=34
```

Where par - is parameter, val - is value if required.

For pipe from updating data (property PipeName):

```
PipeName
AttrName;par1=val;par2
```

In the case of pipe in command mode (when requested) , parameters for individual attributes are set to property `Commands`. The name of the attribute must be appended with `;pipecomm`, as well as the required parameter.

```
AttrName;pipecomm;par1=val;par2
```

List of currently available parameters for commands and attributes:

 - For attributes, commands and pipe: **precf**, **precs**, **prec**
 - Only for attributes: **niter**
 - Only for command: **bindata**
 
#### Changing the  precision and formatting the output values of attributes, pipe, and commands

By default, `setprecision(5)` is used to output attribute, pipe, and command data.

To set other  precision values, add a postfix `;prec=N` to the attribute name. 

N is the required precision.

```
AttributeOrCommandName;prec=10
```

It is also possible to set additional formatting flags:

- **precf** - Setting the flag std::fixed
- **precs** - Setting the flag std::scientific

Example output for 1476379200 (type double with precision = 10)

- **;prec=10** - output: 1476379200
- **;precf=10** - output: 1476379200.0000000000
- **;precs=10** - output: 1.4763792000e+009

Example output for 1476379200 (type double with precision  by default)
- **;precf** - output: 1476379200.000000
- **;precs** - output: 1.476379e+009

#### Setting the periodicity of the output of values for attributes

Used in server mode or for timer

For individual attributes, you can set the frequency of the output. To set the periodicity for the attribute name in the property (or in input message for timer), add `;niter=N/M` or `;niter=N`

 - **N** - Output once in N iterations.  (unsigned short)
 - **M** - The iteration number at which the value is displayed.
 
`M` should be less than `N`. The default value for `M` is `0`.

## The format of the returned error messages

Reading attributes in server mode:

```json
{
	"event": "error",
	"type_req": "attribute",
	"err_mess": "Сообщение ошибки"
}
```

Other Requests:

```json
{
	"event": "error",
	"type_req": "Request Type",
	"name_req": "Request Name (If used)",
	"id_req": "Request id",
	"err_mess": "message or [Message array]"
}
```

## Property list for defining Tango device

  - **Mode** — Device operating mode. [Device operating mode](#device-operating-mode); (string)  
  - **Port** — Listening port at connection; (DevShort)
  - **DeviceServer** - tango id of the used device. As template parameter may serve simple name of a device or name pattern for group (e.g., `domain_*/family/member_*`). `Used only if any server mode is selected.` ; (string)
  - **Attributes** — — a list of device attributes you want to read, if reading all attributes is required, add __all_attrs__ (not operational in group mode) `Used only if any server mode is selected.` ; (array of string)
  - **Commands** — a list of device commands you want to execute through WS. `Used only if any server mode is selected.` ; (array of string)
  - **PipeName** - Name of DevicePipe for reading. [0]
When using GROUP, the DevicePipe name must be the same for all devices.
If you want to set properties for specific attributes, add them in the format `NameAttr;property`
Used only if any server mode is selected. ; (array of string)
  - **AuthDS** — Tango web authentication device server (TangoWebAuth ) name.
Responsible for user authentication in case of commands execution ; (string)
  - **Secure** — Shall we use SSL encryption?
Set true, for secure wss connection, otherwise false;  (bool)
  - **Certificate** — Full path to the certificate in use (if Secure == true); (string)
  - **Key** - Full path to the file used with Private key (if Secure == true); (string)
  - **MaxNumberOfConnections** - maximum number of connections. If the limit is reached, further connections will be lost with `400 Bad Request error`. If 0 is set, the number of connections will be unlimited. ; (DevUShort)
  - **MaximumBufferSize** - maximum buffer size for each connection, KiB. The Default value is 1000. Possible values range from1 to 10000 (if setting a value outside the range, the default value will be set). If exceeding the set maximum buffer size, the connection will be lost by the server; (DevULong)
  - **ResetTimestampDifference** - The difference in timestamps (seconds) after which a WS server is reset. The difference is counted by CheckPoll method between update timestamp in UpdateData method and current timestamp. Minimum value is 60. 
Default and MinValue = 60
Used only if any server mode is selected ; (DevUShort)
  - **Options** - additional options. A list of additional options for the device. (array of string)
  
## Device operating mode

Tango module works in one of the following modes:

  - **SERVER** - Only server control of the output of information read from the attributes and pipe. And also the execution of commands in the tango-module (or group), registered in the Property. In this mode, the module is started by default. mode:  `ser`.
  - **SERVNCLIENT_ALL_RO** - Server and client management of the output of information read from the attributes and pipe. And also the execution of commands in the tango-module (or group), registered in the Property; mode:  `ser_cli_all_ro`.
  - **SERVNCLIENT_ALL** - Server and client management of the output of information read from the attributes and pipe. Running the commands in the tango module (or group) specified in the Property. Also the execution of commands and reading of attributes from any tango modules; mode: `ser_cli_all`
  - **SERVNCLIENT_ALIAS_RO** - Server and client management of the output of information read from the attributes and pipe. Running the commands in the tango module (or group) specified in the Property. It is forbidden to run commands in the modules specified by the client. Reading attributes only from tango modules that have alias; option: `ser_cli_ali_ro`
  - **SERVNCLIENT_ALIAS** - Server and client management of the output of information read from the attributes and pipe. Running the commands in the tango module (or group) specified in the Property. Reading attributes and the execution of commands only from tango modules that have alias; mode: `ser_cli_ali`
  - **CLIENT_ALL_RO** - Client control of the output of information read from the attributes and pipe. When using this mode, only the user-defined data comes from the server. Also it is forbidden to run commands; mode: `cli_all_ro`
  - **CLIENT_ALL** - Client control of the output of information read from the attributes and pipe. When using this mode, only the user-defined data comes from the server. It is possible to start commands on any tango-modules; mode: `cli_all`
  - **CLIENT_ALIAS_RO** - Client control of the output of information read from attributes and pipe only from tango modules that have alias. When using this mode, only the user-defined data comes from the server.; mode: `cli_ali_ro`
  - **CLIENT_ALIAS** - Client control of the output of information read from attributes and pipe, only from tango modules that have alias. When using this mode, only the user-defined data comes from the server. It is also possible to run commands only from tango modules that have alias; mode: `cli_ali`