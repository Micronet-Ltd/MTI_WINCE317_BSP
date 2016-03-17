#ifndef _GSM710_API_H_
#define _GSM710_API_H_

#include <windows.h>



//==============================================================================================
//            definitions
//==============================================================================================

#define GSM710_INQUEUE_NAME		TEXT("GSM710_INQUEUE_NAME")
#define GSM710_OUTQUEUE_NAME	TEXT("GSM710_OUTQUEUE_NAME")
#define DRIVER_MUX07_10_REG_KEY1 (TEXT("Drivers\\GSM0710_1"))
#define DRIVER_MUX07_10_REG_KEY2 (TEXT("Drivers\\GSM0710_2"))

//==============================================================================================
//            errors definitions
//==============================================================================================

// OK
#define GSM710ERROR_SUCCESS						0		

// Errors
#define GSM710ERROR_INVALID_REGISTRY_KEY		10
#define GSM710ERROR_INVALID_REGISTRY_VALUE		11
#define GSM710ERROR_REGISTRY_UPDATING_ERROR		12
#define GSM710ERROR_SYSTEM_FAILED				13
#define GSM710ERROR_COMPORT_OPEN_FAILED			14
#define GSM710ERROR_COMPORT_CONFIG_FAILED		15
#define GSM710ERROR_MODEM_NOT_RESPONDING		16
#define GSM710ERROR_ALREADY_ACTIVATED			17
#define GSM710ERROR_ALREADY_DEACTIVATED			18
#define GSM710ERROR_ACTIVATION_FAILED			19
#define GSM710ERROR_DRIVER_NOT_RESPONDING		20
#define GSM710ERROR_DRIVER_ERROR				21
#define GSM710ERROR_INVALID_PARAMETER			22

//==============================================================================================
//     enumerations
//==============================================================================================

typedef enum
{
    GSM710MESSAGETYPE_CONNECTIONSTATE = 0,
	GSM710MESSAGETYPE_MODEMSTATE = 1,
	GSM710MESSAGETYPE_POWERSTATE = 2,
	GSM710MESSAGETYPE_CONNECTIONERROR = 3
} GSM710MESSAGE_TYPE;

//==============================================================================================
//     structure definitions
//==============================================================================================

// GSM710 Message
typedef struct
{
    GSM710MESSAGE_TYPE	Type;
	UINT32				Value;
} GSM710MESSAGE;

// ================================= General functions====================================

#ifdef __cplusplus
	extern "C" {
#endif

	//------------------------------------------------------------------------------
	// Function name	: Mic_GSM710Activate
	// Description	    : This function activates / deactivates MUX driver.
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : Activate: 1 = activates MUX, 0 = deactivates
	//                  : Default: 1 = update current activation state in registry, 
	//					:		   0 = do not update
	//					: Note: The second argument is ignored, if the registry parameter
	//					:		"HKLM\Drivers\BuiltIn\GPRS\EnableExternalModem" is set to 1
	//					:
    // Examples         : Mic_GSM710Activate(1, 0) - activate MUX 
	//					: Mic_GSM710Activate(1, 1) - activate MUX and update in registry
    //------------------------------------------------------------------------------
	INT32 MIC_GSM710Activate(INT32 Activate, INT32 Default);

#ifdef __cplusplus
	}
#endif	

#endif //_GSM710_API_H_