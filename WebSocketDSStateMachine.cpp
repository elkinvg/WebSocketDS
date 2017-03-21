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

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_TimestampDiff_allowed()
 *	Description : Execution allowed for TimestampDiff attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_TimestampDiff_allowed(TANGO_UNUSED(Tango::AttReqType type))
{

	//	Not any excluded states for TimestampDiff attribute in read access.
	/*----- PROTECTED REGION ID(WebSocketDS::TimestampDiffStateAllowed_READ) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::TimestampDiffStateAllowed_READ
	return true;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_NumberOfConnections_allowed()
 *	Description : Execution allowed for NumberOfConnections attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_NumberOfConnections_allowed(TANGO_UNUSED(Tango::AttReqType type))
{

	//	Not any excluded states for NumberOfConnections attribute in read access.
	/*----- PROTECTED REGION ID(WebSocketDS::NumberOfConnectionsStateAllowed_READ) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::NumberOfConnectionsStateAllowed_READ
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

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_SendCommand_allowed()
 *	Description : Execution allowed for SendCommand attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_SendCommand_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Not any excluded states for SendCommand command.
	/*----- PROTECTED REGION ID(WebSocketDS::SendCommandStateAllowed) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::SendCommandStateAllowed
	return true;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_Reset_allowed()
 *	Description : Execution allowed for Reset attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_Reset_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Not any excluded states for Reset command.
	/*----- PROTECTED REGION ID(WebSocketDS::ResetStateAllowed) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::ResetStateAllowed
	return true;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_CheckPoll_allowed()
 *	Description : Execution allowed for CheckPoll attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_CheckPoll_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Not any excluded states for CheckPoll command.
	/*----- PROTECTED REGION ID(WebSocketDS::CheckPollStateAllowed) ENABLED START -----*/
    if (get_state() == Tango::OFF )
    {
        return false;
    }
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::CheckPollStateAllowed
	return true;
}

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_SendCommandBin_allowed()
 *	Description : Execution allowed for SendCommandBin attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_SendCommandBin_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Not any excluded states for SendCommandBin command.
	/*----- PROTECTED REGION ID(WebSocketDS::SendCommandBinStateAllowed) ENABLED START -----*/
	
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::SendCommandBinStateAllowed
	return true;
}


/*----- PROTECTED REGION ID(WebSocketDS::WebSocketDSStateAllowed.AdditionalMethods) ENABLED START -----*/

//    Additional Methods

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::WebSocketDSStateAllowed.AdditionalMethods

}	//	End of namespace
