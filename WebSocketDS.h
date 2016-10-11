﻿/*----- PROTECTED REGION ID(WebSocketDS.h) ENABLED START -----*/
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
#ifdef _WIN32
#define WINVER 0x0A00
#endif

//#define _WIN32_WINNT_WIN7 0x0601
//#define _MSC_VER 1600
//#define WINVER 0x0601
//#define _WEBSOCKETPP_CPP11_STRICT_
//#include <websocketpp/config/asio_no_tls.hpp>
//#include <websocketpp/config/asio.hpp>
#include <websocketpp/config/asio.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <websocketpp/common/thread.hpp>
#include "tango_processor.h"

//#define USELOG

//#include <unordered_map>

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

namespace WebSocketDS_ns
{
/*----- PROTECTED REGION ID(WebSocketDS::Additional Class Declarations) ENABLED START -----*/

class WSThread;
class WSThread_tls;
class WSThread_plain;

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::Additional Class Declarations

class WebSocketDS : public TANGO_BASE_CLASS
{

/*----- PROTECTED REGION ID(WebSocketDS::Data Members) ENABLED START -----*/

    //    Add your own data members
private:
    //WSThread *wsThread;
    WSThread *wsThread;
    Tango::DeviceProxy *device;
    //std::unique_ptr<Tango::DeviceProxy> device;
    tango_processor processor;
    std::vector<bool>  isJsonAttribute;

    // тип запроса. Пока только команда. Для атрибута тип не нужен
    enum class TYPE_WS_REQ {COMMAND};


    std::chrono::seconds timeFromUpdateData;
public:
    std::map<std::string, Tango::CommandInfo> accessibleCommandInfo;
    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::Data Members

//	Device property data members
public:
	//	DeviceServer:	Using DeviceServer name
	string	deviceServer;
	//	Port:	Using port of WebSocket
	Tango::DevShort	port;
	//	Attributes:	Attributes list
	vector<string>	attributes;
	//	Commands:	Commandes list from using DS
	vector<string>	commands;
	//	Secure:	Shall we use SSL encryption?
	//  It will be used wss connection (websocket secure)
	Tango::DevBoolean	secure;
	//	Certificate:	Certificate file name (crt) with path
	//  example: /etc/ssl/certs/ssl-cert-snakeoil.pem
	string	certificate;
	//	Key:	Private key file name
	//  Example: /etc/ssl/private/ssl-cert-snakeoil.key
	string	key;
	//	AuthDS:	Tango web authentication device server (TangoWebAuth ) name.
	string	authDS;

//	Attribute data members
public:
	Tango::DevString	*attr_JSON_read;

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
	 *	Command SendCommandToDevice related method
	 *	Description: Command for sending command to device from property.
	 *
	 *	@param argin input argument must be in JSON. Command must be included to device property ``Commands``
	 *               {``command`` : ``nameOfCommand``, ``argin`` : [``1``,``2``,``3``]}
	 *               OR
	 *               {``command`` : ``nameOfCommand``, ``argin`` : ``1``}
	 *	@returns Output in JSON.
	 */
	virtual Tango::DevString send_command_to_device(Tango::DevString argin);
	virtual bool is_SendCommandToDevice_allowed(const CORBA::Any &any);
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
    void initAttrAndComm();
    void fromException(Tango::DevFailed &e, string func);

    bool initDeviceServer();
    bool initWsThread();

#ifdef TESTFAIL
    void sendLogToFile();
#endif
    // for getting attribute's configuration
    void gettingAttrUserConf(string&);
    // exception for command
    string exceptionStringOut(string id, string commandName, string errorMessage, TYPE_WS_REQ type_req);
    // exception for attribute
    string exceptionStringOut(string errorMessage);

    /*----- PROTECTED REGION END -----*/	//	WebSocketDS::Additional Method prototypes
};

/*----- PROTECTED REGION ID(WebSocketDS::Additional Classes Definitions) ENABLED START -----*/

//    Additional Classes Definitions

class UserControl: public Tango::LogAdapter
{
public:
    UserControl(WebSocketDS *dev):
        Tango::LogAdapter(dev)
    {
        ds = dev;
    }
    ~UserControl(){};

