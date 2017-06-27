/*----- PROTECTED REGION ID(WebSocketDS.h) ENABLED START -----*/
//=============================================================================
//
// file :        WebSocketDS.h
//
// description : Include file for the WebSocketDS class
//
// project :     
//
// This file is part of Tango device class.
// 
// Tango is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Tango is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Tango.  If not, see <http://www.gnu.org/licenses/>.
// 
// $Author:  $
//
// $Revision:  $
// $Date:  $
//
// $HeadURL:  $
//
//=============================================================================
//                This file is generated by POGO
//        (Program Obviously used to Generate tango Object)
//=============================================================================


#ifndef WebSocketDS_H
#define WebSocketDS_H

#include <tango.h>

#include <chrono>

#include "common.h"
#include "WSTangoConn.h"


/*----- PROTECTED REGION END -----*/	//	WebSocketDS.h

/**
 *  WebSocketDS class description:
 *    WebSocket access to tango device-server attributes.
 *    
 *    Configuration should be done via properties:
 *    
 *    Port - port to listen incoming ws connections;
 *    DeviceServer - tango id of a required device server;
 *    Attributes - list of required DS attributes, you wish to read via WS;
 *    Commands - list of required DS commandes, you wish to executed via WS;
 *    AuthDS - Tango web authentication device server (TangoWebAuth ) name.
 *    Secure - It will be used wss connection (websocket secure). (true if you want)
 *    Certificate - Certificate file name (crt) with full path (if Secure = true)
 *    Key - Private key file name (if Secure = true)
 *    Options - Various options for the device server
 *    
 *    Then you should set polling to the UpdateData command. (1000 means that all connected clients would read attributes once per second).
 *    
 *    Data format: JSON string with array of attrubute objects {atrrtibute name, attribute value, quality, timestamp};
 *    
 *    if you want to record in the logs, define uselog in Property ``Options``.
 *    The database (defined in AuthDS) must contain a table `command_history` with columns:
 *        // id - autoincrement
 *        // argin[0] = timestamp_string UNIX_TIMESTAMP
 *        // argin[1] = login
 *        // argin[2] = deviceName
 *        // argin[3] = IP
 *        // argin[4] = commandName
 *        // argin[5] = commandJson
 *        // argin[6] = statusBool
 *        // argin[7] = isGroup
 */

namespace WebSocketDS_ns
{
/*----- PROTECTED REGION ID(WebSocketDS::Additional Class Declarations) ENABLED START -----*/
//    class WSTangoConn;


/*----- PROTECTED REGION END -----*/	//	WebSocketDS::Additional Class Declarations

class WebSocketDS : public TANGO_BASE_CLASS
{

/*----- PROTECTED REGION ID(WebSocketDS::Data Members) ENABLED START -----*/

private:
    //WSThread *wsThread;
    //std::unique_ptr<GroupOrDeviceForWs> groupOrDevice;
    std::unique_ptr<WSTangoConn> wsTangoConn;

