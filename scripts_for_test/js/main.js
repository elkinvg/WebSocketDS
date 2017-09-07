$(document).ready(function () {
    var webSocket = null;
    var wsUri = window.myWsAddr; // from my_sett.js
    try {
        initWebSocket(wsUri, onMessage);
    }
    catch (ex) {
        reconnect();
    }

	webSocket.binaryType = 'arraybuffer';

	var id = 0;
	var event_sub_id = 0;
    var timeout = 0;
    var rident = 0;

    if (test_device === undefined)
        var test_device = "sys/tg_test/1";

    if (test_group_device === undefined)
        var test_group_device = "sys/tg_test/*";

    if (test_pipe_name === undefined)
        var test_pipe_name = "string_long_short_ro";

    if (window.myLogin === undefined)
        var myLogin = "unknown";
    else
        var myLogin = window.myLogin;

    if (window.myPassword === undefined)
        var myPassword = "unknown";
    else
        var myPassword = myPassword;

    // Server mode___________________________________________________

    // SINGLE DEVICE

    /**
     * Command to the device. Mode is not "group"
     * Command "DevFloat" must be written in the propertie "Command"
     */
    $("#send_command_dev").click(function(){
        argin = {};
        argin.type_req = "command";
        argin.command_name = "DevFloat";
        argin.argin = 123.456;
        send_message_to_ws(argin);
    });

    /**
     * Read pipe from device.
     *
     */
    $("#pipe_comm").click(function(){
        argin = {};
        argin.type_req = "read_pipe";
        argin.pipe_name = test_pipe_name;
        send_message_to_ws(argin);
    });

    /**
     * Write data to the device attribute
     * Attribute "double_scalar" must be written in the propertie "Attributes"
     *      with postfix ";wrt" or "onlywrt". Example double_scalar;wrt
     */
    $("#attr_write_ser").click(function(){
        argin = {};
        argin.type_req = "write_attr";
        argin.attr_name = "double_scalar";
        argin.argin = 3456.7891;
        send_message_to_ws(argin);
    });

    // GROUP OF DEVICE

    /**
     * Write data to the device (from group) attribute
     * Attribute "short_scalar" must be written in the propertie "Attributes"
     *      with postfix ";wrt" or "onlywrt". Example short_scalar;wrt
     */
    $("#attr_write_dev").click(function(){
        argin = {};
        argin.type_req = "write_attr_dev";
        argin.attr_name = "short_scalar";
        argin.device_name = test_device;
        argin.argin = 1000;
        send_message_to_ws(argin);
    });

    /**
     * Write data to the group attribute
     * Attribute "double_scalar" must be written in the propertie "Attributes"
     *      with postfix ";wrt" or "onlywrt". Example double_scalar;wrt
     */
    $("#attr_write_gr").click(function(){
        argin = {};
        argin.type_req = "write_attr_gr";
        argin.attr_name = "double_scalar";
        argin.argin = 999.999;
        send_message_to_ws(argin);
    });


    /**
     * Command to the group. Mode is "group"
     * Command "DevLong" must be written in the propertie "Command"
     */
    $("#send_com_dev_from_gr").click(function(){
        argin = {};
        argin.type_req = "command_device";
        argin.device_name = test_device;
        argin.command_name = "DevLong";
        argin.argin = 123456;
        send_message_to_ws(argin);
    });

    /**
     * Command to the group. Mode is "group"
     * Command "DevDouble" must be written in the propertie "Command"
     */
    $("#send_com_gr").click(function(){
        argin = {};
        argin.type_req = "command_group";
        argin.command_name = "DevDouble";
        argin.argin = 1000000;
        send_message_to_ws(argin);
    });

    /**
     * Read pipe from device from group.
     *
     */
    $("#pipe_comm_dev").click(function(){
        argin = {};
        argin.type_req = "read_pipe_dev";
        argin.device_name = test_device;
        argin.pipe_name = test_pipe_name;
        send_message_to_ws(argin);
    });

    /**
     * Read pipe from group.
     *
     */
    $("#pipe_comm_gr").click(function(){
        argin = {};
        argin.type_req = "read_pipe_gr";
        argin.pipe_name = test_pipe_name;
        send_message_to_ws(argin);
    });




    // Client mode __________________________________________________

    // TIMER

    /**
     * Start timer
     */

    $("#timer_start").click(function(){
        argin = {};
        argin.type_req = "timer_start";
        argin.msec = 1000;
        argin.devices = {};

        argin.devices[test_device] =
            {
                "attr": ["float_scalar", "long_scalar", "double_scalar;prec=2"],
                "pipe": test_pipe_name
            };
        send_message_to_ws(argin);
    });

    /**
     * Add device to the timer
     */

    $("#timer_add_devs").click(function(){
        argin = {};
        argin.type_req = "timer_add_devs";
        argin.devices = {
            "sys/tg_test/2": {
                "attr": ["long64_scalar","Status","double_scalar;precf=4"],
                "pipe": test_pipe_name
            }
        };
        send_message_to_ws(argin);
    });

    /**
     * Delete device from timer
     */
    $("#timer_remove_devs").click(function () {
        argin = {};
        argin.type_req = "timer_remove_devs";
        argin.devices = "sys/tg_test/2";
        send_message_to_ws(argin);
    });

    /**
     * Add attributes in device from timer
     */
    $("#timer_upd_devs_add").click(function () {
        argin = {};
        argin.type_req = "timer_upd_devs_add";

        argin.devices = {};

        argin.devices[test_device] =
            {
                "attr": ["Status", "State"]
            };
        send_message_to_ws(argin);
    });

    /**
     * Delete attributes from timer
     */
    $("#timer_upd_devs_rem").click(function () {
        argin = {};
        argin.type_req = "timer_upd_devs_rem";

        argin.devices = {};

        argin.devices[test_device] =
            {
                "attr": ["long64_scalar","Status","double_scalar"],
                "pipe": test_pipe_name
            };
        send_message_to_ws(argin);
    });

    /**
     * Check timer
     */
    $("#timer_check").click(function(){
        argin = {};
        argin.type_req = "timer_check";
        send_message_to_ws(argin);
    });

    /**
     * Change the period in the timer
     */
    $("#timer_change").click(function(){
        argin = {};
        argin.type_req = "timer_change";
        argin.msec = 3000;
        send_message_to_ws(argin);
    });

    /**
     * Stop timer
     */
    $("#timer_stop").click(function(){
        argin = {};
        argin.type_req = "timer_stop";
        send_message_to_ws(argin);
    });

    /**
     * Start timer with group of device
     */
    $("#timer_gr_start").click(function(){
        argin = {};
        argin.type_req = "timer_start";
        argin.msec = 1000;
        argin.group = {};

        argin.group[test_group_device] =
            {
                "attr": ["float_scalar", "long_scalar", "double_scalar;prec=2"],
                "pipe": test_pipe_name
            };
        send_message_to_ws(argin);
    });

    /**
     * Add group of devices in timer
     */
    $("#timer_gr_add").click(function(){
        argin = {};
        argin.type_req = "timer_add_devs";
        argin.group = {};

        argin.group[test_group_device] =
            {
                "attr": ["float_scalar", "long_scalar", "double_scalar;prec=2"],
                "pipe": test_pipe_name
            };
        send_message_to_ws(argin);
    });

    /**
     * Remove group from timer
     */
    $("#timer_gr_rem").click(function () {
        argin = {};
        argin.type_req = "timer_remove_devs";
        argin.group = test_group_device;
        send_message_to_ws(argin);
    });

    ////////////////////////////////////////
    // Command, write and read of attributes
    ////////////////////////////////////////

    /**
     * Command to the device from the client
     */
    $("#command_dev_cl").click(function(){
        argin = {};
        argin.type_req = "command_device_cl";
        argin.device_name = test_device;
        argin.command_name = "DevDouble;precf=5";
        argin.argin = 1.234;
        send_message_to_ws(argin);
    });

    /**
     * Read attribute
     */

    $("#attr_read_cl").click(function(){
        argin = {};
        argin.type_req = "attr_device_cl";
        argin.device_name = test_device;
        argin.pipe = test_pipe_name;
        argin.attributes = ["ulong_scalar", "double_scalar;prec=7"];
        send_message_to_ws(argin);
    });

    /**
     * Write attribute
     */

    $("#attr_write_cl").click(function(){
        argin = {};
        argin.type_req = "write_attr_dev_cl";
        argin.device_name = test_device;
        argin.attr_name = "float_scalar";
        argin.argin = 7250.25;
        send_message_to_ws(argin);
    });

    // EVENT SUBSCRIBER

    /**
     * Event subscription
     */
    $("#event_sub").click(function () {
        argin = {};
        argin.type_req = "eventreq_add_dev";

        var attributes ={};
        attributes[test_device] = ["double_scalar", "float_scalar;precf=3"];
        argin["periodic"] = attributes;
        send_message_to_ws(argin);
    });

    /**
     * Event unsubscription (full)
     */
    $("#event_unsub_all").click(function () {
        argin = {};
        argin.type_req = "eventreq_off";
        send_message_to_ws(argin);
    });

    /**
     * Event unsubscription (partial)
     */
    $("#event_unsub_one").click(function () {
        argin = {};
        argin.type_req = "eventreq_rem_dev";
        argin.event_sub_id = event_sub_id;
        send_message_to_ws(argin);
    });

    /**
     * Get subscriber id
     */
    $("#event_get_id").click(function () {
        argin = {};
        argin.type_req = "eventreq_check_dev";
        argin.event_type = "periodic";
        argin.device = test_device;
        argin.attribute = "double_scalar";
        send_message_to_ws(argin);
    });

    // Other buttons  __________________________________________________

    /**
     * Close connection
     */

    $("#close_conn").click(function () {
        $("#info_mess").html('the connection was closed');
        webSocket.close();
    });

    /**
     * Rand_Identification request
     */
    $("#rident_req").click(function () {
        argin = {};
        argin.login = myLogin;
        argin.type_req = "rident_req";
        send_message_to_ws(argin);
    });

    /**
     * Rand_Identification send answer
     * in function check_user_ident()
     *      md5(rand_ident+md5(login))
     */
    $("#rident_ans").click(function () {
        argin = {};
        argin.login = myLogin;
        argin.type_req = "rident_ans";
        var formd5 = rident +  $.md5(myLogin);
        var md5 = $.md5(formd5);
        argin.rident_hash = md5;
        send_message_to_ws(argin);
    });

    /**
     * send message to ws
     */
    function send_message_to_ws(argin) {
        argin.id = id;
        var sender = JSON.stringify(argin);
        webSocket.send(sender);
        id++;
    }

    /**
     * Reconnection
     */
    function reconnect() {
        clearTimeout(timeout);
        timeout = setTimeout(function () {
            initWebSocket(wsUri, onMessage);
        }, 3000);
    }

    /**
     * Init WebSocket connection
     */
    function initWebSocket(wsUri, onMessage) {
        try {
            if (typeof MozWebSocket == 'function') {
                WebSocket = MozWebSocket;
            }

            // close connection if it is open
            if (webSocket && webSocket.readyState == 1) {
                webSocket.close();
            }

            webSocket = new WebSocket(wsUri);

            webSocket.onclose = onClose;

            webSocket.onmessage = function (evt) {
                onMessage(evt.data);
            };

            webSocket.onerror = onError;
        }
        catch (ex) {
            $("#error_mess").html(ex.message);
        }
    }

    /**
     * Close
     */

    function onClose(evt) {
        if (evt.wasClean) {
            $("#info_mess").html("Connection closed cleanly");
        } else {
            $("#error_mess").html('Code: ' + evt.code + ' ||| Reason: ' + evt.reason);
        }
        reconnect();
    }

    /**
     * Error
     */

    function onError(evt) {
        $("#error_mess").html("Error: " + evt.message);
        reconnect();
    }

    /**
     * Message from WebSocket
     */
    function onMessage(data) {
        try {
            if (data.length === 0)
                return;

            var fromJson = $.parseJSON(data);

            if (fromJson.type_req === undefined) {
                $("#error_mess").html("key \"type_req\" not found");
                return;
            }

            if (fromJson.event == "error") {
                $("#error_mess").html(data);
                return;
            }

            if (fromJson.resp !== undefined) {
                $("#info_mess").html(data);
                if (Array.isArray(fromJson.resp)) {
                    var tmp = fromJson.resp[0]["event_sub_id"];
                    if (tmp !== undefined)
                        event_sub_id = tmp;
                }
                return;
            }

            if (
                fromJson.type_req == "attribute"
                || fromJson.type_req == "group_attribute"
            )
            {
                $("#out_from_server").html(data);
                return;
            }

            if (fromJson.type_req == "from_timer" ) {
                $("#out_from_timer").html(data);
                return;
            }

            if (fromJson.type_req == "from_event" ) {
                $("#out_from_event").html(data);
                return;
            }

            $("#output").html(data);

            if (fromJson.type_req === "rident_req") {
                rident = fromJson.rident;
            }

            try {
                var tmp = fromJson.data["event_sub_id"];
            }
            catch (e){}

            if (tmp !== undefined)
                event_sub_id = tmp;
        }
        catch(e) {
            $("#error_mess").html(e.message)
        }
    }

});
