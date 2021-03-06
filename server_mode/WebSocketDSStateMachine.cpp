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
//  FAULT   |  


namespace WebSocketDS_ns
{
//=================================================
//		Attributes Allowed Methods
//=================================================


//=================================================
//		Commands Allowed Methods
//=================================================

//--------------------------------------------------------
/**
 *	Method      : WebSocketDS::is_UpdateData_allowed()
 *	Description : Execution allowed for UpdateData attribute
 */
//--------------------------------------------------------
bool WebSocketDS::is_UpdateData_allowed(TANGO_UNUSED(const CORBA::Any &any))
{
	//	Not any excluded states for UpdateData command.
	/*----- PROTECTED REGION ID(WebSocketDS::UpdateDataStateAllowed) ENABLED START -----*/
    if (wsTangoConn == nullptr) {
        return false;
    }
	/*----- PROTECTED REGION END -----*/	//	WebSocketDS::UpdateDataStateAllowed
	return true;
}


/*----- PROTECTED REGION ID(WebSocketDS::WebSocketDSStateAllowed.AdditionalMethods) ENABLED START -----*/

//    Additional Methods

/*----- PROTECTED REGION END -----*/	//	WebSocketDS::WebSocketDSStateAllowed.AdditionalMethods

}	//	End of namespace
