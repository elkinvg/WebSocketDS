/*----- PROTECTED REGION ID(WebSocketDSStateMachine.cpp) ENABLED START -----*/
static const char *RcsId = "$Id:  $";
//=============================================================================
//
// file :        WebSocketDSStateMachine.cpp
//
// description : State machine file for the WebSocketDS class
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

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::WebSocketDSStateMachine.cpp

//================================================================
//  States  |  Description
//================================================================
//  ON      |  
//  OFF     |  
//  FAULT   |  


namespace WebSocketDS_ns
{
//=================================================
//		Attributes Allowed Methods
//=================================================

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_JSON_allowed()
 *	Description : Execution allowed for JSON attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_JSON_allowed(TANGO_UNUSED(Tango::AttReqType type))
{

	//	Not any excluded states for JSON attribute in read access.
	/*----- PROTECTED REGION ID(WebSocketDS::JSONStateAllowed_READ) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::JSONStateAllowed_READ
	return true;
}


//=================================================
//		Commands Allowed Methods
//=================================================

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_On_allowed()
 *	Description : Execution allowed for On attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_On_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Compare device state with not allowed states.
	if (get_state()==Tango::ON)
	{
	/*----- PROTECTED REGION ID(WebSocketDS::OnStateAllowed) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::OnStateAllowed
		return false;
	}
	return true;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_Off_allowed()
 *	Description : Execution allowed for Off attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_Off_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Compare device state with not allowed states.
	if (get_state()==Tango::OFF)
	{
	/*----- PROTECTED REGION ID(WebSocketDS::OffStateAllowed) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::OffStateAllowed
		return false;
	}
	return true;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_UpdateData_allowed()
 *	Description : Execution allowed for UpdateData attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_UpdateData_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Compare device state with not allowed states.
	if (get_state()==Tango::OFF ||
		get_state()==Tango::FAULT)
	{
	/*----- PROTECTED REGION ID(WebSocketDS::UpdateDataStateAllowed) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::UpdateDataStateAllowed
		return false;
	}
	return true;
}


/*----- PROTECTED REGION ID(WebSocketDS::WebSocketDSStateAllowed.AdditionalMethods) ENABLED START -----*/

//	Additional Methods

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::WebSocketDSStateAllowed.AdditionalMethods

}	//	End of namespace
