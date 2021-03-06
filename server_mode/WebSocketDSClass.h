/*----- PROTECTED REGION ID(WebSocketDSClass.h) ENABLED START -----*/
//=============================================================================
//
// file :        WebSocketDSClass.h
//
// description : Include for the WebSocketDS root class.
//               This class is the singleton class for
//                the WebSocketDS device class.
//               It contains all properties and methods which the 
//               WebSocketDS requires only once e.g. the commands.
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


#ifndef WebSocketDSClass_H
#define WebSocketDSClass_H

#include <tango.h>
#include <WebSocketDS.h>


/*----- PROTECTED REGION END -----*/	//	WebSocketDSClass.h


namespace WebSocketDS_ns
{
/*----- PROTECTED REGION ID(WebSocketDSClass::classes for dynamic creation) ENABLED START -----*/


/*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::classes for dynamic creation

//=========================================
//	Define classes for commands
//=========================================
//	Command UpdateData class definition
class UpdateDataClass : public Tango::Command
{
public:
	UpdateDataClass(const char   *name,
	               Tango::CmdArgType in,
				   Tango::CmdArgType out,
				   const char        *in_desc,
				   const char        *out_desc,
				   Tango::DispLevel  level)
	:Command(name,in,out,in_desc,out_desc, level)	{};

	UpdateDataClass(const char   *name,
	               Tango::CmdArgType in,
				   Tango::CmdArgType out)
	:Command(name,in,out)	{};
	~UpdateDataClass() {};
	
	virtual CORBA::Any *execute (Tango::DeviceImpl *dev, const CORBA::Any &any);
	virtual bool is_allowed (Tango::DeviceImpl *dev, const CORBA::Any &any)
	{return (static_cast<WebSocketDS *>(dev))->is_UpdateData_allowed(any);}
};


/**
 *	The WebSocketDSClass singleton definition
 */

#ifdef _TG_WINDOWS_
class __declspec(dllexport)  WebSocketDSClass : public Tango::DeviceClass
#else
class WebSocketDSClass : public Tango::DeviceClass
#endif
{
	/*----- PROTECTED REGION ID(WebSocketDSClass::Additionnal DServer data members) ENABLED START -----*/
    
    
    /*----- PROTECTED REGION END -----*/	//	WebSocketDSClass::Additionnal DServer data members

	public:
		//	write class properties data members
		Tango::DbData	cl_prop;
		Tango::DbData	cl_def_prop;
		Tango::DbData	dev_def_prop;
	
		//	Method prototypes
		static WebSocketDSClass *init(const char *);
		static WebSocketDSClass *instance();
		~WebSocketDSClass();
		Tango::DbDatum	get_class_property(string &);
		Tango::DbDatum	get_default_device_property(string &);
		Tango::DbDatum	get_default_class_property(string &);
	
	protected:
		WebSocketDSClass(string &);
		static WebSocketDSClass *_instance;
		void command_factory();
		void attribute_factory(vector<Tango::Attr *> &);
		void pipe_factory();
		void write_class_property();
		void set_default_property();
		void get_class_property();
		string get_cvstag();
		string get_cvsroot();
	
	private:
		void device_factory(const Tango::DevVarStringArray *);
		void create_static_attribute_list(vector<Tango::Attr *> &);
		void erase_dynamic_attributes(const Tango::DevVarStringArray *,vector<Tango::Attr *> &);
		vector<string>	defaultAttList;
		Tango::Attr *get_attr_object_by_name(vector<Tango::Attr *> &att_list, string attname);
};

}	//	End of namespace

#endif   //	WebSocketDS_H
