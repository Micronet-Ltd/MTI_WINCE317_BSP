// Copyright 2009, 2013 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  AIODrvr.cpp
//
//  Automotive Input/Output Driver
//  
//
//  Created by Anna Rayer  September 2009
//	Changed by Vladimir Zatulovsky for CE300, CE317
//
//-----------------------------------------------------------------------------/*

#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>
#include <gpio.h>
#include <aio_api.h>
#include "aio.h"


#ifndef SHIP_BUILD

#undef ZONE_ERROR
#undef ZONE_WARN
#undef ZONE_FUNCTION
#undef ZONE_INFO

#define ZONE_ERROR          DEBUGZONE(0)
#define ZONE_WARN           DEBUGZONE(1)
#define ZONE_FUNCTION       DEBUGZONE(2)
#define ZONE_INFO           DEBUGZONE(3)

//------------------------------------------------------------------------------
//
//  Global:  dpCurSettings
//
DBGPARAM dpCurSettings = {
    L"AIO", {
        L"Errors",      L"Warnings",    L"Function",    L"Info",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined",
        L"Undefined",   L"Undefined",   L"Undefined",   L"Undefined"
    },
    ZONE_ERROR|ZONE_WARN
};

#endif

//==============================================================================================
//            local definitions
//==============================================================================================
//
#define MAX_PINS_NO                             2
#define INPUT_PINS_NUMBER                       1
#define MAX_OUTPUT_PINS                         1

#define  AIO_QUEUE_NAME		                     L"AIO queue name"
#define  IN_QUEUE_NAME                           L"In"
#define  OUT_QUEUE_NAME                          L"Out"

#define  AIO_REG_KEY			            	L"Drivers\\BuiltIn\\AIO"

typedef enum{
	INVALID_STA = -1,
	LO,
	HIG
}NOTIFY_STAT;

//------------------------------------------------------------------------------
//
typedef enum{
	UNREGISTERED = 0,
	REGISTERED,
    NOTIFY
} NOTIFY_STATE;

//==============================================================================================
//            structure definitions
//==============================================================================================
typedef struct __AIO_PINS_INFO {
    AUT_PINS    pinID; 
	DWORD       automotiveNo;
	PIN_TYPE    pinType; 
	DWORD       gpioID;
	UINT32      isWakeupSource;
	
	UINT32      stableTime;  
	UINT32      debounceTime;
	PIN_MODE    mode;


	HANDLE	    hIntEvent;			// GPIO Interrupt Event used by IST
	HANDLE	    hInputThread;		// Interrupt Service Thread
	BOOL		quit;				// Flag Used by Loop in IST (TRUE=exit thread)

	UINT32      pInitCtx;
	UINT32      isIntrInitialized;
	UINT32      currentPinState;

} AIO_PINS_INFO, *PAIO_PINS_INFO;

//------------------------------------------------------------------------------
typedef struct _AIO_INIT_CONTEXT
{
	PAIO_PINS_INFO      pinsInfoTable;
	HANDLE              hGpio;
	DWORD               priority256;
	CRITICAL_SECTION    OpenCS;	        // @field Protects Open Linked List
	UINT32				OpenContexts[32];
} AIO_INIT_CONTEXT,    *PAIO_INIT_CONTEXT;

//------------------------------------------------------------------------------
typedef struct __AIO_OPEN_CONTEXT
{
    PAIO_INIT_CONTEXT pAIOHead;                // @field Pointer back to our AIO_INIT_CONTEXT
	HANDLE            hInQueue[MAX_INPUT_PINS];
	HANDLE            hOutQueue[MAX_INPUT_PINS];
	TCHAR             daQueueName[MAX_INPUT_PINS][AIO_QUEUE_NAME_LEN/2];
	NOTIFY_STATE      notifyState[MAX_INPUT_PINS];
	DWORD             modeOwner[MAX_PINS_NO];
	UINT32            accState[MAX_PINS_NO];
} AIO_OPEN_CONTEXT, *PAIO_OPEN_CONTEXT;

//==============================================================================================
//            Device registry parameters
//==============================================================================================
static const DEVICE_REGISTRY_PARAM s_deviceInitRegParams[] = {
    {
        L"Priority256", PARAM_DWORD, FALSE, offset(AIO_INIT_CONTEXT, priority256),
        fieldsize(AIO_INIT_CONTEXT, priority256), (VOID*)250
    }
};

//------------------------------------------------------------------------------
static const DEVICE_REGISTRY_PARAM s_devicePinInfoRegParams[] = {

	{
        L"StableTime", PARAM_DWORD, FALSE, offset(AIO_PINS_INFO, stableTime),
        fieldsize(AIO_PINS_INFO, stableTime), (VOID*)0

    },
	{
        L"DebounceTime", PARAM_DWORD, FALSE, offset(AIO_PINS_INFO, debounceTime),
        fieldsize(AIO_PINS_INFO, debounceTime), (VOID*)20000

    }
};

//==============================================================================================
//           Pins_Info Table
//==============================================================================================
static AIO_PINS_INFO s_pinsInfoTable[MAX_PINS_NO]=
{	
	{ INPUT_PIN_0, 0,  INPUT_PIN,  GPIO_102,  0, 0, 0,(PIN_MODE)0, 0, 0, 0, 0, 0, 0},
	{ OUTPUT_PIN_0,1,  OUTPUT_PIN ,GPIO_94, 0, 0, 0,(PIN_MODE)0, 0, 0, 0, 0, 0, 0}
};

//==============================================================================================
//           Local Functions
//==============================================================================================
static DWORD InputThread(VOID *pContext);
static DWORD RegisterForStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID);
static DWORD DeregisterForStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID);
static DWORD StartStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID);
static DWORD StopStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID);
static DWORD GetInputPinState(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,DWORD* val);
static DWORD GetInputInfo(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,DWORD* state,DWORD* mode);
static DWORD SetStableTime(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,DWORD stableTime);
static DWORD GetStableTime(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,DWORD* stableTime);
static DWORD SetInputMode(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,PIN_MODE mode);
static DWORD GetQueueName(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,PAIO_GET_NAME_CONTEXT nameContext);
static DWORD SetOutputMode(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,PIN_MODE mode);
static DWORD SetOutputState(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,DWORD state);
static DWORD GetOutputInfo(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,DWORD* state,DWORD* mode);
static PAIO_PINS_INFO GetPinInfo(PAIO_INIT_CONTEXT pInitCtx,AUT_PINS pinID);
static DWORD SetInitConfig(PAIO_INIT_CONTEXT pCxt);
static DWORD ResetAccState(PAIO_INIT_CONTEXT pInitCtx,AUT_PINS pinNo);
static DWORD CalculateAccState(PAIO_INIT_CONTEXT pInitCtx,AUT_PINS pinID);
static DWORD DisablePinIntrIfNeeded(PAIO_PINS_INFO pPinInfo);

BOOL AIO_Deinit ( DWORD dwContext );
BOOL AIO_Close( DWORD Handle );

