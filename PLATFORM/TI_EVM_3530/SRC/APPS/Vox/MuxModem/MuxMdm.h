/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:          H module template
 *  Author(s):       Michael Streshinsky
 *  Creation Date:  6-Nov-2007
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

#ifndef _MUXMDM_H
#define _MUXMDM_H

/************** HEADER FILE INCLUDES ******************************************/
//#include "VoiceModem.h"
#include "Modem.h"

/************** GLOBAL CONSTANTS **********************************************/
/************** STRUCTURES, ENUMS, AND TYPEDEFS *******************************/

typedef enum
{
    MUX_START_ACTIVATING = 0,
    MUX_ACTIVATION_IN_PROGRESS,  // by other Application
	MUX_ACTIVATED,
	MUX_ACTIVATION_ERR,

} MUX_ACTIVATION_ST_e;


/************** FUNCTION PROTOTYPES *******************************************/

#ifdef __cplusplus
	extern "C" {
#endif

MUX_ACTIVATION_ST_e   ActivateMux(void);
BOOL                  testVoicePort(LPCTSTR pszVoicePortName, DWORD timeout);
BOOL                  Mic_GSM710ModemOn(BOOL bNeedRegistration);
void                  SendModemOnToMux(void);
BOOL                  TurnOffMuxModem(void);
BOOL                  InitMux(VMODEM_CONTEXT_s  *pVModemContext);
void                  DeInitMux(void);

#ifdef __cplusplus
		}
#endif	

/************** GLOBAL VARIABLES **********************************************/
/************** MACROS ********************************************************/



#endif /* _MUXMDM_H */
