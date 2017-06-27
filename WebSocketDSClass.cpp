/*----- PROTECTED REGION ID(WebSocketDSClass.cpp) ENABLED START -----*/
static const char *RcsId      = "$Id:  $";
static const char *TagName    = "$Name:  $";
static const char *CvsPath    = "$Source:  $";
static const char *SvnPath    = "$HeadURL:  $";
static const char *HttpServer = "http://www.esrf.eu/computing/cs/tango/tango_doc/ds_doc/";
//=============================================================================
//
// file :        WebSocketDSClass.cpp
//
// description : C++ source for the WebSocketDSClass.
//               A singleton class derived from DeviceClass.
//               It implements the command and attribute list
//               and all properties and methods required
//               by the WebSocketDS once per process.
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


#include <WebSocketDSClass.h>

/*----- PROTECTED REGION END -----*/	//	WebSocketDSClass.cpp

//-------------------------------------------------------------------
/**
 *	Create WebSocketDSClass singleton and
 *	return it in a C function for Python usage
 */
//-------------------------------------------------------------------
extern "C" {
#ifdef _TG_WINDOWS_

__declspec(dllexport)

#endif

	Tango::DeviceClass *_create_WebSocketDS_class(const char *name) {
		return WebSocketDS_ns::WebSocketDSClass::init(name);
	}
}

