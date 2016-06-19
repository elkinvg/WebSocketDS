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



/*----- PROTECTED REGION END -----*/    //    WebSocketDS.cpp

/**
 *  WebSocketDS class description:
 *    WebSocket access to tango device-server attributes.
 *
 *    Configuration should be done via properties:
 *
 *    Port - port to listen incoming ws connections;
 *    DeviceServer - tango id of a required device server;
 *    Attributes - list of required DS attributes, you wish to read via WS;
 *    Then you should set polling to the UpdateData command. (1000 means that all connected clients would read attributes once per second).
 *
 *    Data format: JSON string with array of attrubute objects {atrrtibute name, attribute value, quality, timestamp};
 */

//================================================================
//  The following table gives the correspondence
//  between command and method names.
//
//  Command name         |  Method name
//================================================================
//  State                |  Inherited (no method)
//  Status               |  Inherited (no method)
//  On                   |  on
//  Off                  |  off
//  UpdateData           |  update_data
//  SendCommandToDevice  |  send_command_to_device
//================================================================

//================================================================
//  Attributes managed is:
//================================================================
//  JSON  |  Tango::DevString    Scalar
//================================================================

namespace WebSocketDS_ns
{
    /*----- PROTECTED REGION ID(WebSocketDS::namespace_starting) ENABLED START -----*/

    //    static initializations

    /*----- PROTECTED REGION END -----*/    //    WebSocketDS::namespace_starting

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::WebSocketDS()
     *    Description : Constructors for a Tango device
     *                implementing the classWebSocketDS
     */
    //--------------------------------------------------------
    WebSocketDS::WebSocketDS(Tango::DeviceClass *cl, string &s)
        : TANGO_BASE_CLASS(cl, s.c_str())
    {
        /*----- PROTECTED REGION ID(WebSocketDS::constructor_1) ENABLED START -----*/
        init_device();

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::constructor_1
    }
    //--------------------------------------------------------
    WebSocketDS::WebSocketDS(Tango::DeviceClass *cl, const char *s)
        : TANGO_BASE_CLASS(cl, s)
    {
        /*----- PROTECTED REGION ID(WebSocketDS::constructor_2) ENABLED START -----*/
        init_device();

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::constructor_2
    }
    //--------------------------------------------------------
    WebSocketDS::WebSocketDS(Tango::DeviceClass *cl, const char *s, const char *d)
        : TANGO_BASE_CLASS(cl, s, d)
    {
        /*----- PROTECTED REGION ID(WebSocketDS::constructor_3) ENABLED START -----*/
        init_device();

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::constructor_3
    }

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::delete_device()
     *    Description : will be called at device destruction or at init command
     */
    //--------------------------------------------------------
    void WebSocketDS::delete_device()
    {
        DEBUG_STREAM << "WebSocketDS::delete_device() " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::delete_device) ENABLED START -----*/

        //    Delete device allocated objects
        wsThread->stop();
        void *ptr;
        DEBUG_STREAM << "Waiting for the thread to exit" << endl;
        wsThread->join(&ptr);

