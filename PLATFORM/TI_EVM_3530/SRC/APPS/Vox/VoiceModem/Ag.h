/** =============================================================================
 *
 *  Copyright (c) 2011 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           H module template
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   7-Apr-2011
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef _AG_H
#define _AG_H

/************** HEADER FILE INCLUDES ******************************************/
#include <windows.h>

/************** GLOBAL CONSTANTS **********************************************/
/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/
typedef enum _AgHsStates
{
	// Ag states
    AG_HS_OFF    = 0,
	AG_HS_ACL,  // Control Connection
	AG_HS_SCO,  // Audio Connection, means that ACL already established

	AG_HS_LAST_STATE

} AG_HS_STATES;

typedef enum _AgHsEvents
{
	AG_HS_BUTTON_PRESS = 0,
	AG_HS_MIC_GAIN,
	AG_HS_SPEAKER_GAIN,
	AG_HS_CTRL,
	AG_HS_AUDIO,

	AG_HS_MAX_SIGNALS

} AG_HS_EVENTS;

/************** FUNCTION PROTOTYPES *******************************************/

#ifdef __cplusplus
	extern "C" {
#endif

 DWORD WINAPI AgThread(LPVOID lpParam);

#ifdef __cplusplus
		}
#endif	

/************** GLOBAL VARIABLES **********************************************/
/************** MACROS ********************************************************/



#endif /* _AG_H */