// ****************************************************************
//
//      @doc EXTERNAL
//      @func           HANDLE | AIO_Init | AIO driver initialization.
//
//      @parm           LPCTSTR  | Identifier | Port identifier.  The device loader
//                              passes in the registry key that contains information
//                              about the active device.
//
//      @remark         This routine is called at device load time in order
//                              to perform any initialization.   Typically the init
//                              routine does as little as possible, postponing memory
//                              allocation and device power-on to Open time.
//
//       @rdesc         Returns a pointer to the ioc head which is passed into
//                              the AIO_OPEN and AIO_DEINIT entry points as a device handle.
DWORD AIO_Init(LPCTSTR pContext)
{
	DEBUGMSG(ZONE_FUNCTION, ( _T ( "AIO: AIO_Init, pContext=%s\r\n" ), pContext ) );
	
	AIO_INIT_CONTEXT *pCxt =0;
	int i;
	_TCHAR szBuffer[MAX_PATH];

	// allocate AIO context object
	if(!(pCxt = (AIO_INIT_CONTEXT *)LocalAlloc(LPTR, sizeof(AIO_INIT_CONTEXT))))
		return 0;

	memset(pCxt, 0, sizeof(AIO_INIT_CONTEXT));
	pCxt->pinsInfoTable = s_pinsInfoTable;
	
	// set all the open contexts to be null

	for(i = 0; i < (sizeof(pCxt->OpenContexts)/sizeof(pCxt->OpenContexts[0])); i++)
	{
		pCxt->OpenContexts[i] = 0;
	}

	
    // Read device parameters
    if(GetDeviceRegistryParams(pContext, pCxt, dimof(s_deviceInitRegParams), s_deviceInitRegParams) != ERROR_SUCCESS)
	{
        RETAILMSG(1, (L"ERROR: AIO_Init: Failed read AIO driver registry parameters\r\n"));
        AIO_Deinit((DWORD)pCxt);
		return 0;
	}



	for ( i = 0; i < MAX_PINS_NO; i++)
	{
		pCxt->pinsInfoTable[i].pInitCtx           =(UINT32)pCxt;
		pCxt->pinsInfoTable[i].isIntrInitialized  = FALSE;
		pCxt->pinsInfoTable[i].quit               = FALSE;
		pCxt->pinsInfoTable[i].currentPinState    = INVALID_STATE;

		if(pCxt->pinsInfoTable[i].pinType == INPUT_PIN)
		{
			
            // initialize input threads
	
			_stprintf(szBuffer,  L"%s\\Input%d", AIO_REG_KEY, (pCxt->pinsInfoTable[i].automotiveNo+1));
            
            // Read device parameters
			if(GetDeviceRegistryParams(szBuffer, &pCxt->pinsInfoTable[i], dimof(s_devicePinInfoRegParams), s_devicePinInfoRegParams) != ERROR_SUCCESS)
			{
				RETAILMSG(1, (L"ERROR: AIO_Init: Failed read AIO driver pin info registry parameters, reg = %s\r\n",szBuffer));
				AIO_Deinit((DWORD)pCxt);
				return 0;
			}

			// Open gpio driver
			pCxt->hGpio = GPIOOpen();
			if (pCxt->hGpio == NULL)
			{
				RETAILMSG(1, (L"ERROR: AIO_Init: " L"Failed to open Gpio driver \r\n"));
				AIO_Deinit((DWORD)pCxt);
				return NULL;
			}

			pCxt->pinsInfoTable[i].hIntEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(!pCxt->pinsInfoTable[i].hIntEvent)
			{
				RETAILMSG(1, (L"ERROR: AIO_Init: input thread event failure\r\n"));
				AIO_Deinit((DWORD)pCxt);
				return 0;
			}

			if (!GPIOInterruptInitialize(pCxt->hGpio, pCxt->pinsInfoTable[i].gpioID,pCxt->pinsInfoTable[i].hIntEvent))
			{
				RETAILMSG(1, (L"ERROR: AIO_Init: GPIOInterruptInitialize failed \r\n"));
				AIO_Deinit((DWORD)pCxt);
				return 0;
			}
			if (!GPIOInterruptMask(pCxt->hGpio, pCxt->pinsInfoTable[i].gpioID,TRUE))
			{
				RETAILMSG(1, (L"ERROR: AIO_Init: GpioInterruptMask failed \r\n"));
				AIO_Deinit((DWORD)pCxt);
				return 0;
			}

			RETAILMSG(0, (L"AIO_Init: Create input thread : for pinID %x(%d), event - %x \r\n", pCxt->pinsInfoTable[i].pinID, pCxt->pinsInfoTable[i].gpioID, pCxt->pinsInfoTable[i].hIntEvent));
			pCxt->pinsInfoTable[i].hInputThread = CreateThread(0, 0, InputThread, &(pCxt->pinsInfoTable[i]), 0, 0);
			if(!pCxt->pinsInfoTable[i].hInputThread)
			{
				RETAILMSG (1, (L"ERROR: AIO_Init: Failed to create aio thread\r\n"));
				AIO_Deinit((DWORD)pCxt);
				return 0;
			}
		}
	}

	InitializeCriticalSection(&(pCxt->OpenCS));
	SetInitConfig(pCxt);

	return (DWORD)pCxt;
}

//------------------------------------------------------------------------------
// Function name	: AIO_Deinit
// Description	    : DeInitialization of all used resources
// Return type		: BOOL  
// Argument         : DWORD dwContext
//------------------------------------------------------------------------------
BOOL AIO_Deinit ( DWORD dwContext )
{
    AIO_INIT_CONTEXT *pInitCtx = (AIO_INIT_CONTEXT *)dwContext;
	PAIO_OPEN_CONTEXT pOpenCtx;
	int i;

	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "IOC: +AIO_Deinit, dwContext=%d\r\n" ), dwContext ));

	// Check if we get correct context
    if(!pInitCtx)
	{
        RETAILMSG (1, (L"ERROR: AIO_Deinit: Incorrect context parameter\r\n"));
        return 0;
	}

	EnterCriticalSection(&(pInitCtx->OpenCS));
	for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		pOpenCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
		if (pOpenCtx)
		{
			AIO_Close((DWORD)pOpenCtx);
		}
	}


	// to close all the existed threads
	for ( i = 0; i < MAX_PINS_NO; i++)
	{
		if (!GPIOInterruptDisable(pInitCtx->hGpio, pInitCtx->pinsInfoTable[i].gpioID))
		{
			RETAILMSG (1,(TEXT("AIO_Deinit: GPIOInterruptDisable: failed \r\n")));
		}
		
		pInitCtx->pinsInfoTable[i].quit = TRUE;
		pInitCtx->pinsInfoTable[i].pInitCtx =0;
		pInitCtx->pinsInfoTable[i].isIntrInitialized = FALSE;

		if(pInitCtx->pinsInfoTable[i].hIntEvent && pInitCtx->pinsInfoTable[i].hInputThread)
		{
			// signal input thread event
			SetEvent(pInitCtx->pinsInfoTable[i].hIntEvent);
			// Wait until thread exits
			WaitForSingleObject(pInitCtx->pinsInfoTable[i].hInputThread, INFINITE);
			// Close handles
			CloseHandle(pInitCtx->pinsInfoTable[i].hInputThread);
            pInitCtx->pinsInfoTable[i].hInputThread = 0;

			CloseHandle(pInitCtx->pinsInfoTable[i].hIntEvent);
			pInitCtx->pinsInfoTable[i].hIntEvent = 0;
		}
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	DeleteCriticalSection(&(pInitCtx->OpenCS));

	  // Close Gpio 
    if(pInitCtx->hGpio)
		GPIOClose(pInitCtx->hGpio);
	LocalFree(pInitCtx);

	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "APD: -AIO_Deinit\r\n" ) ) );

    return TRUE;
}


