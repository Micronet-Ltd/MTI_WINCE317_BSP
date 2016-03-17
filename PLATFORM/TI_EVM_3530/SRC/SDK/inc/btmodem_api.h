#ifndef _BTMODEM_API_H_
#define _BTMODEM_API_H_

#include <windows.h>
#include <Bt_api.h>
//==============================================================================================
//		definitions
//==============================================================================================
#define BTMODEM_INQUEUE_NAME		TEXT("BTMODEM_INQUEUE_NAME")
#define BTMODEM_OUTQUEUE_NAME		TEXT("BTMODEM_OUTQUEUE_NAME")

#define BTMODEM_PREFIX			L"BTM1:"

//==============================================================================================
//		errors
//==============================================================================================
#define BTMODEM_ERROR_SUCCESS						0
#define BTMODEM_ERROR_INVALID_PARAMETER				1
#define BTMODEM_ERROR_INVALID_MODEM_NAME			2
#define BTMODEM_ERROR_INVALID_REGISTRY_KEY			3
#define BTMODEM_ERROR_INVALID_REGISTRY_VALUE		4
#define BTMODEM_ERROR_REGISTRY_UPDATING_ERROR		5
#define BTMODEM_ERROR_ACTIVATING_ERROR				6
#define BTMODEM_ERROR_DEACTIVATING_ERROR			7
#define BTMODEM_ERROR_NOT_ACTIVATED					8
#define BTMODEM_ERROR_ALREADY_ACTIVATED				9
#define BTMODEM_ERROR_UNKNOWN_ERROR					10
#define BTMODEM_ERROR_MODEM_ON_FAIL					11
#define BTMODEM_ERROR_MODEM_OFF_FAIL				12
#define BTMODEM_ERROR_GET_MODEM_POWER_FAIL			13
#define BTMODEM_ERROR_MUX_ACTIVATION_FAIL			14
#define BTMODEM_ERROR_MUX_DEACTIVATION_FAIL			15
#define BTMODEM_ERROR_ALREADY_REGISTERED			16
#define BTMODEM_ERROR_REGISTRATION_ERROR			17
#define BTMODEM_ERROR_MODEM_PORT_ERROR				18
#define BTMODEM_ERROR_MODEM_REGISTRATION_ERROR		19
#define BTMODEM_ERROR_DRIVER_ERROR					20
#define BTMODEM_ERROR_NO_BLUETOOTH_STACK_DETECTED	21
#define BTMODEM_ERROR_BLUETOOTH_PORT_ERROR			22
#define BTMODEM_ERROR_TIMEOUT_ERROR					23
#define BTMODEM_ERROR_BLUETOOTH_PORT_NOT_FOUND		24

//==============================================================================================
//		enumerations
//==============================================================================================
typedef enum
{
    BTMODEM_SERVICE_REQUEST = 0,
	BTMODEM_MODEM_STATUS,
	BTMODEM_SERVICE_STATUS
} BTMODEM_MESSAGE_TYPE;

// BTModem service status
enum
{
	BTMODEM_TURNING_ON_MODEM = 0,
	BTMODEM_MODEM_TURNED_ON,
	BTMODEM_WAITING_FOR_SIM_REGISTRATION,
	BTMODEM_SIM_REGISTERED,
	BTMODEM_MUX_ACTIVATION,
	BTMODEM_MUX_ACTIVATED,
	BTMODEM_STARTING_SERVICE,
	BTMODEM_SERVICE_STARTED,
	BTMODEM_STOPPING_SERVICE,
	BTMODEM_SERVICE_STOPPED,
	BTMODEM_MUX_DEACTIVATION,
	BTMODEM_MUX_DEACTIVATED,
	BTMODEM_TURNING_OFF_MODEM,
	BTMODEM_MODEM_TURNED_OFF
};

//==============================================================================================
//     structure definitions
//==============================================================================================

#pragma pack (push, 1)
// BTModem Message
typedef struct
{
	UINT32		Request;
	BT_ADDR		Addr;
} SERVICE_REQUEST;

typedef struct
{
	UINT32		Status;
} MODEM_STATUS;

typedef struct
{
	UINT32		Status;
} SERVICE_STATUS;

typedef union
{
    MODEM_STATUS		ModemStatus;
	SERVICE_REQUEST		ServiceRequest;
	SERVICE_STATUS		ServiceStatus;
	// ....
} BTMODEM_MESSAGE_BODY;

typedef struct
{
    BTMODEM_MESSAGE_TYPE	Type;
	BTMODEM_MESSAGE_BODY	MsgBody;
} BTMODEM_MESSAGE;

#pragma pack (pop)

#ifdef __cplusplus
	extern "C" {
#endif

	//------------------------------------------------------------------------------
	// Function name	: MIC_BTModemActivate
	// Description	    : This function activates Bluetooth DUN profile service.
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : hService: BTModem device handle
	//                  : modemName: Modem registry entry name
    //------------------------------------------------------------------------------
	UINT32 MIC_BTModemActivate(HANDLE hService, TCHAR *modemName);

	//------------------------------------------------------------------------------
	// Function name	: MIC_BTModemDeactivate
	// Description	    : This function deactivates Bluetooth DUN profile service.
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : hService: BTModem device handle
    //------------------------------------------------------------------------------
	UINT32 MIC_BTModemDeactivate(HANDLE hService);

	//------------------------------------------------------------------------------
	// Function name	: MIC_BTModemNotifyRegister
	// Description	    : This function registers Bluetooth DUN profile notifications
	//					: queue
	//                  : 
	// Return type		: 0 if succeeded, error number if failed.
	// Arguments        : hService: BTModem device handle
    //------------------------------------------------------------------------------
	UINT32 MIC_BTModemNotifyRegister(HANDLE hService);

#ifdef __cplusplus
	}
#endif	

#endif //_BTMODEM_API_H_