Testing the WebSocketDS
=======================

For testing, add a file `"my_sett.js"` in directory `js`

Example:

```javascript
var protocol = "ws";
var host = "localhost";

window.myWsAddr = protocol + "://" + host + ":6789?login=test&password=test";

window.myPassword = "test"; // 
window.myLogin = "test";    //
```

For normal working, this script should contain the global variable `window.myWsAddr`, with URL of your websocket.

## USERANDIDENT2

If you use a method `USERANDIDENT2` for authentication (To activate this feature, you need to define `tident=rndid2` in Property "Options"), `"my_sett.js"` should contain the global variable `window.myPassword` and `window.myLogin`.

Device `AuthDS` must contain method `check_user_ident(const Tango::DevVarStringArray *argin)` and return true or false.

From this script, the following data is sent to the server:

```
login = "your Login";
rident_hash = MD5( rident + MD5( login ) );
rident;
```

Where `rident` - is value sent from the server.


