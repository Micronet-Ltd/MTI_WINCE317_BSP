#ifndef _GPS_API_H
#define _GPS_API_H

#define GPS_PORT_NAME          L"COM7:"

#ifdef __cplusplus
	extern "C" {
#endif
//------------------------------------------------------------------------------
// Function name	: MIC_GPSPower
// Description	    : //GPS power status On/Off 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments        : power status 
//--------------------------------------------
INT32 MIC_GPSPower(INT32 power);

//--------------------------------------------
// Function name	: MIC_GPSGetPowerStatus
// Description	    : //get GPS power status
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments        : pointer topower status 
//-------------------------------------------
INT32 MIC_GPSGetPowerStatus(INT32* pPower);
//--------------------------------------------
#ifdef __cplusplus
	}
#endif	

#endif