namespace WebSocketDS_ns
{
//===================================================================
//	Initialize pointer for singleton pattern
//===================================================================
WebSocketDSClass *WebSocketDSClass::_instance = NULL;

//--------------------------------------------------------
/**
 * method : 		WebSocketDSClass::WebSocketDSClass(string &s)
 * description : 	constructor for the WebSocketDSClass
 *
 * @param s	The class name
 */
//--------------------------------------------------------
WebSocketDSClass::WebSocketDSClass(string &s):Tango::DeviceClass(s)
{
	cout2 << "Entering WebSocketDSClass constructor" << endl;
	set_default_property();
	write_class_property();

	/*----- PROTECTED REGION ID(WebSocketDSClass::constructor) ENABLED START -----*/
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::constructor

	cout2 << "Leaving WebSocketDSClass constructor" << endl;
}

//--------------------------------------------------------
/**
 * method : 		WebSocketDSClass::~WebSocketDSClass()
 * description : 	destructor for the WebSocketDSClass
 */
//--------------------------------------------------------
WebSocketDSClass::~WebSocketDSClass()
{
	/*----- PROTECTED REGION ID(WebSocketDSClass::destructor) ENABLED START -----*/
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::destructor

	_instance = NULL;
}


//--------------------------------------------------------
/**
 * method : 		WebSocketDSClass::init
 * description : 	Create the object if not already done.
 *                  Otherwise, just return a pointer to the object
 *
 * @param	name	The class name
 */
//--------------------------------------------------------
WebSocketDSClass *WebSocketDSClass::init(const char *name)
{
	if (_instance == NULL)
	{
		try
		{
			string s(name);
			_instance = new WebSocketDSClass(s);
		}
		catch (bad_alloc &)
		{
			throw;
		}
	}
	return _instance;
}

//--------------------------------------------------------
/**
 * method : 		WebSocketDSClass::instance
 * description : 	Check if object already created,
 *                  and return a pointer to the object
 */
//--------------------------------------------------------
WebSocketDSClass *WebSocketDSClass::instance()
{
	if (_instance == NULL)
	{
		cerr << "Class is not initialised !!" << endl;
		exit(-1);
	}
	return _instance;
}



//===================================================================
//	Command execution method calls
//===================================================================
//--------------------------------------------------------
/**
 * method : 		OnClass::execute()
 * description : 	method to trigger the execution of the command.
 *
 * @param	device	The device on which the command must be executed
 * @param	in_any	The command input data
 *
 *	returns The command output data (packed in the Any object)
 */
//--------------------------------------------------------
CORBA::Any *OnClass::execute(Tango::DeviceImpl *device, TANGO_UNUSED(const CORBA::Any &in_any))
{
	cout2 << "OnClass::execute(): arrived" << endl;
	((static_cast<WebSocketDS *>(device))->on());
	return new CORBA::Any();
}

//--------------------------------------------------------
/**
 * method : 		OffClass::execute()
 * description : 	method to trigger the execution of the command.
 *
 * @param	device	The device on which the command must be executed
 * @param	in_any	The command input data
 *
 *	returns The command output data (packed in the Any object)
 */
//--------------------------------------------------------
CORBA::Any *OffClass::execute(Tango::DeviceImpl *device, TANGO_UNUSED(const CORBA::Any &in_any))
{
	cout2 << "OffClass::execute(): arrived" << endl;
	((static_cast<WebSocketDS *>(device))->off());
	return new CORBA::Any();
}

//--------------------------------------------------------
/**
 * method : 		UpdateDataClass::execute()
 * description : 	method to trigger the execution of the command.
 *
 * @param	device	The device on which the command must be executed
 * @param	in_any	The command input data
 *
 *	returns The command output data (packed in the Any object)
 */
//--------------------------------------------------------
CORBA::Any *UpdateDataClass::execute(Tango::DeviceImpl *device, TANGO_UNUSED(const CORBA::Any &in_any))
{
	cout2 << "UpdateDataClass::execute(): arrived" << endl;
	((static_cast<WebSocketDS *>(device))->update_data());
	return new CORBA::Any();
}

//--------------------------------------------------------
/**
 * method : 		ResetClass::execute()
 * description : 	method to trigger the execution of the command.
 *
 * @param	device	The device on which the command must be executed
 * @param	in_any	The command input data
 *
 *	returns The command output data (packed in the Any object)
 */
//--------------------------------------------------------
CORBA::Any *ResetClass::execute(Tango::DeviceImpl *device, TANGO_UNUSED(const CORBA::Any &in_any))
{
	cout2 << "ResetClass::execute(): arrived" << endl;
	((static_cast<WebSocketDS *>(device))->reset());
	return new CORBA::Any();
}

//--------------------------------------------------------
/**
 * method : 		CheckPollClass::execute()
 * description : 	method to trigger the execution of the command.
 *
 * @param	device	The device on which the command must be executed
 * @param	in_any	The command input data
 *
 *	returns The command output data (packed in the Any object)
 */
//--------------------------------------------------------
CORBA::Any *CheckPollClass::execute(Tango::DeviceImpl *device, TANGO_UNUSED(const CORBA::Any &in_any))
{
	cout2 << "CheckPollClass::execute(): arrived" << endl;
	((static_cast<WebSocketDS *>(device))->check_poll());
	return new CORBA::Any();
}


//===================================================================
//	Properties management
//===================================================================
//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::get_class_property()
 *	Description : Get the class property for specified name.
 */
//--------------------------------------------------------
Tango::DbDatum WebSocketDSClass::get_class_property(string &prop_name)
{
	for (unsigned int i=0 ; i<cl_prop.size() ; i++)
		if (cl_prop[i].name == prop_name)
			return cl_prop[i];
	//	if not found, returns  an empty DbDatum
	return Tango::DbDatum(prop_name);
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::get_default_device_property()
 *	Description : Return the default value for device property.
 */
//--------------------------------------------------------
Tango::DbDatum WebSocketDSClass::get_default_device_property(string &prop_name)
{
	for (unsigned int i=0 ; i<dev_def_prop.size() ; i++)
		if (dev_def_prop[i].name == prop_name)
			return dev_def_prop[i];
	//	if not found, return  an empty DbDatum
	return Tango::DbDatum(prop_name);
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::get_default_class_property()
 *	Description : Return the default value for class property.
 */
//--------------------------------------------------------
Tango::DbDatum WebSocketDSClass::get_default_class_property(string &prop_name)
{
	for (unsigned int i=0 ; i<cl_def_prop.size() ; i++)
		if (cl_def_prop[i].name == prop_name)
			return cl_def_prop[i];
	//	if not found, return  an empty DbDatum
	return Tango::DbDatum(prop_name);
}


//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::set_default_property()
 *	Description : Set default property (class and device) for wizard.
 *                For each property, add to wizard property name and description.
 *                If default value has been set, add it to wizard property and
 *                store it in a DbDatum.
 */
//--------------------------------------------------------
void WebSocketDSClass::set_default_property()
{
	string	prop_name;
	string	prop_desc;
	string	prop_def;
	vector<string>	vect_data;

	//	Set Default Class Properties

	//	Set Default device Properties
	prop_name = "Mode";
	prop_desc = "Device operating mode\n\nser - Server mode\nser_cli_all - Server mode. Client mode (You can use all devices)\nser_cli_all_ro - Server mode. Client mode (You can use all devices only for reading attributes and pipes)\nser_cli_ali - Server mode. Client mode. (You can use devices that have an alias.)\nser_cli_ali_ro - Server mode. Client mode. (You can use devices that have an alias only for reading attributes and pipes)\ncli_all - Client mode (You can use all devices)\ncli_all_ro - Client mode (You can use all devices only for reading attributes and pipes)\ncli_ali -  Client mode. (You can use devices that have an alias.)\ncli_ali_ro - Client mode. (You can use devices that have an alias only for reading attributes and pipes)";
	prop_def  = "ser";
	vect_data.clear();
	vect_data.push_back("ser");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Port";
	prop_desc = "Using port of WebSocket";
	prop_def  = "";
	vect_data.clear();
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "DeviceServer";
	prop_desc = "Using DeviceServer name \nor  a device name pattern (e.g. domain_* / family/ member_*) for communicate with a group of devices.\nUsed only if any server mode is selected.";
	prop_def  = "";
	vect_data.clear();
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Attributes";
	prop_desc = "A list of device attributes you want to read, if reading all attributes is required, add __all_attrs__ (not operational in group mode); \nUsed only if any server mode is selected.";
	prop_def  = "";
	vect_data.clear();
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Commands";
	prop_desc = "a list of device commands you want to execute through WS\nUsed only if any server mode is selected.";
	prop_def  = "";
	vect_data.clear();
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "PipeName";
	prop_desc = "Name of DevicePipe for reading. [0]\nWhen using GROUP, the DevicePipe name must be the same for all devices.\nIf you want to set properties for specific attributes, add them in the format ``NameAttr;property``\nUsed only if any server mode is selected.";
	prop_def  = "";
	vect_data.clear();
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Secure";
	prop_desc = "Shall we use SSL encryption?\nset true, for secure wss connection, otherwise false;";
	prop_def  = "false";
	vect_data.clear();
	vect_data.push_back("false");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Certificate";
	prop_desc = "full path to the certificate in use (if Secure = true)\nexample: /etc/ssl/certs/ssl-cert-snakeoil.pem";
	prop_def  = "/etc/ssl/certs/server.crt";
	vect_data.clear();
	vect_data.push_back("/etc/ssl/certs/server.crt");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Key";
	prop_desc = "full path to the file in use with Private key (if Secure = true)\nExample: /etc/ssl/private/ssl-cert-snakeoil.key";
	prop_def  = "/etc/ssl/private/server.key";
	vect_data.clear();
	vect_data.push_back("/etc/ssl/private/server.key");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "AuthDS";
	prop_desc = "Tango web authentication device server (TangoWebAuth ) name.\nresponsible for user authentication in case of commands execution";
	prop_def  = "auth/web/1";
	vect_data.clear();
	vect_data.push_back("auth/web/1");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "MaxNumberOfConnections";
	prop_desc = "maximum number of connections. If the limit is reached, further connections will be lost with 400 Bad Request error. If 0 is set, the number of connections will be unlimited.";
	prop_def  = "0";
	vect_data.clear();
	vect_data.push_back("0");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "MaximumBufferSize";
	prop_desc = "maximum buffer size for each connection, KiB. The Default value is 1000. Possible values range from1 to 10000 (if setting a value outside the range, the default value will be set). If exceeding the set maximum buffer size, the connection will be lost by the server;";
	prop_def  = "1000";
	vect_data.clear();
	vect_data.push_back("1000");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "ResetTimestampDifference";
	prop_desc = "The difference in timestamps (seconds) after which a WS server is reset. The difference is counted by CheckPoll method between update timestamp in UpdateData method and current timestamp. Minimum value is 60. \nDefault and MinValue = 60\nUsed only if any server mode is selected";
	prop_def  = "60";
	vect_data.clear();
	vect_data.push_back("60");
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
	prop_name = "Options";
	prop_desc = "Options for device.\nFormat of options:\n	nameOfOption or nameOfOption=value";
	prop_def  = "";
	vect_data.clear();
	if (prop_def.length()>0)
	{
		Tango::DbDatum	data(prop_name);
		data << vect_data ;
		dev_def_prop.push_back(data);
		add_wiz_dev_prop(prop_name, prop_desc,  prop_def);
	}
	else
		add_wiz_dev_prop(prop_name, prop_desc);
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::write_class_property()
 *	Description : Set class description fields as property in database
 */
//--------------------------------------------------------
void WebSocketDSClass::write_class_property()
{
	//	First time, check if database used
	if (Tango::Util::_UseDb == false)
		return;

	Tango::DbData	data;
	string	classname = get_name();
	string	header;
	string::size_type	start, end;

	//	Put title
	Tango::DbDatum	title("ProjectTitle");
	string	str_title("WebSocket access to tango device-server attributes, pipes and commands");
	title << str_title;
	data.push_back(title);

	//	Put Description
	Tango::DbDatum	description("Description");
	vector<string>	str_desc;
	str_desc.push_back("WebSocket access to tango device-server attributes.");
	str_desc.push_back("");
	str_desc.push_back("Configuration should be done via properties:");
	str_desc.push_back("");
	str_desc.push_back("Port - port to listen incoming ws connections;");
	str_desc.push_back("DeviceServer - tango id of a required device server;");
	str_desc.push_back("Attributes - list of required DS attributes, you wish to read via WS;");
	str_desc.push_back("Commands - list of required DS commandes, you wish to executed via WS;");
	str_desc.push_back("AuthDS - Tango web authentication device server (TangoWebAuth ) name.");
	str_desc.push_back("Secure - It will be used wss connection (websocket secure). (true if you want)");
	str_desc.push_back("Certificate - Certificate file name (crt) with full path (if Secure = true)");
	str_desc.push_back("Key - Private key file name (if Secure = true)");
	str_desc.push_back("Options - Various options for the device server");
	str_desc.push_back("");
	str_desc.push_back("Then you should set polling to the UpdateData command. (1000 means that all connected clients would read attributes once per second).");
	str_desc.push_back("");
	str_desc.push_back("Data format: JSON string with array of attrubute objects {atrrtibute name, attribute value, quality, timestamp};");
	str_desc.push_back("");
	str_desc.push_back("if you want to record in the logs, define uselog in Property ``Options``.");
	str_desc.push_back("The database (defined in AuthDS) must contain a table `command_history` with columns:");
	str_desc.push_back("    // id - autoincrement");
	str_desc.push_back("    // argin[0] = timestamp_string UNIX_TIMESTAMP");
	str_desc.push_back("    // argin[1] = login");
	str_desc.push_back("    // argin[2] = deviceName");
	str_desc.push_back("    // argin[3] = IP");
	str_desc.push_back("    // argin[4] = commandName");
	str_desc.push_back("    // argin[5] = commandJson");
	str_desc.push_back("    // argin[6] = statusBool");
	str_desc.push_back("    // argin[7] = isGroup");
	description << str_desc;
	data.push_back(description);

	//	put cvs or svn location
	string	filename("WebSocketDS");
	filename += "Class.cpp";

	// check for cvs information
	string	src_path(CvsPath);
	start = src_path.find("/");
	if (start!=string::npos)
	{
		end   = src_path.find(filename);
		if (end>start)
		{
			string	strloc = src_path.substr(start, end-start);
			//	Check if specific repository
			start = strloc.find("/cvsroot/");
			if (start!=string::npos && start>0)
			{
				string	repository = strloc.substr(0, start);
				if (repository.find("/segfs/")!=string::npos)
					strloc = "ESRF:" + strloc.substr(start, strloc.length()-start);
			}
			Tango::DbDatum	cvs_loc("cvs_location");
			cvs_loc << strloc;
			data.push_back(cvs_loc);
		}
	}

	// check for svn information
	else
	{
		string	src_path(SvnPath);
		start = src_path.find("://");
		if (start!=string::npos)
		{
			end = src_path.find(filename);
			if (end>start)
			{
				header = "$HeadURL: ";
				start = header.length();
				string	strloc = src_path.substr(start, (end-start));
				
				Tango::DbDatum	svn_loc("svn_location");
				svn_loc << strloc;
				data.push_back(svn_loc);
			}
		}
	}

	//	Get CVS or SVN revision tag
	
	// CVS tag
	string	tagname(TagName);
	header = "$Name: ";
	start = header.length();
	string	endstr(" $");
	
	end   = tagname.find(endstr);
	if (end!=string::npos && end>start)
	{
		string	strtag = tagname.substr(start, end-start);
		Tango::DbDatum	cvs_tag("cvs_tag");
		cvs_tag << strtag;
		data.push_back(cvs_tag);
	}
	
	// SVN tag
	string	svnpath(SvnPath);
	header = "$HeadURL: ";
	start = header.length();
	
	end   = svnpath.find(endstr);
	if (end!=string::npos && end>start)
	{
		string	strloc = svnpath.substr(start, end-start);
		
		string tagstr ("/tags/");
		start = strloc.find(tagstr);
		if ( start!=string::npos )
		{
			start = start + tagstr.length();
			end   = strloc.find(filename);
			string	strtag = strloc.substr(start, end-start-1);
			
			Tango::DbDatum	svn_tag("svn_tag");
			svn_tag << strtag;
			data.push_back(svn_tag);
		}
	}

	//	Get URL location
	string	httpServ(HttpServer);
	if (httpServ.length()>0)
	{
		Tango::DbDatum	db_doc_url("doc_url");
		db_doc_url << httpServ;
		data.push_back(db_doc_url);
	}

	//  Put inheritance
	Tango::DbDatum	inher_datum("InheritedFrom");
	vector<string> inheritance;
	inheritance.push_back("TANGO_BASE_CLASS");
	inher_datum << inheritance;
	data.push_back(inher_datum);

	//	Call database and and values
	get_db_class()->put_property(data);
}

//===================================================================
//	Factory methods
//===================================================================

//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::device_factory()
 *	Description : Create the device object(s)
 *                and store them in the device list
 */
//--------------------------------------------------------
void WebSocketDSClass::device_factory(const Tango::DevVarStringArray *devlist_ptr)
{
	/*----- PROTECTED REGION ID(WebSocketDSClass::device_factory_before) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::device_factory_before

	//	Create devices and add it into the device list
	for (unsigned long i=0 ; i<devlist_ptr->length() ; i++)
	{
		cout4 << "Device name : " << (*devlist_ptr)[i].in() << endl;
		device_list.push_back(new WebSocketDS(this, (*devlist_ptr)[i]));
	}

	//	Manage dynamic attributes if any
	erase_dynamic_attributes(devlist_ptr, get_class_attr()->get_attr_list());

	//	Export devices to the outside world
	for (unsigned long i=1 ; i<=devlist_ptr->length() ; i++)
	{
		//	Add dynamic attributes if any
		WebSocketDS *dev = static_cast<WebSocketDS *>(device_list[device_list.size()-i]);
		dev->add_dynamic_attributes();

		//	Check before if database used.
		if ((Tango::Util::_UseDb == true) && (Tango::Util::_FileDb == false))
			export_device(dev);
		else
			export_device(dev, dev->get_name().c_str());
	}

	/*----- PROTECTED REGION ID(WebSocketDSClass::device_factory_after) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::device_factory_after
}
//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::attribute_factory()
 *	Description : Create the attribute object(s)
 *                and store them in the attribute list
 */
//--------------------------------------------------------
void WebSocketDSClass::attribute_factory(vector<Tango::Attr *> &att_list)
{
	/*----- PROTECTED REGION ID(WebSocketDSClass::attribute_factory_before) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::attribute_factory_before
	//	Attribute : JSON
	JSONAttrib	*json = new JSONAttrib();
	Tango::UserDefaultAttrProp	json_prop;
	//	description	not set for JSON
	//	label	not set for JSON
	//	unit	not set for JSON
	//	standard_unit	not set for JSON
	//	display_unit	not set for JSON
	//	format	not set for JSON
	//	max_value	not set for JSON
	//	min_value	not set for JSON
	//	max_alarm	not set for JSON
	//	min_alarm	not set for JSON
	//	max_warning	not set for JSON
	//	min_warning	not set for JSON
	//	delta_t	not set for JSON
	//	delta_val	not set for JSON
	
	json->set_default_properties(json_prop);
	//	Not Polled
	json->set_disp_level(Tango::OPERATOR);
	//	Not Memorized
	att_list.push_back(json);

	//	Attribute : TimestampDiff
	TimestampDiffAttrib	*timestampdiff = new TimestampDiffAttrib();
	Tango::UserDefaultAttrProp	timestampdiff_prop;
	timestampdiff_prop.set_description("The difference between the timestamps from UpdateData and CheckPoll");
	timestampdiff_prop.set_label("TimestampDifference");
	timestampdiff_prop.set_unit("s");
	//	standard_unit	not set for TimestampDiff
	//	display_unit	not set for TimestampDiff
	//	format	not set for TimestampDiff
	//	max_value	not set for TimestampDiff
	//	min_value	not set for TimestampDiff
	//	max_alarm	not set for TimestampDiff
	//	min_alarm	not set for TimestampDiff
	//	max_warning	not set for TimestampDiff
	//	min_warning	not set for TimestampDiff
	//	delta_t	not set for TimestampDiff
	//	delta_val	not set for TimestampDiff
	
	timestampdiff->set_default_properties(timestampdiff_prop);
	//	Not Polled
	timestampdiff->set_disp_level(Tango::OPERATOR);
	//	Not Memorized
	att_list.push_back(timestampdiff);

	//	Attribute : NumberOfConnections
	NumberOfConnectionsAttrib	*numberofconnections = new NumberOfConnectionsAttrib();
	Tango::UserDefaultAttrProp	numberofconnections_prop;
	numberofconnections_prop.set_description("Number of WS clients");
	//	label	not set for NumberOfConnections
	//	unit	not set for NumberOfConnections
	//	standard_unit	not set for NumberOfConnections
	//	display_unit	not set for NumberOfConnections
	//	format	not set for NumberOfConnections
	//	max_value	not set for NumberOfConnections
	//	min_value	not set for NumberOfConnections
	//	max_alarm	not set for NumberOfConnections
	//	min_alarm	not set for NumberOfConnections
	//	max_warning	not set for NumberOfConnections
	//	min_warning	not set for NumberOfConnections
	//	delta_t	not set for NumberOfConnections
	//	delta_val	not set for NumberOfConnections
	
	numberofconnections->set_default_properties(numberofconnections_prop);
	//	Not Polled
	numberofconnections->set_disp_level(Tango::OPERATOR);
	//	Not Memorized
	att_list.push_back(numberofconnections);


	//	Create a list of static attributes
	create_static_attribute_list(get_class_attr()->get_attr_list());
	/*----- PROTECTED REGION ID(WebSocketDSClass::attribute_factory_after) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::attribute_factory_after
}
//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::pipe_factory()
 *	Description : Create the pipe object(s)
 *                and store them in the pipe list
 */
//--------------------------------------------------------
void WebSocketDSClass::pipe_factory()
{
	/*----- PROTECTED REGION ID(WebSocketDSClass::pipe_factory_before) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::pipe_factory_before
	/*----- PROTECTED REGION ID(WebSocketDSClass::pipe_factory_after) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::pipe_factory_after
}
//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::command_factory()
 *	Description : Create the command object(s)
 *                and store them in the command list
 */
//--------------------------------------------------------
void WebSocketDSClass::command_factory()
{
	/*----- PROTECTED REGION ID(WebSocketDSClass::command_factory_before) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::command_factory_before


	//	Command On
	OnClass	*pOnCmd =
		new OnClass("On",
			Tango::DEV_VOID, Tango::DEV_VOID,
			"",
			"",
			Tango::OPERATOR);
	command_list.push_back(pOnCmd);

	//	Command Off
	OffClass	*pOffCmd =
		new OffClass("Off",
			Tango::DEV_VOID, Tango::DEV_VOID,
			"",
			"",
			Tango::OPERATOR);
	command_list.push_back(pOffCmd);

	//	Command UpdateData
	UpdateDataClass	*pUpdateDataCmd =
		new UpdateDataClass("UpdateData",
			Tango::DEV_VOID, Tango::DEV_VOID,
			"",
			"",
			Tango::OPERATOR);
	pUpdateDataCmd->set_polling_period(3000);
	command_list.push_back(pUpdateDataCmd);

	//	Command Reset
	ResetClass	*pResetCmd =
		new ResetClass("Reset",
			Tango::DEV_VOID, Tango::DEV_VOID,
			"",
			"",
			Tango::OPERATOR);
	command_list.push_back(pResetCmd);

	//	Command CheckPoll
	CheckPollClass	*pCheckPollCmd =
		new CheckPollClass("CheckPoll",
			Tango::DEV_VOID, Tango::DEV_VOID,
			"",
			"",
			Tango::OPERATOR);
	pCheckPollCmd->set_polling_period(10000);
	command_list.push_back(pCheckPollCmd);

	/*----- PROTECTED REGION ID(WebSocketDSClass::command_factory_after) ENABLED START -----*/
    
    //    Add your own code
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::command_factory_after
}

//===================================================================
//	Dynamic attributes related methods
//===================================================================

//--------------------------------------------------------
/**
 * method : 		WebSocketDSClass::create_static_attribute_list
 * description : 	Create the a list of static attributes
 *
 * @param	att_list	the ceated attribute list
 */
//--------------------------------------------------------
void WebSocketDSClass::create_static_attribute_list(vector<Tango::Attr *> &att_list)
{
	for (unsigned long i=0 ; i<att_list.size() ; i++)
	{
		string att_name(att_list[i]->get_name());
		transform(att_name.begin(), att_name.end(), att_name.begin(), ::tolower);
		defaultAttList.push_back(att_name);
	}

	cout2 << defaultAttList.size() << " attributes in default list" << endl;

	/*----- PROTECTED REGION ID(WebSocketDSClass::create_static_att_list) ENABLED START -----*/
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::create_static_att_list
}


//--------------------------------------------------------
/**
 * method : 		WebSocketDSClass::erase_dynamic_attributes
 * description : 	delete the dynamic attributes if any.
 *
 * @param	devlist_ptr	the device list pointer
 * @param	list of all attributes
 */
//--------------------------------------------------------
void WebSocketDSClass::erase_dynamic_attributes(const Tango::DevVarStringArray *devlist_ptr, vector<Tango::Attr *> &att_list)
{
	Tango::Util *tg = Tango::Util::instance();

	for (unsigned long i=0 ; i<devlist_ptr->length() ; i++)
	{
		Tango::DeviceImpl *dev_impl = tg->get_device_by_name(((string)(*devlist_ptr)[i]).c_str());
		WebSocketDS *dev = static_cast<WebSocketDS *> (dev_impl);

		vector<Tango::Attribute *> &dev_att_list = dev->get_device_attr()->get_attribute_list();
		vector<Tango::Attribute *>::iterator ite_att;
		for (ite_att=dev_att_list.begin() ; ite_att != dev_att_list.end() ; ++ite_att)
		{
			string att_name((*ite_att)->get_name_lower());
			if ((att_name == "state") || (att_name == "status"))
				continue;
			vector<string>::iterator ite_str = find(defaultAttList.begin(), defaultAttList.end(), att_name);
			if (ite_str == defaultAttList.end())
			{
				cout2 << att_name << " is a UNWANTED dynamic attribute for device " << (*devlist_ptr)[i] << endl;
				Tango::Attribute &att = dev->get_device_attr()->get_attr_by_name(att_name.c_str());
				dev->remove_attribute(att_list[att.get_attr_idx()], true, false);
				--ite_att;
			}
		}
	}
	/*----- PROTECTED REGION ID(WebSocketDSClass::erase_dynamic_attributes) ENABLED START -----*/
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::erase_dynamic_attributes
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDSClass::get_attr_by_name()
 *	Description : returns Tango::Attr * object found by name
 */
//--------------------------------------------------------
Tango::Attr *WebSocketDSClass::get_attr_object_by_name(vector<Tango::Attr *> &att_list, string attname)
{
	vector<Tango::Attr *>::iterator it;
	for (it=att_list.begin() ; it<att_list.end() ; ++it)
		if ((*it)->get_name()==attname)
			return (*it);
	//	Attr does not exist
	return NULL;
}


/*----- PROTECTED REGION ID(WebSocketDSClass::Additional Methods) ENABLED START -----*/

/*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::Additional Methods
} //	namespace
