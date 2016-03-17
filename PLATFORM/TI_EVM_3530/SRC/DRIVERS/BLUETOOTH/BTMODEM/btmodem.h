#ifndef _BTMODEM_H_
#define _BTMODEM_H_

#include <windows.h>
//==============================================================================================
//		definitions
//==============================================================================================
#define MUX_UP_MAX_TIMEOUT				5000
#define REGISTRATION_MAX_TIMEOUT		30 // sec

#define REGKEY_EXTMODEMS		TEXT("ExtModems")
#define REGKEY_MODEMGW			TEXT("SOFTWARE\\Microsoft\\bluetooth\\modemgw")
#define REGKEY_BTM_SERVICE		TEXT("Services\\BTMODEM")

#define SERVICE_UP_TIMEOUT_VALUE		1000
#define SERVICE_UP_TIMEOUT_COUNTER		40

//==============================================================================================
//		enumerations
//==============================================================================================
enum
{
	BTMCOMMAND_GET_EXTENDED_ERROR = 1,
	BTMCOMMAND_ACTIVATE,
	BTMCOMMAND_DEACTIVATE,
	BTMCOMMAND_CREATE_MSG_QUEUE,
	BTMCOMMAND_GET_CONTEXT,
	BTMCOMMAND_SET_CONTEXT
};

//==============================================================================================
//     structure definitions
//==============================================================================================

#pragma pack (push, 1)

typedef struct
{
	UINT32		IsOpened;
	UINT32		MuxWasActive;
	UINT32		ModemWasOn;
	UINT32		IsMuxModem;
	DWORD		LastError;
	TCHAR		ModemName[MAX_PATH];
} BTMODEM_CONTEXT;

#pragma pack (pop)

#endif //_BTMODEM_H_