/*----- PROTECTED REGION ID(WebSocketDS.cpp) ENABLED START -----*/
static const char *RcsId = "$Id:  $";
//=============================================================================
//
// file :        WebSocketDS.cpp
//
// description : C++ source for the WebSocketDS class and its commands.
//               The class is derived from Device. It represents the
//               CORBA servant object which will be accessed from the
//               network. All commands which can be executed on the
//               WebSocketDS are implemented in this file.
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


#include <WebSocketDS.h>
#include <WebSocketDSClass.h>
#include <cmath>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>    

#include "WSThread_plain.h"
#include "WSThread_tls.h"

#include "StringProc.h"
#include "DeviceForWs.h"
#include "GroupForWs.h"

/*----- PROTECTED REGION END -----*/	//	WebSocketDS.cpp

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
 *    
 *    Then you should set polling to the UpdateData command. (1000 means that all connected clients would read attributes once per second).
 *    
 *    Data format: JSON string with array of attrubute objects {atrrtibute name, attribute value, quality, timestamp};
 *    
 *    if you want to record in the logs, define #USELOG in makefile.
 *    The database (defined in AuthDS) must contain a table `command_history` with columns:
 *        // id - autoincrement
 *        // argin[0] = timestamp_string UNIX_TIMESTAMP
 *        // argin[1] = login
 *        // argin[2] = deviceName
 *        // argin[3] = IP
 *        // argin[4] = commandName
 *        // argin[5] = commandJson
 *        // argin[6] = statusBool
 */

//================================================================
//  The following table gives the correspondence
//  between command and method names.
//
//  Command name    |  Method name
//================================================================
//  State           |  Inherited (no method)
//  Status          |  Inherited (no method)
//  On              |  on
//  Off             |  off
//  UpdateData      |  update_data
//  SendCommand     |  send_command
//  Reset           |  reset
//  CheckPoll       |  check_poll
//  SendCommandBin  |  send_command_bin
//================================================================

//================================================================
//  Attributes managed are:
//================================================================
//  JSON                 |  Tango::DevString	Scalar
//  TimestampDiff        |  Tango::DevULong	Scalar
//  NumberOfConnections  |  Tango::DevULong	Scalar
//================================================================

