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

#ifndef _MODEM_H
#define _MODEM_H

/************** HEADER FILE INCLUDES ******************************************/
#include <windows.h>
#include "VoiceModem.h"

/************** GLOBAL CONSTANTS **********************************************/
/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/
typedef enum _ModemStates
{
	// voice modem states
    VMODEM_IDLE    = 0,
    VMODEM_DIALING,
	VMODEM_RINGING,
    VMODEM_CONNECTED,

	VMODEM_LAST_STATE

} VMODEM_STATES;



#define   VMODEM_MAX_STATES  VMODEM_LAST_STATE

typedef enum _ModemEvents
{
	// modem responses events
	VMODEM_CURRENT_CALL = 0,   // "AT+CLCC" - get current call status
    VMODEM_OK,             // "OK"
	VMODEM_RING,           // "RING"
	VMODEM_NO_CARRIER,     // "NO CARRIER"
	VMODEM_ERROR,          // "ERROR"
	VMODEM_NO_DIALTONE,    // "NO DIAL TONE"
	VMODEM_BUSY,           // "BUSY"
	VMODEM_NO_ANSWER,      // "NO ANSWER"

	// modem commands events
	VMODEM_DIAL,
	VMODEM_HANGUP,
	VMODEM_ANSWER,

	// AG HS Events
	//VMODEM_AG_BUTTON_PRESS,
	//VMODEM_AG_MIC_GAIN,
	//VMODEM_AG_SPEAKER_GAIN,
	//VMODEM_AG_CTRL,
	//VMODEM_AG_AUDIO,

	// Modem GUI events
	VMODEM_CALL_KEY,
	VMODEM_END_KEY,

	// Timeout events, add timeout events here, sequentially
	VMODEM_TIMEOUT_EV_0,  // Ringing timeout

	VMODEM_MAX_SIGNALS

} VMODEM_EVENTS;

#define   VMODEM_SIGNALS  VMODEM_MAX_SIGNALS


/************** FUNCTION PROTOTYPES *******************************************/

#ifdef __cplusplus
	extern "C" {
#endif

BOOL   isModemExists(void);
BOOL   modemOn(VMODEM_CONTEXT_s  *pVModemContext);
BOOL   modemOnMux(VMODEM_CONTEXT_s  *pVModemContext);
BOOL   modemOff(VMODEM_CONTEXT_s  *pVModemContext);
void   modemSetAudioProfile(DWORD pfofile);
BOOL   isModemOn(void);
INT32  modemVoiceDial(const char *pCmdStr);
INT32  modemVoiceHangUp(void);
INT32  modemVoiceAnswer(void);

#ifdef __cplusplus
		}
#endif	

/************** GLOBAL VARIABLES **********************************************/
/************** MACROS ********************************************************/



#endif /* _MODEM_H */