//------------------------------------------------------------------------------
// Function name	: AIO_IOControl
// Description	    : IOControl ... :-/
// Return type		: BOOL   IOC_IOControl 
// Argument         : DWORD  hHandle
// Argument         : DWORD  dwIoControlCode
// Argument         : PBYTE  pInBuf
// Argument         : DWORD  nInBufSize
// Argument         : PBYTE  pOutBuf
// Argument         : DWORD  nOutBufSize
// Argument         : PDWORD pBytesReturned
//------------------------------------------------------------------------------
BOOL AIO_IOControl(DWORD hHandle, DWORD dwIoControlCode, PBYTE pInBuf, DWORD nInBufSize, PBYTE pOutBuf, DWORD nOutBufSize, PDWORD pBytesReturned)
{

	PAIO_OPEN_CONTEXT pOpenCtx = (PAIO_OPEN_CONTEXT)hHandle;
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT)pOpenCtx->pAIOHead;
	DWORD	*pBuff;

	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "AIO: +AIO_IOControl, IOControl=%d\r\n" ), dwIoControlCode ) );

	BOOL bRet = TRUE;
	

	// Check if we get correct context
    if(!pInitCtx)
	{
        RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

        SetLastError(ERROR_INVALID_PARAMETER);

        return 0;
    }
 
	EnterCriticalSection(&(pInitCtx->OpenCS));
	switch (dwIoControlCode)
	{
		case IOCTL_ATSR_GETQUEUENAME:
		{

			AUT_PINS                           pinNo;
			DWORD                              errorCode;
			//TCHAR                            name[AIO_QUEUE_NAME_LEN/2];
			AIO_GET_NAME_CONTEXT               nameContext;
			PAIO_GET_NAME_CONTEXT              pBuff;


			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < sizeof(AIO_GET_NAME_CONTEXT))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = GetQueueName(pOpenCtx,pinNo,&nameContext);

				if (errorCode)
					bRet = FALSE;

				pBuff = (PAIO_GET_NAME_CONTEXT)pOutBuf;

				CeSafeCopyMemory (pBuff, &nameContext, sizeof(nameContext));
				//CeSafeCopyMemory (pBuff->queueName, name, sizeof(name));

			}

			break;

		}
		case IOCTL_INPUT_REGISTER_STATE_CHANGE:
		{
			AUT_PINS          pinNo;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = RegisterForStateNotify(pOpenCtx,pinNo);
				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (pOutBuf, &errorCode, sizeof(errorCode));

			}

			break;
		}
		case IOCTL_INPUT_DEREGISTER_STATE_CHANGE:
		{
			AUT_PINS          pinNo;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = DeregisterForStateNotify(pOpenCtx,pinNo);
				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (pOutBuf, &errorCode, sizeof(errorCode));

			}

			break;
		}
		case IOCTL_INPUT_START_CHANGE_EVENT:
		{
			AUT_PINS          pinNo;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = StartStateNotify(pOpenCtx,pinNo);
				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (pOutBuf, &errorCode, sizeof(errorCode));

			}

			break;
		}
		case IOCTL_INPUT_STOP_CHANGE_EVENT:
		{
			AUT_PINS          pinNo;
			DWORD             errorCode;
		
			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = StopStateNotify(pOpenCtx,pinNo);
				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (pOutBuf, &errorCode, sizeof(errorCode));

			}

			break;
		}
		case IOCTL_INPUT_GET_STATE:
		{
			AUT_PINS          pinNo;
			DWORD             errorCode;
			DWORD             pinState;
			
			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < 2*sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = GetInputPinState(pOpenCtx,pinNo,&pinState);
				if (errorCode)
					bRet = FALSE;

				pBuff = (DWORD*)pOutBuf;

				CeSafeCopyMemory (&pBuff[1], &pinState, sizeof(DWORD));
				CeSafeCopyMemory (&pBuff[0], &errorCode, sizeof(DWORD));

			}

			break;
		}
		case IOCTL_INPUT_GET_INFO:
		{
			AUT_PINS          pinNo;
			DWORD             state,mode;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < sizeof(DWORD) ||!pOutBuf || nOutBufSize < 3*sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);
			}
			else
			{
				pBuff = (DWORD*)pOutBuf;

				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = GetInputInfo(pOpenCtx,pinNo,&state,&mode);

				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (&pBuff[0], &errorCode, sizeof(DWORD));
				CeSafeCopyMemory (&pBuff[1], &state, sizeof(state));
				CeSafeCopyMemory (&pBuff[2], &mode, sizeof(mode));
			}

			break;
		}
		case IOCTL_INPUT_SET_STABLE_TIME:
		{
			AUT_PINS          pinNo;
			DWORD             stableTime;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < 2*sizeof(DWORD) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{

				pBuff = (DWORD*)pInBuf;

				CeSafeCopyMemory (&pinNo,      &pBuff[0], sizeof(pinNo));
				CeSafeCopyMemory (&stableTime, &pBuff[1], sizeof(stableTime));

				errorCode = SetStableTime(pOpenCtx,pinNo,stableTime);
				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (&pOutBuf[0], &errorCode, sizeof(DWORD));
			}

			break;
		}
		case IOCTL_INPUT_GET_STABLE_TIME:
		{
			AUT_PINS          pinNo;
			DWORD             errorCode;
			DWORD             stableTime;
			
			if(!pInBuf || nInBufSize < sizeof(AUT_PINS) ||!pOutBuf || nOutBufSize < 2*sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = GetStableTime(pOpenCtx,pinNo,&stableTime);
				if (errorCode)
					bRet = FALSE;

				pBuff = (DWORD*)pOutBuf;

				CeSafeCopyMemory (&pBuff[0], &errorCode, sizeof(DWORD));
				CeSafeCopyMemory (&pBuff[1], &stableTime, sizeof(DWORD));
			}

			break;
		}
		case IOCTL_INPUT_SET_MODE:
		{
			AUT_PINS          pinNo;
			DWORD             mode;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < 2*sizeof(DWORD) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				pBuff = (DWORD*)pInBuf;

				CeSafeCopyMemory (&pinNo, &pBuff[0], sizeof(pinNo));
				CeSafeCopyMemory (&mode,  &pBuff[1], sizeof(DWORD));

				errorCode = SetInputMode(pOpenCtx,pinNo,(PIN_MODE)mode);
				if (errorCode)
					bRet = FALSE;

				CeSafeCopyMemory (&pOutBuf[0], &errorCode, sizeof(DWORD));
			}
			break;
		}
		case IOCTL_INPUT_GET_PINS_NO:
		{
			DWORD             pinsNo = INPUT_PINS_NUMBER ;
			
			if(!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				pBuff = (DWORD*)pOutBuf;

				CeSafeCopyMemory (pBuff, &pinsNo, sizeof(DWORD));
				bRet = TRUE;

			}

			break;
		}
		case IOCTL_OUTPUT_SET_MODE:
		{
			AUT_PINS          pinNo;
			DWORD             mode;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < 2*sizeof(DWORD) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				pBuff = (DWORD*)pInBuf;

				CeSafeCopyMemory (&pinNo, &pBuff[0], sizeof(pinNo));
				CeSafeCopyMemory (&mode,  &pBuff[1], sizeof(DWORD));

				errorCode = SetOutputMode(pOpenCtx,pinNo,(PIN_MODE)mode);

				if (errorCode)
					bRet = FALSE;

				
				CeSafeCopyMemory (&pOutBuf[0], &errorCode, sizeof(DWORD));

			}

			break;
		}
		case IOCTL_OUTPUT_SET_STATE:
		{
			AUT_PINS          pinNo;
			DWORD             state;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < 2*sizeof(DWORD) ||!pOutBuf || nOutBufSize < sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				pBuff = (DWORD*)pInBuf;

				CeSafeCopyMemory (&pinNo, &pBuff[0], sizeof(pinNo));
				CeSafeCopyMemory (&state,  &pBuff[1], sizeof(DWORD));

				errorCode = SetOutputState(pOpenCtx,pinNo,state);

				if (errorCode)
					bRet = FALSE;

				
				CeSafeCopyMemory (&pOutBuf[0], &errorCode, sizeof(DWORD));

			}

			break;
		}
		case IOCTL_OUTPUT_GET_INFO:
		{
			AUT_PINS          pinNo;
			DWORD             state,mode;
			DWORD             errorCode;

			if(!pInBuf || nInBufSize < sizeof(DWORD) ||!pOutBuf || nOutBufSize < 3*sizeof(DWORD))
			{
				RETAILMSG(1, (L"ERROR: AIO_IOControl: Incorrect context parameter\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);

			}
			else
			{
				pBuff = (DWORD*)pOutBuf;

				CeSafeCopyMemory (&pinNo, pInBuf, sizeof(pinNo));

				errorCode = GetOutputInfo(pOpenCtx,pinNo,&state,&mode);

				if (errorCode)
					bRet = FALSE;

				
				CeSafeCopyMemory (&pBuff[0], &errorCode, sizeof(DWORD));
				CeSafeCopyMemory (&pBuff[1], &state, sizeof(state));
				CeSafeCopyMemory (&pBuff[2], &mode, sizeof(mode));
			}

			break;
		}
		default:
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			bRet = FALSE;
			break;
		}
    }

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	                                

	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "AIO: -AIO_IOControl, bRet=%d\r\n" ), bRet ) );

    return bRet;
}

