#ifndef _TEMPSENSOR_API_H_
#define _TEMPSENSOR_API_H_

#include <windows.h>
//==============================================================================================
//		definitions
//==============================================================================================

//==============================================================================================
//		errors
//==============================================================================================
#define TEMPSENSOR_OK						0
#define TEMPSENSOR_ERROR_INVALID_PARAMETER	1
#define TEMPSENSOR_ERROR_DRIVER_ERROR		2
#define TEMPSENSOR_ERROR_BUFFER_OVERFLOW	3
#define TEMPSENSOR_ERROR_ALREADY_REGISTERED	4
#define TEMPSENSOR_ERROR_NOT_REGISTERED		5
#define TEMPSENSOR_ERROR_QUEUE_REGISTERED	6

//==============================================================================================
//		enumerations
//==============================================================================================

//==============================================================================================
//     structure definitions
//==============================================================================================

#pragma pack (push, 1)

// Notification message
typedef struct
{
    INT16	CurrentTemperature;
	INT16	LimiterTemperature;
} TEMPSENSOR_DATA;

#pragma pack (pop)

#ifdef __cplusplus
	extern "C" {
#endif

	//------------------------------------------------------------------------------
	// Function name	: MIC_TemperatureSensorOpen
	// Description	    : This function opens temperature sensor device
	//                  : 
	// Return type		: Valid handle if succeeded, INVALID_HANDLE_VALUE if failed.
	//					: To obtain extended error information, call the 
	//					: GetLastError function
	// Arguments        : None
    //------------------------------------------------------------------------------
	HANDLE MIC_TemperatureSensorOpen();

	//------------------------------------------------------------------------------
	// Function name	: MIC_TemperatureSensorClose
	// Description	    : This function closes temperature sensor device
	//                  : 
	// Return type		: Returns TRUE if the close succeeds; otherwise, returns FALSE.
	//					: To obtain extended error information, call the 
	//					: GetLastError function
	// Arguments        : hTempSensor: Handle to an open temperature sensor device 
    //------------------------------------------------------------------------------
	BOOL MIC_TemperatureSensorClose(HANDLE hTempSensor);

	//------------------------------------------------------------------------------
	// Function name	: MIC_TemperatureSensorRegister
	// Description	    : This function registers temperature sensor notifications
	//					: queue
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : hTempSensor: Handle to an open temperature sensor device
	//					: pQueueName[out]: Pointer to a buffer containing queue name
	//					: pOutBufferLen [in, out]: Pointer to the size, in bytes, 
	//					: of the queue name parameter. If this size is insufficient
	//					: to hold the queue name, this function updates this 
	//					: parameter with the required size, and returns an error 
	//					: code of TEMPSENSOR_ERROR_BUFFER_OVERFLOW. 
    //------------------------------------------------------------------------------
	UINT32 MIC_TemperatureSensorRegister(HANDLE hTempSensor, TCHAR *pQueueName, UINT32 *pOutBufferLen);

	//------------------------------------------------------------------------------
	// Function name	: MIC_TemperatureSensorDeregister
	// Description	    : This function deregisters temperature sensor notifications
	//					: queue
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : hTempSensor: Handle to an open temperature sensor device
    //------------------------------------------------------------------------------
	UINT32 MIC_TemperatureSensorDeregister(HANDLE hTempSensor);

#ifdef __cplusplus
	}
#endif	

#endif //_TEMPSENSOR_API_H_