#ifndef _WIRELESS_API_H_
#define _WIRELESS_API_H_

#include <windows.h>
//==============================================================================================
//		definitions
//==============================================================================================

//==============================================================================================
//		errors
//==============================================================================================
#define WIRELESS_ERROR_SUCCESS						0
#define WIRELESS_ERROR_INVALID_PARAMETER			1
#define WIRELESS_ERROR_ALREADY_ON					2
#define WIRELESS_ERROR_ALREADY_OFF					3
#define WIRELESS_ERROR_INVALID_REGISTRY_KEY			4
#define WIRELESS_ERROR_INVALID_REGISTRY_VALUE		5
#define WIRELESS_ERROR_REGISTRY_UPDATING_ERROR		6
#define WIRELESS_ERROR_UNKNOWN_ERROR				7
#define WIRELESS_ERROR_DRIVER_ERROR					8
#define WIRELESS_ERROR_MODULE_NOT_DETECTED			9
#define WIRELESS_ERROR_TIMEOUT_ERROR				10
#define WIRELESS_ERROR_ACCESS_DENIED				11

//==============================================================================================
//		enumerations
//==============================================================================================

//==============================================================================================
//     structure definitions
//==============================================================================================

#ifdef __cplusplus
	extern "C" {
#endif

	//------------------------------------------------------------------------------
	// Function name	: MIC_WirelessModulePower
	// Description	    : This function turns on/off the wireless module.
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : power: 1 for turning on and 
	//					:		 0 for turning off the module
	//                  : save: 1 for saving the current power state in registry
	//					:		0 = does not save
    //------------------------------------------------------------------------------
	UINT32 MIC_WirelessModulePower(UINT32 power, UINT32 save);

	//------------------------------------------------------------------------------
	// Function name	: MIC_WirelessModuleGetPowerStatus
	// Description	    : This function returns wireless module power status.
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : OUT: power =	1 if module is turned on
	//					:				0 if module is turned off
    //------------------------------------------------------------------------------
	UINT32 MIC_WirelessModuleGetPowerStatus(UINT32* power);

	typedef UINT32 (*PFN_MIC_WiFiEnable)(UINT32 power, UINT32 save);
	typedef UINT32 (*PFN_MIC_BluetoothEnable)(UINT32 power, UINT32 save);
	typedef UINT32 (*PFN_MIC_WiFiGetHWStatus)(UINT32* power);
	typedef UINT32 (*PFN_MIC_BluetoothGetHWStatus)(UINT32* power);

#ifdef __cplusplus
	}
#endif	

#endif //_WIRELESS_API_H_