        CORBA::string_free(*attr_JSON_read);

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::delete_device
        delete[] attr_JSON_read;
    }

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::init_device()
     *    Description : will be called at device initialization.
     */
    //--------------------------------------------------------
    void WebSocketDS::init_device()
    {
        DEBUG_STREAM << "WebSocketDS::init_device() create device " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::init_device_before) ENABLED START -----*/

        //    Initialization before get_device_property() call

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::init_device_before


        //    Get the device properties from database
        get_device_property();

        attr_JSON_read = new Tango::DevString[1];
        /*----- PROTECTED REGION ID(WebSocketDS::init_device) ENABLED START -----*/

        try
        {
            wsThread = new WSThread(this, "", port);
            set_state(Tango::ON);
            set_status("Device is On");
            device = new Tango::DeviceProxy(deviceServer);
        }
        catch (Tango::DevFailed &e)
        {
            Tango::Except::print_exception(e);
            set_state(Tango::FAULT);
            set_status("Couldn't connect to device: " + deviceServer);
        }


        DEBUG_STREAM << "Attributes: " << endl;
        for (int i = 0; i < attributes.size(); i++)
        {
            std::string at = attributes.at(i);
            boost::to_lower(at);
            isJsonAttribute.push_back(at.find("json") != std::string::npos);
            DEBUG_STREAM << attributes.at(i) << endl;
        }

        DEBUG_STREAM << "Commands: " << endl;
        for (auto& com : commands) {
            try {
                Tango::CommandInfo info = device->command_query(com);
                accessibleCommandInfo.insert(std::pair<std::string, Tango::CommandInfo>(com, info));
                DEBUG_STREAM << com << endl;
            }
            catch (Tango::DevFailed &e)
            {
                ERROR_STREAM << "command " << com << " not found" << endl;
            }
        }

        attr_JSON_read[0] = Tango::string_dup("{\"success\": false}");
        update_data();
        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::init_device
    }

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::get_device_property()
     *    Description : Read database to initialize property data members.
     */
    //--------------------------------------------------------
    void WebSocketDS::get_device_property()
    {
        /*----- PROTECTED REGION ID(WebSocketDS::get_device_property_before) ENABLED START -----*/

        //    Initialize property data members
        port = 9002;

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::get_device_property_before


        //    Read device properties from database.
        Tango::DbData    dev_prop;
        dev_prop.push_back(Tango::DbDatum("Port"));
        dev_prop.push_back(Tango::DbDatum("DeviceServer"));
        dev_prop.push_back(Tango::DbDatum("Attributes"));
        dev_prop.push_back(Tango::DbDatum("Commands"));

        //    is there at least one property to be read ?
        if (dev_prop.size() > 0)
        {
            //    Call database and extract values
            if (Tango::Util::instance()->_UseDb == true)
                get_db_device()->get_property(dev_prop);

            //    get instance on WebSocketDSClass to get class property
            Tango::DbDatum    def_prop, cl_prop;
            WebSocketDSClass    *ds_class =
                (static_cast<WebSocketDSClass *>(get_device_class()));
            int    i = -1;

            //    Try to initialize Port from class property
            cl_prop = ds_class->get_class_property(dev_prop[++i].name);
            if (cl_prop.is_empty() == false)    cl_prop >> port;
            else {
                //    Try to initialize Port from default device value
                def_prop = ds_class->get_default_device_property(dev_prop[i].name);
                if (def_prop.is_empty() == false)    def_prop >> port;
            }
            //    And try to extract Port value from database
            if (dev_prop[i].is_empty() == false)    dev_prop[i] >> port;

            //    Try to initialize DeviceServer from class property
            cl_prop = ds_class->get_class_property(dev_prop[++i].name);
            if (cl_prop.is_empty() == false)    cl_prop >> deviceServer;
            else {
                //    Try to initialize DeviceServer from default device value
                def_prop = ds_class->get_default_device_property(dev_prop[i].name);
                if (def_prop.is_empty() == false)    def_prop >> deviceServer;
            }
            //    And try to extract DeviceServer value from database
            if (dev_prop[i].is_empty() == false)    dev_prop[i] >> deviceServer;

            //    Try to initialize Attributes from class property
            cl_prop = ds_class->get_class_property(dev_prop[++i].name);
            if (cl_prop.is_empty() == false)    cl_prop >> attributes;
            else {
                //    Try to initialize Attributes from default device value
                def_prop = ds_class->get_default_device_property(dev_prop[i].name);
                if (def_prop.is_empty() == false)    def_prop >> attributes;
            }
            //    And try to extract Attributes value from database
            if (dev_prop[i].is_empty() == false)    dev_prop[i] >> attributes;

            //    Try to initialize Commands from class property
            cl_prop = ds_class->get_class_property(dev_prop[++i].name);
            if (cl_prop.is_empty() == false)    cl_prop >> commands;
            else {
                //    Try to initialize Commands from default device value
                def_prop = ds_class->get_default_device_property(dev_prop[i].name);
                if (def_prop.is_empty() == false)    def_prop >> commands;
            }
            //    And try to extract Commands value from database
            if (dev_prop[i].is_empty() == false)    dev_prop[i] >> commands;

        }

        /*----- PROTECTED REGION ID(WebSocketDS::get_device_property_after) ENABLED START -----*/

        //    Check device property data members init

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::get_device_property_after
    }

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::always_executed_hook()
     *    Description : method always executed before any command is executed
     */
    //--------------------------------------------------------
    void WebSocketDS::always_executed_hook()
    {
        //DEBUG_STREAM << "WebSocketDS::always_executed_hook()  " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::always_executed_hook) ENABLED START -----*/

        //    code always executed before all requests

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::always_executed_hook
    }

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::read_attr_hardware()
     *    Description : Hardware acquisition for attributes
     */
    //--------------------------------------------------------
    void WebSocketDS::read_attr_hardware(TANGO_UNUSED(vector<long> &attr_list))
    {
        DEBUG_STREAM << "WebSocketDS::read_attr_hardware(vector<long> &attr_list) entering... " << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::read_attr_hardware) ENABLED START -----*/

        //    Add your own code

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::read_attr_hardware
    }

    //--------------------------------------------------------
    /**
     *    Read attribute JSON related method
     *    Description:
     *
     *    Data type:    Tango::DevString
     *    Attr type:    Scalar
     */
    //--------------------------------------------------------
    void WebSocketDS::read_JSON(Tango::Attribute &attr)
    {
        DEBUG_STREAM << "WebSocketDS::read_JSON(Tango::Attribute &attr) entering... " << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::read_JSON) ENABLED START -----*/
        //    Set the attribute value
        attr.set_value(attr_JSON_read);

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::read_JSON
    }

    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::add_dynamic_attributes()
     *    Description : Create the dynamic attributes if any
     *                for specified device.
     */
    //--------------------------------------------------------
    void WebSocketDS::add_dynamic_attributes()
    {
        /*----- PROTECTED REGION ID(WebSocketDS::add_dynamic_attributes) ENABLED START -----*/

        //    Add your own code to create and add dynamic attributes if any

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::add_dynamic_attributes
    }

    //--------------------------------------------------------
    /**
     *    Command On related method
     *    Description:
     *
     */
    //--------------------------------------------------------
    void WebSocketDS::on()
    {
        DEBUG_STREAM << "WebSocketDS::On()  - " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::on) ENABLED START -----*/

        set_state(Tango::ON);
        set_status("Device is On");


        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::on
    }
    //--------------------------------------------------------
    /**
     *    Command Off related method
     *    Description:
     *
     */
    //--------------------------------------------------------
    void WebSocketDS::off()
    {
        DEBUG_STREAM << "WebSocketDS::Off()  - " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::off) ENABLED START -----*/

        set_state(Tango::OFF);
        set_status("Device is Off");

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::off
    }
    //--------------------------------------------------------
    /**
     *    Command UpdateData related method
     *    Description:
     *
     */
    //--------------------------------------------------------
    void WebSocketDS::update_data()
    {
        DEBUG_STREAM << "WebSocketDS::UpdateData()  - " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::update_data) ENABLED START -----*/
        //double r = rand_float(0,100);
        //attr_MyAttr_read[0] = r;


        try
        {
            std::vector<Tango::DeviceAttribute> *attrList = device->read_attributes(attributes);
            std::stringstream json;
            json << "[";
            for (int i = 0; i < attributes.size(); i++)
            {
                if (i != 0) json << ", ";
                Tango::DeviceAttribute att = attrList->at(i);
                if (isJsonAttribute.at(i))
                    json << processor.process_device_attribute_json(att);
                else
                    //json << processor.process_attribute(att);
                    json << processor.process_attribute_t(att);
            }
            json << "]";
            delete attrList;
            CORBA::string_free(*attr_JSON_read);
            attr_JSON_read[0] = Tango::string_dup(json.str().c_str());
            wsThread->send_all(json.str().c_str());
        }
        catch (Tango::DevFailed &e)
        {
            Tango::Except::print_exception(e);
            set_state(Tango::FAULT);
            set_status("Couldn't read attribute from device: " + deviceServer);
        }
        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::update_data
    }
    //--------------------------------------------------------
    /**
     *    Command SendCommandToDevice related method
     *    Description: Command for sending command to device from property.
     *
     *    @param argin input argument must be in JSON. Command must be included to device property ``Commands``
     *               {``command`` : ``nameOfCommand``, ``argin`` : [``1``,``2``,``3``]}
     *               OR
     *               {``command`` : ``nameOfCommand``, ``argin`` : ``1``}
     *    @returns Output in JSON.
     */
    //--------------------------------------------------------
    Tango::DevString WebSocketDS::send_command_to_device(Tango::DevString argin)
    {
        Tango::DevString argout;
        DEBUG_STREAM << "WebSocketDS::SendCommandToDevice()  - " << device_name << endl;
        /*----- PROTECTED REGION ID(WebSocketDS::send_command_to_device) ENABLED START -----*/

        //    Add your own code
        try
        {
            /// ??? check if command.size() = 0
            //            string command = processor.getCommandName(argin);
            std::map<std::string, std::string> jsonArgs = processor.getCommandName(argin);

            if (jsonArgs.find("error") != jsonArgs.end()) {
                std::string tmp = "{\"error\":";
                tmp += "\"" + jsonArgs["error"] + "\"}";
                return CORBA::string_dup(tmp.c_str());
            }

            if (jsonArgs["argin"].size() == 0)
                return "{\"error\": \"argin invalid\"}";


            if (jsonArgs["command"] == processor.NONE)
                return "{\"error\": \"String command not found\"}";
            if (jsonArgs["argin"] == processor.NONE)
                return "{\"error\": \"argin not found\"}";
            



            bool isCommandAccessible = processor.checkCommand(jsonArgs["command"], accessibleCommandInfo);

            if (isCommandAccessible) {
                Tango::CommandInfo comInfo = accessibleCommandInfo[jsonArgs["command"]];
                int type = comInfo.in_type;
                Tango::DeviceData out;

                if (type == Tango::DEV_VOID)
                    out = device->command_inout(jsonArgs["command"]);
                else {
                    if (jsonArgs["argin"] == "Array") {
                        if (!processor.isMassive(type)) return "{\"error\": \"The input data do not have to be an array\"}";
                    }
                    Tango::DeviceData deviceData = processor.gettingDevDataFromJsonStr(argin, type);

                    /// ??? check string(argin)
                    /// ??? check if wrong input data
                    try {
                        out = device->command_inout(jsonArgs["command"], deviceData);
                    }
                    catch (Tango::DevFailed &e) {
                        argout = CORBA::string_dup("{\"error\": \"Exception from command_inout. Check the format of the data\"}");
                        return argout;
                    }
                }

                string fromDevData = processor.gettingJsonStrFromDevData(out, jsonArgs);
                argout = CORBA::string_dup(fromDevData.c_str());
            }
            else {
                argout = CORBA::string_dup("{\"error\": \"Command not found on DeviceServer\"}");
            }
        }
        catch (Tango::DevFailed &e) {
            Tango::Except::print_exception(e);
            std::string exc;
            exc = "{\"error\":" + (std::string)"\"" + (string)(e.errors[0].desc) + "\"}";
            //CORBA::string_dup(e.errors[0].desc);
            //argout = CORBA::string_dup("{\"error\": \"Exception from send_command_to_device\"}");
            argout = CORBA::string_dup(exc.c_str());
            return argout;
            // ADD MESSAGE ???
        }

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::send_command_to_device
        return argout;
    }
    //--------------------------------------------------------
    /**
     *    Method      : WebSocketDS::add_dynamic_commands()
     *    Description : Create the dynamic commands if any
     *                for specified device.
     */
    //--------------------------------------------------------
    void WebSocketDS::add_dynamic_commands()
    {
        /*----- PROTECTED REGION ID(WebSocketDS::add_dynamic_commands) ENABLED START -----*/

        //    Add your own code to create and add dynamic commands if any

        /*----- PROTECTED REGION END -----*/    //    WebSocketDS::add_dynamic_commands
    }

    /*----- PROTECTED REGION ID(WebSocketDS::namespace_ending) ENABLED START -----*/
    //void WebSocketDS::testFromJson(std::string& input) {
    //    boost::property_tree::ptree pt;
    //    std::stringstream ss;
    //    ss << input;
    //    boost::property_tree::read_json(ss, pt);
    //    //pt.get_child("argin");
    //    //auto tmp1 = pt.get<std::string>("argin");
    //    for (boost::property_tree::ptree::value_type &v : pt.get_child("argin"))
    //    {
    //        // fruit.first contain the string ""
    //        cout << "SD: " << v.second.data() << " | ";
    //        //fruits.push_back(fruit.second.data());
    //    }
    //    cout << endl;
    //    auto ii = device->command_list_query();
    //    auto aa = device->command_query("DevVarShortArray");
    //    //vector<short> parsed = pt.get<short>("argin");
    //    auto ttt = pt;
    //}
    // //--------------------------------------------------------
    // /**
    //  *    Read attribute MyAttr related method
    //  *    Description:
    //  *
    //  *    Data type:    Tango::DevDouble
    //  *    Attr type:    Scalar
    //  */
    // //--------------------------------------------------------
    // void WebSocketDS::read_MyAttr(Tango::Attribute &attr)
    // {
    //     DEBUG_STREAM << "WebSocketDS::read_MyAttr(Tango::Attribute &attr) entering... " << endl;
    //     //    Set the attribute value
    //     attr.set_value(attr_MyAttr_read);
    //
    // }

    // //--------------------------------------------------------
    // /**
    //  *    Read attribute JSONCharts related method
    //  *    Description:
    //  *
    //  *    Data type:    Tango::DevString
    //  *    Attr type:    Scalar
    //  */
    // //--------------------------------------------------------
    // void WebSocketDS::read_JSONCharts(Tango::Attribute &attr)
    // {
    //     DEBUG_STREAM << "WebSocketDS::read_JSONCharts(Tango::Attribute &attr) entering... " << endl;
    //     //    Set the attribute value
    //     attr.set_value(attr_JSONCharts_read);
    //
    // }


    /*----- PROTECTED REGION END -----*/    //    WebSocketDS::namespace_ending
} //    namespace