//------------------------------------------------------------------------------
// Function name	: AIO_Open
// Description	    : This function returns handle for safe access to automotive pins.
//                    This handle must be used for all automotive pins’ operations. 
// Return type		: DWORD  
// Argument         : DWORD dwContext

//------------------------------------------------------------------------------
DWORD AIO_Open ( DWORD dwContext, DWORD dwAccess, DWORD dwShareMode )
{
	AIO_INIT_CONTEXT *pInitCtx = (AIO_INIT_CONTEXT *)dwContext;
	PAIO_OPEN_CONTEXT pOpenCtx;
	MSGQUEUEOPTIONS		msgOptions;
	DWORD				rc = 0;

	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_Open\r\n" ) ) );

	 // Return NULL if AIO_Init failed.
    if (!pInitCtx ) {
        RETAILMSG (1,(TEXT("Open attempted on uninited device!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(NULL);
    }

	EnterCriticalSection(&(pInitCtx->OpenCS));
	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		if(pInitCtx->OpenContexts[i] == 0)
		{
			// Create device handle

			 // OK, lets allocate an open structure
			pOpenCtx    =  (PAIO_OPEN_CONTEXT)LocalAlloc(LPTR, sizeof(AIO_OPEN_CONTEXT));
			
			if(!pOpenCtx)
			{
				RETAILMSG(1, (L"ERROR: AIO_Open: Failed allocate open context driver structure\r\n"));

				SetLastError(ERROR_INVALID_PARAMETER);
				rc = 0;
				break;
			}

			memset(pOpenCtx, 0, sizeof(AIO_OPEN_CONTEXT));

			memset(&msgOptions, 0, sizeof(msgOptions));
			msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
			msgOptions.dwFlags = 0;
			msgOptions.cbMaxMessage = sizeof(AIO_INPUT_CONTEXT);
			msgOptions.bReadAccess = 0;

			// initialize input specific data
            for(int pinNo = 0; pinNo < MAX_INPUT_PINS;pinNo++ )
			{
				memset(pOpenCtx->daQueueName[pinNo], 0, sizeof(pOpenCtx->daQueueName[pinNo]));
				wsprintf(pOpenCtx->daQueueName[pinNo], L"%s%d%d%s", AIO_QUEUE_NAME, i,pinNo,OUT_QUEUE_NAME); 

				// create out msg queue
				//wsprintf(QueueName, L"%s%s", pOpenCtx->QueueName[pinNo], OUT_QUEUE_NAME); 
				pOpenCtx->hOutQueue[pinNo] = CreateMsgQueue(pOpenCtx->daQueueName[pinNo], &msgOptions);

				if(pOpenCtx->hOutQueue[pinNo] == NULL)
				{
					rc = 0;
					break;
				}

				pOpenCtx->notifyState[pinNo] = UNREGISTERED;
				RETAILMSG(0, (L"AIO_Open: %s\r\n",  pOpenCtx->daQueueName[pinNo]));
			}

			//memset(&pOpenCtx->mode, 0, sizeof(pOpenCtx->mode)); 


			pOpenCtx->pAIOHead = pInitCtx;  // pointer back to our parent
     
			pInitCtx->OpenContexts[i] = (UINT32)pOpenCtx;
			rc = (DWORD)pOpenCtx;
			break;
		}
	}

    LeaveCriticalSection(&(pInitCtx->OpenCS));

	//pInitCtx->pAccessOwner = pOpenCtx;
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: -AIO_Open\r\n" ) ) );
    
    return rc;
}

//------------------------------------------------------------------------------
// Function name	: AIO_Close
// Description	    : ...
// Return type		: BOOL 
// Argument         : DWORD Handle
//------------------------------------------------------------------------------
BOOL AIO_Close( DWORD Handle )
{
	PAIO_OPEN_CONTEXT pOpenCtx = (PAIO_OPEN_CONTEXT)Handle;
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT)pOpenCtx->pAIOHead;
	int i;
	DWORD state;
	BOOL  rc = FALSE;


	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_Close\r\n" ) ) );

	if ( !pInitCtx ) {
        RETAILMSG (1, (TEXT("!!AIO_Close: pInitCtx == NULL!!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

	EnterCriticalSection(&(pInitCtx->OpenCS));

    for(int pinNo = 0; pinNo < MAX_PINS_NO;pinNo++ )
	{
		PAIO_PINS_INFO pPinInfo = NULL;

		for (i = 0 ; i < MAX_PINS_NO; i++)
		{
			if (pInitCtx->pinsInfoTable[i].automotiveNo == pinNo)
			{
				pPinInfo = &pInitCtx->pinsInfoTable[i];
				break;

			}			
		}

		// this handle is an owner of accumulated or locked mode
		if (pOpenCtx->modeOwner[pinNo])
		{
			// find the pin catched by this handle and
			// release this pin to be in free mode
			
			if (pPinInfo->mode == ACCUMULATED)
				ResetAccState(pInitCtx,pPinInfo->pinID);

             pPinInfo->mode = FREE;
			 pOpenCtx->modeOwner[pinNo] = 0;
		}

		// if this handle participates in accumualted mode calculation
		// then recalculate the state w/o the handle
		else if ( pPinInfo->mode == ACCUMULATED && pOpenCtx->accState[pinNo])
		{

			pOpenCtx->accState[pinNo]=0;
			state = CalculateAccState(pInitCtx,pPinInfo->pinID);
			if(state)
			{
			  GPIOSetBit(pInitCtx->hGpio, pPinInfo->gpioID);
			}
			else
			  GPIOClrBit(pInitCtx->hGpio, pPinInfo->gpioID);

		}

		// sends msg "handle is closed" to signal WaitForStateChange function
		// this situation is invalid just in the case AIO_Close has not been called
		// to close AIO component but directly CloseHandle();
		if(pOpenCtx->notifyState[pinNo] == NOTIFY /*|| pOpenCtx->notifyState[pinNo] == REGISTERED*/)
		{
			AIO_INPUT_CONTEXT inputContext;

			inputContext.size =  sizeof(AIO_INPUT_CONTEXT),
			inputContext.pinNo = pPinInfo->pinID;
			inputContext.opErrorCode = AIO_ERROR_INVALID_HANDLE;

			inputContext.currentState =  INVALID_STATE;
			inputContext.prevState    =  INVALID_STATE;


			if(pOpenCtx->hOutQueue[pinNo])
			{
				WriteMsgQueue(pOpenCtx->hOutQueue[pinNo], &inputContext, sizeof(inputContext), 10, 0);
				Sleep(0);
			}

			pOpenCtx->notifyState[pinNo] = REGISTERED;
			DisablePinIntrIfNeeded(pPinInfo);
		}

		if(pOpenCtx->hOutQueue[pinNo])
		{
			CloseMsgQueue(pOpenCtx->hOutQueue[pinNo]);
			pOpenCtx->hOutQueue[pinNo]=0;
		}

		if(pOpenCtx->hInQueue[pinNo])
		{
			CloseMsgQueue(pOpenCtx->hInQueue[pinNo]);
			pOpenCtx->hInQueue[pinNo]=0;
		}
    	

	}

	for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		
		if(pInitCtx->OpenContexts[i] == (UINT32)pOpenCtx)
		{
			pOpenCtx->pAIOHead = NULL;
			LocalFree(pOpenCtx);
			pInitCtx->OpenContexts[i] = 0;
			rc = TRUE;
			break;
		}
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));

	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: -AIO_Close\r\n" ) ) );

    return rc;
}

//------------------------------------------------------------------------------
// Function name	: AIO_Read
// Description	    : ...
// Return type		: DWORD AIO_Read 
// Argument         : DWORD Handle
// Argument         : LPVOID pBuffer
// Argument         : DWORD dwNumBytes
//------------------------------------------------------------------------------
DWORD AIO_Read ( DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes )
{
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_Read\r\n" ) ) );

    return (DWORD) -1;
}
	                            
//------------------------------------------------------------------------------
// Function name	: {
// Description	    : ...
// Return type		: DWORD { 
// Argument         : DWORD Handle
// Argument         : LPCVOID pBuffer
// Argument         : DWORD dwNumBytes
//------------------------------------------------------------------------------
DWORD AIO_Write ( DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes )
{
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_WRITE\r\n" ) ) );

    return (DWORD) -1;
}

//------------------------------------------------------------------------------
// Function name	: AIO_Seek
// Description	    : ...
// Return type		: DWORD AIO_Seek 
// Argument         : DWORD Handle
// Argument         : long lDistance
// Argument         : DWORD dwMoveMethod
//------------------------------------------------------------------------------
DWORD AIO_Seek ( DWORD Handle, long lDistance, DWORD dwMoveMethod )
{
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_Seek\r\n" ) ) );
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: -AIO_Seek\r\n" ) ) );

    return (DWORD) -1;
}

//------------------------------------------------------------------------------
// Function name	: AIO_PowerUp
// Description	    : ...
// Return type		: void AIO_PowerUp 
// Argument         : DWORD hDeviceContext
//------------------------------------------------------------------------------
void AIO_PowerUp ( DWORD hDeviceContext )
{
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_PowerUp\r\n" ) ) );

	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: -AIO_PowerUp\r\n" ) ) );
}


//------------------------------------------------------------------------------
// Function name	: AIO_PowerDown
// Description	    : ...
// Return type		: void AIO_PowerDown 
// Argument         : DWORD hDeviceContext
//------------------------------------------------------------------------------
void AIO_PowerDown ( DWORD hDeviceContext )
{
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: +AIO_PowerDown\r\n" ) ) );

	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: -AIO_PowerDown\r\n" ) ) );
}

// Function name	: DllEntry 
// Description	    : ...
// Return type		: BOOL WINAPI 
// Argument         : HANDLE hInstDll
// Argument         : DWORD dwReason
// Argument         : LPVOID lpvReserved
BOOL WINAPI DllEntry ( HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved )
{
	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "AIO: +DllEntry\r\n" ) ) );

    switch ( dwReason ) 
	{
    case DLL_PROCESS_ATTACH:

		DisableThreadLibraryCalls ( ( HINSTANCE )hInstDll );

		DEBUGREGISTER ( ( HINSTANCE )hInstDll );

        DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: DLL_PROCESS_ATTACH\r\n" ) ) );

        break;

    case DLL_PROCESS_DETACH:
    
        DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "AIO: DLL_PROCESS_DETACH\r\n" ) ) );

        break;
    }


	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "AIO: -DllEntry\r\n" ) ) );

	return TRUE;
}

