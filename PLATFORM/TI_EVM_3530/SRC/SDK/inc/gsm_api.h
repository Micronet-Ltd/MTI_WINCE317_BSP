#ifndef _GSM_API_H_
#define _GSM_API_H_

#include <windows.h>



//==============================================================================================
//            local definitions
//==============================================================================================

#define GSM_POWER_ON       1
#define GSM_POWER_OFF      0

//==============================================================================================
//            GSM Port Name Definition
//==============================================================================================
#define GSM_PORT_NAME        L"COM0:"

//==============================================================================================
//            errors definitions
//==============================================================================================

#define GSM_OK                               0

// common errors
#define GSM_ERROR_LOW_BATTERY                 01
#define GSM_HARDWARE_ERROR					  02
#define GSM_ERROR_MODEM_ALREADY_ON			  03
#define GSM_ERROR_MODEM_ALREADY_OFF			  04
#define GSM_DRIVER_ERROR					  10
#define GSM_INTERNAL_ERROR                    10
#define GSM_ERROR_INVALID_PARAMETER           11
#define GSM_ERROR_OPEN_COM                    12
#define GSM_COMPORT_ERROR					  12
#define GSM_ERROR_CLOSE_COM                   13
#define GSM_ERROR_AT_CMD					  14
#define GSM_REGISTRATION_ERROR				  15 
#define GSM_ERROR_NETWORK                     15 
#define GSM_ERROR_CONTROLLED_BY_MUX           16
#define GSM_ERROR_CONTROLLED_BY_BTMODEM       17
#define GSM_ERROR_SIM_NOT_INSERTED            18 
#define GSM_ERROR_ACCESS_DENIED               19


typedef enum
{
    GSM_VOICE_DIAL = 0,
	GSM_VOICE_HANG_UP = 1,
	GSM_VOICE_ANSWER  = 2,

} GSM_VOICE_CMD;

#ifdef __cplusplus
	extern "C" {
#endif

// ================================= General functions====================================

//------------------------------------------------------------------------------
// Function name	: MIC_GSMPower
// Description	    : This function turns on/off the GSM modem.
// Return type		: 0 if succeeded, error number if failed. Use GetLastError() 
//                  : to get detail error.
// Argument         : power – 0 if we want to turn off the Modem and 1 if we want 
//                  : to turn on the modem.
//------------------------------------------------------------------------------
INT32  MIC_GSMPower(INT32 power);



//------------------------------------------------------------------------------
// Function name	: MIC_GSMVoice
// Description	    : This function turn on/off the GSM modem Voice
// Return type		: 0 if succeeded, error number if failed
// Argument  1      : modemVoiceOn = 1 - On, 0 - Off
// Argument  2      : GSMAudioProfile, 0 ... 3, See table below:
//                    --------------------------------- 
//                    |  Number |  Description        |
//                    |   0     |   standard          |
//                    |   1     |   office            | 
//                    |   2     |   open air          |
//                    |   3     |   small /medium car | 
//                    ---------------------------------
// Notes            : Modem should be Off when this function is called.
//                    And any thread listening to Modem Com port should be closed
//                    This function checks response from Modem
//------------------------------------------------------------------------------
INT32  MIC_GSMVoice(HANDLE hCom, INT32 modemVoiceOn, INT32 GSMAudioProfile);


//------------------------------------------------------------------------------
// Function name	: MIC_GSMVoiceCMD
// Description	    : This function implements gsm modem voice commands
// Return type		: 0 if succeeded, error number if failed 
// Argument 1       : GSM_VOICE_CMD voice command
// Argument 2       : string in case of DIAL command
// Notes            : Modem should be On when this function is called.
//                    This function doesn't check the Modem response.
//------------------------------------------------------------------------------
INT32  MIC_GSMVoiceCMD(HANDLE    hCom, GSM_VOICE_CMD voiceCMD, const char *pCmdStr);



//------------------------------------------------------------------------------
// Function name	: MIC_GSMGetPowerStatus
// Description	    : This function returns modem's power status.0 - if modem
//           		: is turned off, 1- if modem is turned on.
// Argument         : OUT: returns 0 - if modem is turned off, 1- if modem is turned on.
// Return type		: 0 if succeeded, error number if failed. Use GetLastError() 
//                  : to get detail error.
//------------------------------------------------------------------------------
INT32  MIC_GSMGetPowerStatus(UINT32* power);

   
//------------------------------------------------------------------------------
// Function name	: MIC_GSMConfig
// Description	    : This function configures the gsm modem.For future use only.
// Return type		: 0 if succeeded, error number if failed 
// Argument         : 
//------------------------------------------------------------------------------
UINT32  MIC_GSMConfig(HANDLE hCom, UINT32 flag);




	#ifdef __cplusplus
		}
	#endif	

#endif //_GSM_API_H_