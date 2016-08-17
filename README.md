# WebSocketDS

WebSocket access to tango device-server attributes.

Configuration should be done via properties:

 - Port - port to listen incoming ws connections;
 - DeviceServer - tango id of a required device server;
 - Attributes - list of required DS attributes, you wish to read via WS;
 - Commands - list of required DS commandes, you wish to executed via WS;
 - AuthDS - Tango web authentication device server (TangoWebAuth ) name.
 - Secure - It will be used wss connection (websocket secure). (true if you want)
 - Certificate - Certificate file name (crt) with full path (if Secure = true)
 - Key - Private key file name (if Secure = true)

Then you should set polling to the UpdateData command. (1000 means that all connected clients would read attributes once per second).

Data format: JSON string with array of attrubute objects {atrrtibute name, attribute value, quality, timestamp};

if you want to record in the logs, define #USELOG in makefile.
The database (defined in AuthDS) must contain a table `command_history` with columns:

* id - autoincrement
* argin[0] = timestamp_string UNIX_TIMESTAMP
* argin[1] = login
* argin[2] = deviceName
* argin[3] = IP
* argin[4] = commandName
* argin[5] = commandJson
* argin[6] = statusBool

### Installation

You need installed Tango

```sh
$ git clone [git-repo-url] 
```