    bool check_permission(map<string, string>& parsedGet, Tango::DevString commandJson);
    bool check_user(map<string, string>& parsedGet);
private:
    WebSocketDS *ds;
#ifdef USELOG
    bool sendLogCommand(vector <string> toLogData, Tango::DeviceProxy *authProxy);
#endif
    string getCommandName(const string& commandJson);

};

typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::server<websocketpp::config::asio_tls> server_tls;

class WSThread: public omni_thread, public Tango::LogAdapter
{
public:
    WSThread(WebSocketDS *dev/*, std::string hostName*/,int portNumber): omni_thread(), Tango::LogAdapter(dev)
    {
        //host = hostName;
        port = portNumber;
        ds = dev;
        uc = unique_ptr<UserControl>(new UserControl(dev));
    }
    virtual ~WSThread();

    virtual void *run_undetached(void *) = 0;

    virtual  void stop() = 0;
    virtual void send_all(std::string msg) = 0;
    virtual void send(websocketpp::connection_hdl hdl, std::string msg) = 0;
protected:
    std::string cache;
    virtual bool on_validate(websocketpp::connection_hdl hdl) = 0;
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    //test
    void  on_fail(websocketpp::connection_hdl);

    string parseOfAddress(string addrFromConn); // parsing of get_remote_endpoint-return
    // remoteEndpoint in websocket output formate[::ffff:127.0.0.1 : 11111]

    map<string, string> parseOfGetQuery(string query);

    virtual map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) = 0;

    bool forValidate(map<string, string> remoteConf);
    int port;

    typedef std::set<websocketpp::connection_hdl,std::owner_less<websocketpp::connection_hdl> > con_list;
    con_list m_connections;
    unique_ptr<UserControl> uc;
    
private:
    vector<string> &split(const string &s, char delim, vector<string> &elems);
    vector<string> split(const string &s, char delim);

    websocketpp::lib::mutex m_action_lock;
    websocketpp::lib::mutex m_connection_lock;
    websocketpp::lib::condition_variable m_action_cond;
    bool local_th_exit;

    WebSocketDS *ds;

//    std::string host;

};

class WSThread_plain: public WSThread
{
public:
    WSThread_plain(WebSocketDS *dev/*, std::string hostName*/,int portNumber):
        WSThread(dev/*,hostName*/,portNumber)
    {
        start_undetached();
    }

    virtual ~WSThread_plain();
    virtual void *run_undetached(void *) override;
    virtual void stop() override;
    virtual void send_all(std::string msg) override;
    virtual void send(websocketpp::connection_hdl hdl, std::string msg) override;
    virtual bool on_validate(websocketpp::connection_hdl hdl) override;
private:
    virtual map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) override;
    server m_server;
};



class WSThread_tls: public WSThread
{
public:
    WSThread_tls(WebSocketDS *dev/*, std::string hostName*/,int portNumber, string cert, string key):
        WSThread(dev/*,hostName*/,portNumber)
    {
        certificate_ = cert;
        key_ = key;
        start_undetached();
    }

    virtual ~WSThread_tls();
    virtual void *run_undetached(void *) override;

    virtual  void stop() override;
    virtual void send_all(std::string msg) override;
    virtual void send(websocketpp::connection_hdl hdl, std::string msg) override;
    virtual bool on_validate(websocketpp::connection_hdl hdl) override;
private:
    context_ptr on_tls_init(websocketpp::connection_hdl hdl);
    std::string get_password();
    virtual map<string, string> getRemoteConf(websocketpp::connection_hdl hdl) override;

    server_tls m_server;

    std::string certificate_;
    std::string key_;
};

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::Additional Classes Definitions

}	//	End of namespace

#endif   //	WebSocketDS_H