namespace WebSocketDS_ns
{
/*----- PROTECTED REGION ID(WebSocketDS::namespace_starting) ENABLED START -----*/

//    static initializations

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::namespace_starting

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::WebSocketDS()
 *	Description : Constructors for a Tango device
 *                implementing the classWebSocketDS
 */
//--------------------------------------------------------
WebSocketDS::WebSocketDS(Tango::DeviceClass *cl, string &s)
 : TANGO_BASE_CLASS(cl, s.c_str())
{
	/*----- PROTECTED REGION ID(WebSocketDS::constructor_1) ENABLED START -----*/
    init_device();

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::constructor_1
}
//--------------------------------------------------------
WebSocketDS::WebSocketDS(Tango::DeviceClass *cl, const char *s)
 : TANGO_BASE_CLASS(cl, s)
{
	/*----- PROTECTED REGION ID(WebSocketDS::constructor_2) ENABLED START -----*/
    init_device();

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::constructor_2
}
//--------------------------------------------------------
WebSocketDS::WebSocketDS(Tango::DeviceClass *cl, const char *s, const char *d)
 : TANGO_BASE_CLASS(cl, s, d)
{
	/*----- PROTECTED REGION ID(WebSocketDS::constructor_3) ENABLED START -----*/
    init_device();

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::constructor_3
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::delete_device()
 *	Description : will be called at device destruction or at init command
 */
//--------------------------------------------------------
void WebSocketDS::delete_device()
{
	DEBUG_STREAM << "WebSocketDS::delete_device() " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::delete_device) ENABLED START -----*/

    if (wsThread != nullptr) {
        wsThread->stop();
        void *ptr;
        DEBUG_STREAM << "Waiting for the thread to exit" << endl;
        wsThread->join(&ptr);
    }  

    CORBA::string_free(*attr_JSON_read);

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::delete_device
	delete[] attr_JSON_read;
	delete[] attr_TimestampDiff_read;
	delete[] attr_NumberOfConnections_read;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::init_device()
 *	Description : will be called at device initialization.
 */
//--------------------------------------------------------
void WebSocketDS::init_device()
{
	DEBUG_STREAM << "WebSocketDS::init_device() create device " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::init_device_before) ENABLED START -----*/

    //    Initialization before get_device_property() call

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::init_device_before
	

	//	Get the device properties from database
	get_device_property();
	
	attr_JSON_read = new Tango::DevString[1];
	attr_TimestampDiff_read = new Tango::DevULong[1];
	attr_NumberOfConnections_read = new Tango::DevULong[1];
	/*----- PROTECTED REGION ID(WebSocketDS::init_device) ENABLED START -----*/
    attr_JSON_read[0] = Tango::string_dup("[{\"success\": false}]");
    attr_NumberOfConnections_read[0] = 0;
    

    timeFromUpdateData = std::chrono::seconds(std::time(NULL));
    attr_TimestampDiff_read[0] = 0;

    if (resetTimestampDifference < 60)
        resetTimestampDifference = 60;

    wsThread = nullptr;
    bool isDsInit = initDeviceServer();

    if (!isDsInit) {
        set_state(Tango::FAULT);
        set_status("Couldn't connect to device: " + deviceServer);
        return;
    }

    bool isWsThreadInit = initWsThread();
    if (!isWsThreadInit) {
        set_state(Tango::FAULT);
        set_status("Couldn't init wsThread");
        return;
    }

    set_state(Tango::ON);
    set_status("Device is On");

    groupOrDevice->initAttrCommPipe(attributes, commands,pipeName);

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::init_device
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::get_device_property()
 *	Description : Read database to initialize property data members.
 */
//--------------------------------------------------------
void WebSocketDS::get_device_property()
{
	/*----- PROTECTED REGION ID(WebSocketDS::get_device_property_before) ENABLED START -----*/

    //    Initialize property data members
    //port = 9002;

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::get_device_property_before


	//	Read device properties from database.
	Tango::DbData	dev_prop;
	dev_prop.push_back(Tango::DbDatum("DeviceServer"));
	dev_prop.push_back(Tango::DbDatum("Port"));
	dev_prop.push_back(Tango::DbDatum("Attributes"));
	dev_prop.push_back(Tango::DbDatum("Commands"));
	dev_prop.push_back(Tango::DbDatum("PipeName"));
	dev_prop.push_back(Tango::DbDatum("Secure"));
	dev_prop.push_back(Tango::DbDatum("Certificate"));
	dev_prop.push_back(Tango::DbDatum("Key"));
	dev_prop.push_back(Tango::DbDatum("AuthDS"));
	dev_prop.push_back(Tango::DbDatum("MaxNumberOfConnections"));
	dev_prop.push_back(Tango::DbDatum("MaximumBufferSize"));
	dev_prop.push_back(Tango::DbDatum("ResetTimestampDifference"));
	dev_prop.push_back(Tango::DbDatum("Options"));

	//	is there at least one property to be read ?
	if (dev_prop.size()>0)
	{
		//	Call database and extract values
		if (Tango::Util::instance()->_UseDb==true)
			get_db_device()->get_property(dev_prop);
	
		//	get instance on WebSocketDSClass to get class property
		Tango::DbDatum	def_prop, cl_prop;
		WebSocketDSClass	*ds_class =
			(static_cast<WebSocketDSClass *>(get_device_class()));
		int	i = -1;

		//	Try to initialize DeviceServer from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  deviceServer;
		else {
			//	Try to initialize DeviceServer from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  deviceServer;
		}
		//	And try to extract DeviceServer value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  deviceServer;

		//	Try to initialize Port from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  port;
		else {
			//	Try to initialize Port from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  port;
		}
		//	And try to extract Port value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  port;

		//	Try to initialize Attributes from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  attributes;
		else {
			//	Try to initialize Attributes from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  attributes;
		}
		//	And try to extract Attributes value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  attributes;

		//	Try to initialize Commands from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  commands;
		else {
			//	Try to initialize Commands from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  commands;
		}
		//	And try to extract Commands value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  commands;

		//	Try to initialize PipeName from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  pipeName;
		else {
			//	Try to initialize PipeName from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  pipeName;
		}
		//	And try to extract PipeName value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  pipeName;

		//	Try to initialize Secure from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  secure;
		else {
			//	Try to initialize Secure from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  secure;
		}
		//	And try to extract Secure value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  secure;

		//	Try to initialize Certificate from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  certificate;
		else {
			//	Try to initialize Certificate from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  certificate;
		}
		//	And try to extract Certificate value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  certificate;

		//	Try to initialize Key from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  key;
		else {
			//	Try to initialize Key from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  key;
		}
		//	And try to extract Key value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  key;

		//	Try to initialize AuthDS from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  authDS;
		else {
			//	Try to initialize AuthDS from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  authDS;
		}
		//	And try to extract AuthDS value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  authDS;

		//	Try to initialize MaxNumberOfConnections from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  maxNumberOfConnections;
		else {
			//	Try to initialize MaxNumberOfConnections from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  maxNumberOfConnections;
		}
		//	And try to extract MaxNumberOfConnections value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  maxNumberOfConnections;

		//	Try to initialize MaximumBufferSize from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  maximumBufferSize;
		else {
			//	Try to initialize MaximumBufferSize from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  maximumBufferSize;
		}
		//	And try to extract MaximumBufferSize value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  maximumBufferSize;

		//	Try to initialize ResetTimestampDifference from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  resetTimestampDifference;
		else {
			//	Try to initialize ResetTimestampDifference from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  resetTimestampDifference;
		}
		//	And try to extract ResetTimestampDifference value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  resetTimestampDifference;

		//	Try to initialize Options from class property
		cl_prop = ds_class->get_class_property(dev_prop[++i].name);
		if (cl_prop.is_empty()==false)	cl_prop  >>  options;
		else {
			//	Try to initialize Options from default device value
			def_prop = ds_class->get_default_device_property(dev_prop[i].name);
			if (def_prop.is_empty()==false)	def_prop  >>  options;
		}
		//	And try to extract Options value from database
		if (dev_prop[i].is_empty()==false)	dev_prop[i]  >>  options;

	}

	/*----- PROTECTED REGION ID(WebSocketDS::get_device_property_after) ENABLED START -----*/

    //    Check device property data members init

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::get_device_property_after
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::always_executed_hook()
 *	Description : method always executed before any command is executed
 */
//--------------------------------------------------------
void WebSocketDS::always_executed_hook()
{
	//DEBUG_STREAM << "WebSocketDS::always_executed_hook()  " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::always_executed_hook) ENABLED START -----*/

    if (groupOrDevice == nullptr || wsThread == nullptr) {
        reInitDevice();
        return;
    }

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::always_executed_hook
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::read_attr_hardware()
 *	Description : Hardware acquisition for attributes
 */
//--------------------------------------------------------
void WebSocketDS::read_attr_hardware(TANGO_UNUSED(vector<long> &attr_list))
{
	DEBUG_STREAM << "WebSocketDS::read_attr_hardware(vector<long> &attr_list) entering... " << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::read_attr_hardware) ENABLED START -----*/

    //    Add your own code

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::read_attr_hardware
}

//--------------------------------------------------------
/**
 *	Read attribute JSON related method
 *	Description: 
 *
 *	Data type:	Tango::DevString
 *	Attr type:	Scalar
 */
//--------------------------------------------------------
void WebSocketDS::read_JSON(Tango::Attribute &attr)
{
	DEBUG_STREAM << "WebSocketDS::read_JSON(Tango::Attribute &attr) entering... " << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::read_JSON) ENABLED START -----*/
    //    Set the attribute value
    attr.set_value(attr_JSON_read);

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::read_JSON
}
//--------------------------------------------------------
/**
 *	Read attribute TimestampDiff related method
 *	Description: The difference between the timestamps from UpdateData and CheckPoll
 *
 *	Data type:	Tango::DevULong
 *	Attr type:	Scalar
 */
//--------------------------------------------------------
void WebSocketDS::read_TimestampDiff(Tango::Attribute &attr)
{
	DEBUG_STREAM << "WebSocketDS::read_TimestampDiff(Tango::Attribute &attr) entering... " << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::read_TimestampDiff) ENABLED START -----*/
	//	Set the attribute value
	attr.set_value(attr_TimestampDiff_read);
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::read_TimestampDiff
}
//--------------------------------------------------------
/**
 *	Read attribute NumberOfConnections related method
 *	Description: Number of WS clients
 *
 *	Data type:	Tango::DevULong
 *	Attr type:	Scalar
 */
//--------------------------------------------------------
void WebSocketDS::read_NumberOfConnections(Tango::Attribute &attr)
{
	DEBUG_STREAM << "WebSocketDS::read_NumberOfConnections(Tango::Attribute &attr) entering... " << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::read_NumberOfConnections) ENABLED START -----*/
	//	Set the attribute value
	attr.set_value(attr_NumberOfConnections_read);
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::read_NumberOfConnections
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::add_dynamic_attributes()
 *	Description : Create the dynamic attributes if any
 *                for specified device.
 */
//--------------------------------------------------------
void WebSocketDS::add_dynamic_attributes()
{
	/*----- PROTECTED REGION ID(WebSocketDS::add_dynamic_attributes) ENABLED START -----*/

    //    Add your own code to create and add dynamic attributes if any

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::add_dynamic_attributes
}

//--------------------------------------------------------
/**
 *	Command On related method
 *	Description: 
 *
 */
//--------------------------------------------------------
void WebSocketDS::on()
{
	DEBUG_STREAM << "WebSocketDS::On()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::on) ENABLED START -----*/

    set_state(Tango::ON);
    set_status("Device is On");

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::on
}
//--------------------------------------------------------
/**
 *	Command Off related method
 *	Description: 
 *
 */
//--------------------------------------------------------
void WebSocketDS::off()
{
	DEBUG_STREAM << "WebSocketDS::Off()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::off) ENABLED START -----*/

    set_state(Tango::OFF);
    set_status("Device is Off");

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::off
}
//--------------------------------------------------------
/**
 *	Command UpdateData related method
 *	Description: 
 *
 */
//--------------------------------------------------------
void WebSocketDS::update_data()
{
	DEBUG_STREAM << "WebSocketDS::UpdateData()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::update_data) ENABLED START -----*/

    timeFromUpdateData = std::chrono::seconds(std::time(NULL));
    string jsonStrOut;
    try {
        jsonStrOut = groupOrDevice->generateJsonForUpdate();

        CORBA::string_free(*attr_JSON_read);
        attr_JSON_read[0] = Tango::string_dup(jsonStrOut.c_str());
    }
    catch (Tango::ConnectionFailed &e)
    {
        fromException(e, "update_data.read_attr ConnectionFailed ");
        string mes = "ConnectionFailed from: " + deviceServer;
        jsonStrOut = StringProc::exceptionStringOut(mes);
    }
    catch (Tango::CommunicationFailed &e)
    {
        fromException(e, "update_data.read_attr CommunicationFailed ");
        string mes = "CommunicationFailed from: " + deviceServer;
        jsonStrOut = StringProc::exceptionStringOut(mes);
    }
    catch (Tango::DevFailed &e)
    {
        fromException(e, "update_data.read_attr ");
        reInitDevice();
        return;
    }
    catch (...) {
        reInitDevice();
        return;
    }

    wsThread->send_all(jsonStrOut.c_str());

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::update_data
}
//--------------------------------------------------------
/**
 *	Command SendCommand related method
 *	Description: Command for sending command to device from property.
 *
 *	@param argin input argument must be in JSON. Command must be included to device property ``Commands``
 *               {``command`` : ``nameOfCommand``, ``argin`` : [``1``,``2``,``3``]}
 *               OR
 *               {``command`` : ``nameOfCommand``, ``argin`` : ``1``}
 *	@returns Output in JSON.
 */
//--------------------------------------------------------
Tango::DevString WebSocketDS::send_command(Tango::DevString argin)
{
	Tango::DevString argout;
	DEBUG_STREAM << "WebSocketDS::SendCommand()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::send_command) ENABLED START -----*/

    argout = groupOrDevice->sendCommand(argin);

	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::send_command
	return argout;
}
//--------------------------------------------------------
/**
 *	Command Reset related method
 *	Description: Restart websocket server
 *
 */
//--------------------------------------------------------
void WebSocketDS::reset()
{
	DEBUG_STREAM << "WebSocketDS::Reset()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::reset) ENABLED START -----*/
    auto instance = Tango::Util::instance();// get_dserver_device();
    auto dsd = instance->get_dserver_device();
    dsd->restart_server();
#ifdef TESTFAIL
    sendLogToFile();
#endif
    //reInitDevice();
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::reset
}
//--------------------------------------------------------
/**
 *	Command CheckPoll related method
 *	Description: 
 *
 */
//--------------------------------------------------------
void WebSocketDS::check_poll()
{
	DEBUG_STREAM << "WebSocketDS::CheckPoll()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::check_poll) ENABLED START -----*/
	
     std::chrono::seconds  checkPollTime = std::chrono::seconds(std::time(NULL));
     Tango::DevULong cpTime,updTime;
     //Tango::DevULong diffTime;
     cpTime = checkPollTime.count();
     updTime = timeFromUpdateData.count();
     attr_TimestampDiff_read[0] = cpTime - updTime;
     DEBUG_STREAM << "WebSocketDS::CheckPoll()  - " << device_name << " time difference: " << attr_TimestampDiff_read[0]  << endl;

     if (attr_TimestampDiff_read[0] > resetTimestampDifference)
         reset();

	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::check_poll
}
//--------------------------------------------------------
/**
 *	Command SendCommandBin related method
 *	Description: Command for sending command to device from property.
 *
 *	@param argin input argument must be in JSON. Command must be included to device property ``Commands``
 *               {``command`` : ``nameOfCommand``, ``argin`` : [``1``,``2``,``3``]}
 *               OR
 *               {``command`` : ``nameOfCommand``, ``argin`` : ``1``}
 *	@returns Output in binary data
 */
//--------------------------------------------------------
Tango::DevVarCharArray *WebSocketDS::send_command_bin(Tango::DevString argin)
{
	Tango::DevVarCharArray *argout;
	DEBUG_STREAM << "WebSocketDS::SendCommandBin()  - " << device_name << endl;
	/*----- PROTECTED REGION ID(WebSocketDS::send_command_bin) ENABLED START -----*/

    argout = groupOrDevice->sendCommandBin(argin);
    
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::send_command_bin
	return argout;
}
//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::add_dynamic_commands()
 *	Description : Create the dynamic commands if any
 *                for specified device.
 */
//--------------------------------------------------------
void WebSocketDS::add_dynamic_commands()
{
	/*----- PROTECTED REGION ID(WebSocketDS::add_dynamic_commands) ENABLED START -----*/

    //    Add your own code to create and add dynamic commands if any

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::add_dynamic_commands
}

/*----- PROTECTED REGION ID(WebSocketDS::namespace_ending) ENABLED START -----*/

string WebSocketDS::readPipeFromDeviceOrGroup(const std::map<std::string, std::string> &pipeConf)
{
    return groupOrDevice->generateJsonFromPipeComm(pipeConf);
}

OUTPUT_DATA_TYPE WebSocketDS::check_type_of_data(const string& commandName) {
    //groupOrDevice->checkOption();
    return groupOrDevice->checkDataType(commandName);
}

void WebSocketDS::reInitDevice() {
    delete_device();
    init_device();
}

void WebSocketDS::fromException(Tango::DevFailed &e, string func)
{
    auto lnh = e.errors.length();
    for (int i=0;i<lnh;i++) {
        ERROR_STREAM << " From " + func + ": " << e.errors[i].desc << endl;
    }
}

bool WebSocketDS::initDeviceServer()
{
    groupOrDevice = nullptr;
    bool isInit = false;
    try {
        if (options.size()) {
            std::vector<std::string> gettedOptions = StringProc::parseInputString(options, ";", true);
            for (auto& opt : gettedOptions) {
                if (opt == "group")
                    _isGroup = true;
                if (opt == "uselog")
                    _isLogActive = true;
                if (opt.find("tident") != string::npos) {
                    auto gettedIdentOpt = StringProc::parseInputString(opt, "=", true);
                    if (gettedIdentOpt.size() > 1) {
                        if (gettedIdentOpt[1] == "rndid")
                            typeOfIdent = TYPE_OF_IDENT::RANDIDENT;
                        if (gettedIdentOpt[1] == "smpl")
                            typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                        else
                            typeOfIdent = TYPE_OF_IDENT::SIMPLE;
                    }
                }
                if (opt == "notshrtatt") {
                    _isShortAttr = false;
                }
            }
        }

        if (_isGroup) {
            groupOrDevice = unique_ptr<GroupForWs>(new GroupForWs(this, deviceServer));
        }
        else 
            groupOrDevice = unique_ptr<DeviceForWs>(new DeviceForWs(this, deviceServer));
        isInit = true;
    }
    catch (Tango::DevFailed &e)
    {
        fromException(e,"initDeviceServer(). ");
    }
    return isInit;
}

bool WebSocketDS::initWsThread()
{
    bool isInit = false;
    try
    {
        if (!secure){
            wsThread = new WSThread_plain(this, port);
        } else {
            wsThread = new WSThread_tls(this,port,certificate,key);
        }
        isInit = true;
    }
    catch (Tango::DevFailed &e)
    {
        fromException(e,"initDeviceServer(). ");
    }
    return isInit;
}

#ifdef TESTFAIL
void WebSocketDS::sendLogToFile()
{
    std::fstream fs;
    fs.open ("/tmp/tango_log/web_socket/test_log.out", std::fstream::in | std::fstream::out | std::fstream::app);
    Tango::DevULong cTime;
    std::chrono::seconds  timeFromUpdateData= std::chrono::seconds(std::time(NULL));
    cTime = timeFromUpdateData.count();
    fs << cTime << " : Websocket deviceserver reloaded" << endl;

    fs.close();
}
#endif

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::namespace_ending
} //	namespace
