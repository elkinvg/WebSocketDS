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

### Installation

You need installed Tango

```sh
$ git clone [git-repo-url] 
```