//==============================================================================================
//
//  Function:     InputThread
//  Description:  Thread per input pin
//
//==============================================================================================
DWORD InputThread(VOID *pContext)
{
	PAIO_PINS_INFO pPinInfo = (PAIO_PINS_INFO)pContext;
	DWORD timeout = INFINITE;

	DWORD i;

	if(!pPinInfo)
	{
        RETAILMSG(1, (L" InputThread: Invalid context passed to thread routine.\r\n"));
        ASSERT(0);
        return -1;
	}

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT )pPinInfo->pInitCtx;
	PAIO_OPEN_CONTEXT pOpenCtx;

	AIO_INPUT_CONTEXT inputContext;


	inputContext.size =  sizeof(AIO_INPUT_CONTEXT),
    inputContext.pinNo = pPinInfo->pinID;
	inputContext.opErrorCode = AIO_OK;

	if(!pInitCtx || pInitCtx->hGpio == INVALID_HANDLE_VALUE)
	{
        RETAILMSG(1, (L" InputThread: Failed opening GPIO driver\r\n"));
        ASSERT( 0 );
        return -1;
	}

	while (!pPinInfo->quit)
	{
		 // Wait for event
        WaitForSingleObject(pPinInfo->hIntEvent, timeout);

        if (pPinInfo->quit)
			break;

		// start debounce process if enabled  (stableTime, debounceTime)
		//
		if(pPinInfo->stableTime)
		{
			// start s/w debounce
			UINT32 stableTime,time,endTime;
			UINT32 val,newVal;
			DWORD  threadPriority = CeGetThreadPriority(GetCurrentThread ());

			// set priority

			CeSetThreadPriority(GetCurrentThread(), pInitCtx->priority256);
			time =  GetTickCount();
			endTime = time + pPinInfo->debounceTime;

            val = GPIOGetBit(pInitCtx->hGpio, pPinInfo->gpioID);
	
			do
			{
				newVal = GPIOGetBit(pInitCtx->hGpio, pPinInfo->gpioID);
				
				if ( newVal != val)
				{
				   time = GetTickCount();
				   val = newVal;
				//   chCounter++;
				}
				//Counter++;
				stableTime = GetTickCount()- time;
				
			}while (time < endTime && stableTime < pPinInfo->stableTime);

			CeSetThreadPriority(GetCurrentThread(), threadPriority);



			if(stableTime < pPinInfo->stableTime)
			{
				// signal is not stable during debounce period
				// error will be sent to the user
			  inputContext.opErrorCode = AIO_ERROR_SIGNAL_NOT_STABLE;
			  inputContext.currentState = inputContext.prevState = INVALID_STATE;
			}
			else
			{
				// signal is stable for user defined stable time period

				inputContext.opErrorCode  = AIO_OK;
				inputContext.currentState = (newVal)?HIGH:LOW;
				inputContext.prevState    = (PIN_STATE)pPinInfo->currentPinState;
				pPinInfo->currentPinState = inputContext.currentState;
			}

			RETAILMSG(0, (L" InputThread DT: CS = 0x%x,PS = 0x%x, ST = %d \r\n",inputContext.currentState,inputContext.prevState, stableTime));
		}
		else // there is no debounce process
		{
			inputContext.currentState = (GPIOGetBit(pInitCtx->hGpio, pPinInfo->gpioID))?HIGH:LOW;  // inverted values

			inputContext.prevState =     (PIN_STATE)pPinInfo->currentPinState;
			inputContext.opErrorCode = AIO_OK;

			pPinInfo->currentPinState = inputContext.currentState;
			RETAILMSG(0, (L" InputThread: CS = 0x%x,PS = 0x%x\r\n",inputContext.currentState,inputContext.prevState));
		}

		//
		EnterCriticalSection(&(pInitCtx->OpenCS));
		
		for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
		{
			pOpenCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];

			if(pOpenCtx != 0 && pOpenCtx->notifyState[pPinInfo->automotiveNo] == NOTIFY)
			{
				RETAILMSG(0, (L"AIO: WriteMsgQueue to  pOpenCtx->hOutQueue[pPinInfo->automotiveNo] 0x%x\r\n", pOpenCtx->hOutQueue[pPinInfo->automotiveNo]));
	
				if(!WriteMsgQueue(pOpenCtx->hOutQueue[pPinInfo->automotiveNo], &inputContext, sizeof(inputContext), INFINITE, 0)) 
				{
					RETAILMSG(1, (L"AIO: WriteMsgQueue failed 0x%x\r\n",GetLastError()));
				}
			}
		}

		// in the case of not stable signal we need disable this pin interrupt and change pin's state for all the handles
		if (inputContext.opErrorCode == AIO_ERROR_SIGNAL_NOT_STABLE && pPinInfo->isIntrInitialized)
		{

			for(i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
			{
				PAIO_OPEN_CONTEXT pCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];

				if(pCtx != 0 && pCtx->notifyState[pPinInfo->automotiveNo] == NOTIFY)
				{
					pCtx->notifyState[pPinInfo->automotiveNo] = REGISTERED;
				}
			}

			if (!GPIOInterruptMask(pInitCtx->hGpio, pPinInfo->gpioID, 1))
			{
				RETAILMSG (1,(TEXT("GPIOInterruptMask: failed \r\n")));
				pPinInfo->quit = TRUE;
			}
			
			pPinInfo->isIntrInitialized =FALSE;


			// sleep for 1 sec to enable to all application handles to get "signal not stable" event (msg)
			Sleep(1000);
			
		}
		else
		{
			// enable interrupt
			if (!GPIOInterruptDone(pInitCtx->hGpio, pPinInfo->gpioID))
			{
				RETAILMSG (1,(TEXT("GPIODone: failed \r\n")));
				pPinInfo->quit = TRUE;
			}
		}

		LeaveCriticalSection(&(pInitCtx->OpenCS));
	}

	return 0;

}