    std::chrono::seconds timeFromUpdateData;
    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::Data Members

//	Device property data members
public:
	//	Mode:	Device operating mode
	//  
	//  ser - Server mode
	//  ser_cli_all - Server mode. Client mode (You can use all devices)
	//  ser_cli_all_ro - Server mode. Client mode (You can use all devices only for reading attributes and pipes)
	//  ser_cli_ali - Server mode. Client mode. (You can use devices that have an alias.)
	//  ser_cli_ali_ro - Server mode. Client mode. (You can use devices that have an alias only for reading attributes and pipes)
	//  cli_all - Client mode (You can use all devices)
	//  cli_all_ro - Client mode (You can use all devices only for reading attributes and pipes)
	//  cli_ali -  Client mode. (You can use devices that have an alias.)
	//  cli_ali_ro - Client mode. (You can use devices that have an alias only for reading attributes and pipes)
	string	mode;
	//	Port:	Using port of WebSocket
	Tango::DevShort	port;
	//	DeviceServer:	Using DeviceServer name 
	//  or  a device name pattern (e.g. domain_* / family/ member_*) for communicate with a group of devices.
	//  Used only if any server mode is selected.
	string	deviceServer;
	//	Attributes:	A list of device attributes you want to read, if reading all attributes is required, add __all_attrs__ (not operational in group mode); 
	//  Used only if any server mode is selected.
	vector<string>	attributes;
	//	Commands:	a list of device commands you want to execute through WS
	//  Used only if any server mode is selected.
	vector<string>	commands;
	//	PipeName:	Name of DevicePipe for reading. [0]
	//  When using GROUP, the DevicePipe name must be the same for all devices.
	//  If you want to set properties for specific attributes, add them in the format ``NameAttr;property``
	//  Used only if any server mode is selected.
	vector<string>	pipeName;
	//	Secure:	Shall we use SSL encryption?
	//  set true, for secure wss connection, otherwise false;
	Tango::DevBoolean	secure;
	//	Certificate:	full path to the certificate in use (if Secure = true)
	//  example: /etc/ssl/certs/ssl-cert-snakeoil.pem
	string	certificate;
	//	Key:	full path to the file in use with Private key (if Secure = true)
	//  Example: /etc/ssl/private/ssl-cert-snakeoil.key
	string	key;
	//	AuthDS:	Tango web authentication device server (TangoWebAuth ) name.
	//  responsible for user authentication in case of commands execution
	string	authDS;
	//	MaxNumberOfConnections:	maximum number of connections. If the limit is reached, further connections will be lost with 400 Bad Request error. If 0 is set, the number of connections will be unlimited.
	Tango::DevUShort	maxNumberOfConnections;
	//	MaximumBufferSize:	maximum buffer size for each connection, KiB. The Default value is 1000. Possible values range from1 to 10000 (if setting a value outside the range, the default value will be set). If exceeding the set maximum buffer size, the connection will be lost by the server;
	Tango::DevUShort	maximumBufferSize;
	//	ResetTimestampDifference:	The difference in timestamps (seconds) after which a WS server is reset. The difference is counted by CheckPoll method between update timestamp in UpdateData method and current timestamp. Minimum value is 60. 
	//  Default and MinValue = 60
	//  Used only if any server mode is selected
	Tango::DevUShort	resetTimestampDifference;
	//	Options:	Options for device.
	//  Format of options:
	//  	nameOfOption or nameOfOption=value
	vector<string>	options;

//	Attribute data members
public:
	Tango::DevString	*attr_JSON_read;
	Tango::DevULong	*attr_TimestampDiff_read;
	Tango::DevULong	*attr_NumberOfConnections_read;

//	Constructors and destructors
public:
	/**
	 * Constructs a newly device object.
	 *
	 *	@param cl	Class.
	 *	@param s 	Device Name
	 */
	WebSocketDS(Tango::DeviceClass *cl,string &s);
	/**
	 * Constructs a newly device object.
	 *
	 *	@param cl	Class.
	 *	@param s 	Device Name
	 */
	WebSocketDS(Tango::DeviceClass *cl,const char *s);
	/**
	 * Constructs a newly device object.
	 *
	 *	@param cl	Class.
	 *	@param s 	Device name
	 *	@param d	Device description.
	 */
	WebSocketDS(Tango::DeviceClass *cl,const char *s,const char *d);
	/**
	 * The device object destructor.
	 */
	~WebSocketDS() {delete_device();};


//	Miscellaneous methods
public:
	/*
	 *	will be called at device destruction or at init command.
	 */
	void delete_device();
	/*
	 *	Initialize the device
	 */
	virtual void init_device();
	/*
	 *	Read the device properties from database
	 */
	void get_device_property();
	/*
	 *	Always executed method before execution command method.
	 */
	virtual void always_executed_hook();


//	Attribute methods
public:
	//--------------------------------------------------------
	/*
	 *	Method      : WebSocketDS::read_attr_hardware()
	 *	Description : Hardware acquisition for attributes.
	 */
	//--------------------------------------------------------
	virtual void read_attr_hardware(vector<long> &attr_list);

/**
 *	Attribute JSON related methods
 *	Description: 
 *
 *	Data type:	Tango::DevString
 *	Attr type:	Scalar
 */
	virtual void read_JSON(Tango::Attribute &attr);
	virtual bool is_JSON_allowed(Tango::AttReqType type);
/**
 *	Attribute TimestampDiff related methods
 *	Description: The difference between the timestamps from UpdateData and CheckPoll
 *
 *	Data type:	Tango::DevULong
 *	Attr type:	Scalar
 */
	virtual void read_TimestampDiff(Tango::Attribute &attr);
	virtual bool is_TimestampDiff_allowed(Tango::AttReqType type);
/**
 *	Attribute NumberOfConnections related methods
 *	Description: Number of WS clients
 *
 *	Data type:	Tango::DevULong
 *	Attr type:	Scalar
 */
	virtual void read_NumberOfConnections(Tango::Attribute &attr);
	virtual bool is_NumberOfConnections_allowed(Tango::AttReqType type);


	//--------------------------------------------------------
	/**
	 *	Method      : WebSocketDS::add_dynamic_attributes()
	 *	Description : Add dynamic attributes if any.
	 */
	//--------------------------------------------------------
	void add_dynamic_attributes();




//	Command related methods
public:
	/**
	 *	Command On related method
	 *	Description: 
	 *
	 */
	virtual void on();
	virtual bool is_On_allowed(const CORBA::Any &any);
	/**
	 *	Command Off related method
	 *	Description: 
	 *
	 */
	virtual void off();
	virtual bool is_Off_allowed(const CORBA::Any &any);
	/**
	 *	Command UpdateData related method
	 *	Description: 
	 *
	 */
	virtual void update_data();
	virtual bool is_UpdateData_allowed(const CORBA::Any &any);
	/**
	 *	Command Reset related method
	 *	Description: Restart websocket server
	 *
	 */
	virtual void reset();
	virtual bool is_Reset_allowed(const CORBA::Any &any);
	/**
	 *	Command CheckPoll related method
	 *	Description: 
	 *
	 */
	virtual void check_poll();
	virtual bool is_CheckPoll_allowed(const CORBA::Any &any);


	//--------------------------------------------------------
	/**
	 *	Method      : WebSocketDS::add_dynamic_commands()
	 *	Description : Add dynamic commands if any.
	 */
	//--------------------------------------------------------
	void add_dynamic_commands();

/*----- PROTECTED REGION ID(WebSocketDS::Additional Method prototypes) ENABLED START -----*/

private:
    void reInitDevice();


    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::Additional Method prototypes
};

/*----- PROTECTED REGION ID(WebSocketDS::Additional Classes Definitions) ENABLED START -----*/


/*----- PROTECTED REGION END -----*/	//	WebSocketDS::Additional Classes Definitions

}	//	End of namespace

#endif   //	WebSocketDS_H