//==============================================================================================
//
//  Function:     RegisterForStateNotify
//  Description:  Initialize the pin's interrupt if not yet initialized
//
//==============================================================================================
DWORD RegisterForStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT) pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN)
	{
		RETAILMSG (1,(TEXT("RegisterForStateNotify: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	if (pOpenCtx->notifyState[pPinInfo->automotiveNo] != UNREGISTERED)
	{
		RETAILMSG (1,(TEXT("RegisterForStateNotify: Error: This handle is already registered notifyState[0x%x]=0x%x\r\n"),
			pPinInfo->automotiveNo,pOpenCtx->notifyState[pPinInfo->automotiveNo] ));

		return AIO_ERROR_ALREDY_REGISTERED;
	}

	pOpenCtx->notifyState[pPinInfo->automotiveNo] = REGISTERED;

	return ret;
}

//==============================================================================================
//
//  Function:     RegisterForStateNotify
//  Description:  Disable the pin's interrupt if there are not other handles 
//                registered on this pin state change
//
//==============================================================================================

DWORD DeregisterForStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID)
{
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT) pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;
	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN)
	{
		RETAILMSG (1,(TEXT("DeregisterForStateNotify: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	if (pOpenCtx->notifyState[pPinInfo->automotiveNo] != REGISTERED)
	{
		RETAILMSG (1,(TEXT("DeregisterForStateNotify: Error: This handle is not registered\r\n")));

		return AIO_ERROR_WRONG_NOTIFICATION_STATE;
	}

	pOpenCtx->notifyState[pPinInfo->automotiveNo] = UNREGISTERED;

	return ret;
}

//==============================================================================================
//
//  Function:     StartStateNotify
//  Description:  Marks this handle as "Notify" 
//
//==============================================================================================
DWORD StartStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID)
{
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT) pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;
	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN)
	{
		RETAILMSG (1,(TEXT("StartStateNotify: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	if (pOpenCtx->notifyState[pPinInfo->automotiveNo] != REGISTERED)
	{
		RETAILMSG (1,(TEXT("StartStateNotify: Error: wrong notify state. notifyState[0x%x]=0x%x\r\n"),
			pPinInfo->automotiveNo,pOpenCtx->notifyState[pPinInfo->automotiveNo] ));

		return AIO_ERROR_WRONG_NOTIFICATION_STATE;
	}

	if(!pPinInfo->isIntrInitialized)
	{
		// enable interrupt
		if (!GPIOInterruptMask(pInitCtx->hGpio, pPinInfo->gpioID, 0))
		{
			RETAILMSG (1,(TEXT("GpioInterruptMask: failed \r\n")));
			return AIO_INTERNAL_ERROR;
		}

		RETAILMSG(0, (TEXT("%S: Interrupt unmasked %d\r\n"), __FUNCTION__, pPinInfo->gpioID));
		pPinInfo->isIntrInitialized = 1;

		// enable wakeup event if requested
		if(pPinInfo->isWakeupSource)
		{
			if (!GPIOWakeEnable(pInitCtx->hGpio, pPinInfo->gpioID))
			{

				DisablePinIntrIfNeeded(pPinInfo);
				RETAILMSG (1,(TEXT("Setting wake up: failed \r\n")));
				return AIO_INTERNAL_ERROR;
			}
			else
			{
				RETAILMSG (0,(TEXT("Setting wake up: Ok, gpioID = %d\r\n"), pPinInfo->gpioID));
			}


		}
	}

	pOpenCtx->notifyState[pPinInfo->automotiveNo] = NOTIFY;

	return ret;
}


//==============================================================================================
//
//  Function:     StopStateNotify
//  Description:  Marks this handle as "NOT Notify" 
//
//==============================================================================================
DWORD StopStateNotify(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT) pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN)
	{
		RETAILMSG (1,(TEXT("StopStateNotify: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	if (pOpenCtx->notifyState[pPinInfo->automotiveNo] != NOTIFY)
	{
		RETAILMSG (1,(TEXT("StopStateNotify: Error: wrong notify state. notifyState[0x%x]=0x%x\r\n"),
			pPinInfo->automotiveNo,pOpenCtx->notifyState[pPinInfo->automotiveNo] ));

		return AIO_ERROR_WRONG_NOTIFICATION_STATE;
	}

	pOpenCtx->notifyState[pPinInfo->automotiveNo] = REGISTERED;

	ret = DisablePinIntrIfNeeded(pPinInfo);

	if(ret == AIO_OK)
	{
		// disable wakeup event if requested
		if(pPinInfo->isWakeupSource)
		{
			if (!GPIOWakeDisable(pInitCtx->hGpio, pPinInfo->gpioID))
			{
				RETAILMSG (1,(TEXT("Disable wake up: failed \r\n")));
				ret =  AIO_INTERNAL_ERROR;
			}

		}
	}

	// if DisablePinIntrIfNeeded failed return back notifyState to its prev state;
	if(ret != AIO_OK) 
		pOpenCtx->notifyState[pPinInfo->automotiveNo] = NOTIFY;

	return ret;
}

//==============================================================================================
//
//  Function:     DisablePinIntrIfNeeded
//  Description:  Check if there are  other handles registered on this pin change 
//
//==============================================================================================
DWORD DisablePinIntrIfNeeded(PAIO_PINS_INFO pPinInfo)
{
	DWORD             ret = AIO_OK;

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT) pPinInfo->pInitCtx;
	BOOL   needDisableInterrupt = TRUE;           

	if (pPinInfo->isIntrInitialized == TRUE)
	{
		// check if there is any other handle registered for notifications
		// if not then disable interrupt

		for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
		{
			PAIO_OPEN_CONTEXT pCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];

			if(pCtx != 0 && pCtx->notifyState[pPinInfo->automotiveNo] == NOTIFY)
			{
				needDisableInterrupt = FALSE;
				break;
			}
		}
		if(needDisableInterrupt)
		{
			if (!GPIOInterruptMask(pInitCtx->hGpio, pPinInfo->gpioID,TRUE))
			{
				RETAILMSG (1,(TEXT("GPIOInterruptDisable: failed \r\n")));
				return AIO_INTERNAL_ERROR;
			}
			
			pPinInfo->isIntrInitialized =FALSE;
		}
	}

	return ret;
}

//==============================================================================================
//
//  Function:     GetInputPinState
//  Description:  Reads input pin state 
//
//==============================================================================================
DWORD GetInputPinState(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,DWORD* val)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT)pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN)
	{
		RETAILMSG (1,(TEXT("GetInputPinState: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	*val = GPIOGetBit(pInitCtx->hGpio, pPinInfo->gpioID);

	return ret;
}

//==============================================================================================
//
//  Function:     SetStableTime
//  Description:  Sets stable time in the registry and Pin Info structure
//
//==============================================================================================
DWORD SetStableTime(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,DWORD stableTime)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT)pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN || stableTime < 0 || stableTime > MAX_STABLE_TIME )
	{
		RETAILMSG (1,(TEXT("SetStableTime: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	// we are in locked mode
	if(pPinInfo->mode == LOCKED && !pOpenCtx->modeOwner[pPinInfo->automotiveNo])
	{
		RETAILMSG (1,(TEXT("SetStableTime: The pin is locked. Wrong permissions\r\n")));
		return AIO_ERROR_INPUT_WRONG_ACCESS;
	}

	// set this parameter to the registry
	DWORD status = ERROR_SUCCESS;
	HKEY hKey;
	_TCHAR szBuffer[MAX_PATH];

	_stprintf(szBuffer,  L"%s\\%Input%d", AIO_REG_KEY,(pPinInfo->automotiveNo+1));

	if ((status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR) szBuffer, 0, 0, &hKey)) != ERROR_SUCCESS) 
	{
		RETAILMSG (1,(TEXT("SetStableTime: Couldn't open registry key\r\n")));
		return AIO_INTERNAL_ERROR;
	}

	RETAILMSG(0, (L"SetStableTime: szBuffer = %s \r\n",szBuffer));
	
	if(status = RegSetValueEx(hKey, L"StableTime", 0, REG_DWORD, (const BYTE*)&stableTime, sizeof(DWORD))!=ERROR_SUCCESS)
	{
		RETAILMSG (1,(TEXT("SetStableTime: Couldn't set registry value\r\n")));
		return AIO_INTERNAL_ERROR;
	}

	RegCloseKey(hKey);

	pPinInfo->stableTime = stableTime;

	return ret;
}

//==============================================================================================
//
//  Function:     GetStableTime
//  Description:  Gets pin stable time.
//
//==============================================================================================
DWORD GetStableTime(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,DWORD* stableTime)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT)pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN  )
	{
		RETAILMSG (1,(TEXT("GetStableTime: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}
	
	*stableTime = pPinInfo->stableTime;

	return ret;
}

//==============================================================================================
//
//  Function:     SetInputMode
//  Description:  Sets input pin mode.
//
//==============================================================================================
DWORD SetInputMode(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,PIN_MODE mode)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT)pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN || (mode != FREE && mode!=LOCKED))
	{
		RETAILMSG (1,(TEXT("SetInputMode: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	if(pPinInfo->mode!=FREE && !pOpenCtx->modeOwner[pPinInfo->automotiveNo])
	{
		RETAILMSG (1,(TEXT("SetInputMode: Wrong Access \r\n")));
		return AIO_ERROR_INPUT_IS_LOCKED;
	}

	// LOCK pin
	if (mode == LOCKED)
	{
		pPinInfo->mode = mode;
        pOpenCtx->modeOwner[pPinInfo->automotiveNo] = 1;	
	}
	else // UNLOCK pins (sets free mode)
	{
		pPinInfo->mode = mode;
        pOpenCtx->modeOwner[pPinInfo->automotiveNo] = 0;	
	}

	return ret;	
}

//==============================================================================================
//
//  Function:     GetQueueName
//  Description:  Get queue names for the current pin.
//
//==============================================================================================
DWORD GetQueueName(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinID,PAIO_GET_NAME_CONTEXT nameContext)
{

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT )pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN )
	{
		RETAILMSG (1,(TEXT("GetQueueName: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	nameContext->opErrorCode = ret;

	memcpy(nameContext->daQueueName, pOpenCtx->daQueueName[pPinInfo->automotiveNo],sizeof(pOpenCtx->daQueueName[pPinInfo->automotiveNo]));

	return ret;	
}

//==============================================================================================
//
//  Function:     SetOutputMode
//  Description:  Sets output mode for the current pin
//
//==============================================================================================
DWORD SetOutputMode(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,PIN_MODE mode)
{
	PAIO_OPEN_CONTEXT pCtx;

	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT )pOpenCtx->pAIOHead;
	DWORD ret = AIO_OK;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinNo);

	if(!pPinInfo || pPinInfo->pinType != OUTPUT_PIN || (mode > ACCUMULATED))
	{
		RETAILMSG (1,(TEXT("SetOutputMode: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	RETAILMSG (0,(TEXT(" + SetOutputMode: new mode = 0x%x, prev mode = %x  \r\n"),mode,pPinInfo->mode));
	switch (mode)
	{
		case FREE:
		{
			// release accumulated or locked mode just in the case we are an owner
			if(pPinInfo->mode!=FREE && !pOpenCtx->modeOwner[pPinInfo->automotiveNo])
				ret = AIO_ERROR_OUTPUT_WRONG_ACCESS;
			else
			{
				pPinInfo->mode = mode;
				ResetAccState(pInitCtx,pinNo);
				pOpenCtx->modeOwner[pPinInfo->automotiveNo] = 0;
			}
			break;
		}
		case LOCKED:
		{
			// FREE->         LOCKED - ok
			// LOCKED->       LOCKED - error
			// ACCUMULATED->  LOCKED - ok but we need to release prev modeOwner
            switch (pPinInfo->mode)
			{
			case FREE: // FREE->LOCKED
				pPinInfo->mode = mode;
				pOpenCtx->modeOwner[pPinInfo->automotiveNo] = 1;
				break;
			case LOCKED: // LOCKED->LOCKED
				ret = AIO_ERROR_OUTPUT_IS_LOCKED;
				break;
			case ACCUMULATED: // ACCUMULATED ->LOCKED
				{
					pPinInfo->mode = mode;
					ResetAccState(pInitCtx,pinNo);
					//pPinInfo->accModeMask = 0;

					// release ownerMask flag captureed when setting acuumulated mode
					for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
					{
						pCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];

						if(pCtx != 0 && pCtx->modeOwner[pPinInfo->automotiveNo])
						{
							pCtx->modeOwner[pPinInfo->automotiveNo] = 0;
						}
					}
				        // set owner flag for the current handle
					pOpenCtx->modeOwner[pPinInfo->automotiveNo] = 1;
				}
				break;
				
			default:
				RETAILMSG (1,(TEXT("SetOutputMode: Invalid Parameter \r\n")));
				ret = AIO_ERROR_INVALID_PARAMETER;
				break;
			};
			
			break;
		}
		case ACCUMULATED:
		{
			// release accumulated or locked mode just in the case we are an owner
			if(pPinInfo->mode==FREE)
			{
				pPinInfo->mode = mode;
				ResetAccState(pInitCtx,pinNo);
				//pPinInfo->accModeMask = 0;
				pOpenCtx->modeOwner[pPinInfo->automotiveNo] = 1;

			}
			else // ACUMULATED->ACCUMULATED - error; LOCKED->ACCUMULATED - error
			{
				ret = (pPinInfo->mode==LOCKED)?AIO_ERROR_OUTPUT_IS_LOCKED:AIO_ERROR_OUTPUT_IS_ACCUMULATED;
			}
			break;
		}
		default:
			RETAILMSG (1,(TEXT("SetOutputMode: Invalid Parameter \r\n")));
			ret = AIO_ERROR_INVALID_PARAMETER;
			break;
	};

	RETAILMSG (0,(TEXT(" - SetOutputMode: setted mode = %x , ret value = %x \r\n"),pPinInfo->mode,ret));

	return ret;	
}

//==============================================================================================
//
//  Function:     SetOutputState
//  Description:  Sets output state for the current pin
//
//==============================================================================================
DWORD SetOutputState(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,DWORD state)
{
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT )pOpenCtx->pAIOHead;
	DWORD             ret      = AIO_OK;
	DWORD             newState;

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinNo);

	if(!pPinInfo || pPinInfo->pinType != OUTPUT_PIN )
	{
		RETAILMSG (1,(TEXT("SetOutputState: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	if (pPinInfo->mode == LOCKED && !pOpenCtx->modeOwner[pPinInfo->automotiveNo])
	{
		RETAILMSG (1,(TEXT("SetOutputState: Pin is locked \r\n")));
		return AIO_ERROR_OUTPUT_IS_LOCKED;
	}
	
	if(pPinInfo->mode == ACCUMULATED)
	{
		pOpenCtx->accState[pPinInfo->automotiveNo] = state;
		newState = CalculateAccState(pInitCtx,pPinInfo->pinID);
	}
	else
		newState = state;

	// set newState
	if(newState)
		GPIOSetBit(pInitCtx->hGpio, pPinInfo->gpioID);
	else
		GPIOClrBit(pInitCtx->hGpio, pPinInfo->gpioID);

	RETAILMSG (0,(TEXT(" - SetOutputState: mode = 0x%x, state = 0x%x, newState = 0x%x  \r\n"),pPinInfo->mode,state,newState));

	return ret;
}

//==============================================================================================
//
//  Function:     GetOutputInfo
//  Description:  Gets output pin mode and state
//
//==============================================================================================
DWORD GetOutputInfo(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,DWORD* state,DWORD* mode)
{
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT )pOpenCtx->pAIOHead;
	DWORD             ret      = AIO_OK;
    

	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinNo);

	if(!pPinInfo || pPinInfo->pinType != OUTPUT_PIN || state == NULL || mode == NULL )
	{
		RETAILMSG (1,(TEXT("GetOutputInfo: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);

		return AIO_ERROR_INVALID_PARAMETER;
	}

	*mode = pPinInfo->mode;

	if( !GPIOIoControl(pInitCtx->hGpio, IOCTL_GPIO_GET_OUTPUT_BIT, (UCHAR *)&pPinInfo->gpioID, sizeof(pPinInfo->gpioID), (UCHAR *)state, sizeof(state), 0, 0) )
	{
		RETAILMSG(1, (L"AIODRV:%S, Failed to read output bit\r\n",__FUNCTION__));

		return(AIO_INTERNAL_ERROR);
	}
 
	if( *state )
		*state = HIGH;
	else
		*state = LOW;


	RETAILMSG (0,(TEXT("GetOutputInfo: *mode = 0x%x, *state = 0x%x \r\n"),*mode,*state));

	return ret;
}

//==============================================================================================
//
//  Function:     GetInputInfo
//  Description:  Gets input pin mode and state
//
//==============================================================================================
DWORD GetInputInfo(PAIO_OPEN_CONTEXT pOpenCtx,AUT_PINS pinNo,DWORD* state,DWORD* mode)
{
	PAIO_INIT_CONTEXT pInitCtx = (PAIO_INIT_CONTEXT )pOpenCtx->pAIOHead;
	DWORD             ret      = AIO_OK;
    
	PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinNo);

	if(!pPinInfo || pPinInfo->pinType != INPUT_PIN || state == NULL || mode == NULL )
	{
		RETAILMSG (1,(TEXT("GetInputInfo: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;


	}

	*mode = pPinInfo->mode;
	*state = (GPIOGetBit(pInitCtx->hGpio, pPinInfo->gpioID))?HIGH:LOW;


	RETAILMSG (0,(TEXT("GetInputInfo: *mode = 0x%x, *state = 0x%x \r\n"),*mode,*state));

	return ret;
}

//==============================================================================================
//
//  Function:     GetPinInfo
//  Description:  Gets pin info structure
//
//==============================================================================================
PAIO_PINS_INFO GetPinInfo(PAIO_INIT_CONTEXT pInitCtx,AUT_PINS pinID)
{
	PAIO_PINS_INFO pPinInfo = NULL;

	for (int i = 0 ; i < MAX_PINS_NO; i++)
	{
		if (pInitCtx->pinsInfoTable[i].pinID.mask == pinID.mask)
		{
			pPinInfo = &pInitCtx->pinsInfoTable[i];
			break;

		}			
	}
	return pPinInfo;
}


//==============================================================================================
//
//  Function:     SetInitConfig
//  Description:  Set initial configuration for automotive pins
//
//==============================================================================================
 DWORD SetInitConfig(PAIO_INIT_CONTEXT pCxt)
 {
	 DWORD ret = AIO_OK;

	 if (pCxt->hGpio == NULL)
	 {
         RETAILMSG(1, (L"ERROR: SetInitConfig: " L"Failed to open Gpio driver \r\n"));
         return AIO_INTERNAL_ERROR;
	 }


	 RETAILMSG(0, (L"AIO SetInitConfig: " L"started \r\n"));
	 for ( int i = 0; i < MAX_PINS_NO; i++)
	 {
		
		if(pCxt->pinsInfoTable[i].pinType == INPUT_PIN)
		{

			RETAILMSG(0, (L"AIO SetInitConfig: " L"GPIOSetMode %d input for gpio %d:  \r\n",i, pCxt->pinsInfoTable[i].gpioID));
			GPIOSetMode(pCxt->hGpio, pCxt->pinsInfoTable[i].gpioID, GPIO_DIR_INPUT | GPIO_INT_LOW_HIGH | GPIO_INT_HIGH_LOW);
		}
		else
		{
			RETAILMSG(0, (L"AIO SetInitConfig: " L"GPIOSetMode %d output for gpio %d:  \r\n",i, pCxt->pinsInfoTable[i].gpioID));
			GPIOSetMode(pCxt->hGpio, pCxt->pinsInfoTable[i].gpioID, GPIO_DIR_OUTPUT);
			//set off output pin
			GPIOClrBit(pCxt->hGpio, pCxt->pinsInfoTable[i].gpioID);
		}
	 }

	 return ret;
 }


//==============================================================================================
//
//  Function:     ResetAccState
//  Description:  Set accumulated state to be 0 for all the handles
//
//==============================================================================================

 DWORD ResetAccState(PAIO_INIT_CONTEXT pInitCtx,AUT_PINS pinNo)
 {
	 PAIO_OPEN_CONTEXT pOpenCtx;

	 PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinNo);

	 if(!pPinInfo || pPinInfo->pinType != OUTPUT_PIN )
	 {
		RETAILMSG (1,(TEXT("SetOutputState: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	 }

	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		pOpenCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];

		if(pOpenCtx != 0)
			pOpenCtx->accState[pPinInfo->automotiveNo] = 0;
	}

	return AIO_OK;
}


//==============================================================================================
//
//  Function:     CalculateAccState
//  Description:   Calculates accumulated state.
//
//==============================================================================================
 DWORD CalculateAccState(PAIO_INIT_CONTEXT pInitCtx,AUT_PINS pinID)
 {
	 PAIO_OPEN_CONTEXT pOpenCtx;
	 DWORD             calcState = 0;

	 PAIO_PINS_INFO pPinInfo=GetPinInfo(pInitCtx,pinID);

	 if(!pPinInfo || pPinInfo->pinType != OUTPUT_PIN )
	 {
		RETAILMSG (1,(TEXT("SetOutputState: Invalid Parameter \r\n")));

		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;


	 }

		
	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		pOpenCtx = (PAIO_OPEN_CONTEXT )pInitCtx->OpenContexts[i];

		if(pOpenCtx != 0)
		{
			calcState |= pOpenCtx->accState[pPinInfo->automotiveNo];
		}
	}

	return calcState;
}