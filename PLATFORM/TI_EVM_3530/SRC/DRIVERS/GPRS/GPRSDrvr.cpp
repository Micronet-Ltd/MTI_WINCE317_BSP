// Copyright 2010 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//   File:  GPSDrvr.cpp
//
//   GPRS Driver
//  
//
//   Created by Anna Rayer  June 2010
//-----------------------------------------------------------------------------/*



#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>
#include <gpio.h>
#include <bsp.h>

#include <args.h>
#include <pegdser.h>
#include <devload.h>
#include "gprs.h"

//#include <aio_api.h>
//#include "aio.h"
#include <gsm_api.h>
#include <gsm710_api.h>
#include <gsm0710_ioctrl.h>
#include "voiceControl.h"
#include "ftdi_ioctl.h"
#include "usbser_api.h"

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
//            Local Definitions
//==============================================================================================




#define GPRS_MSG                                  0
#define MODEM_DEVICE_COOKIE                      'mdmS'
//#define FTDI_ID                                   L"MOM0:"
#define MIN_MODEM_BATTERY_LEVEL                   10


//#define PCA_GPIO_GPRS_0                        PCA_GPIO_80

#define DEVACCESS_MODEMEXT_MODE                   0x10000000
#define MODEM_CONFIG_LOW_POWER_ENABLED            0x00000001

#define MAX_POWER_SAVE_ATTEMPTS                   5000


//==============================================================================================
//            Error Definitions
//==============================================================================================

//==============================================================================================
//            structure definitions
//==============================================================================================

typedef struct _VCOM_INIT_CONTEXT
{
	UINT32               cookie;
	DWORD                powerMask;
    CEDEVICE_POWER_STATE powerState;
	DWORD                dwModemVersion;
	DWORD                dwModemPowerSaveMode;
	UINT32				 OpenContexts[32];
//	DCB                  lastCommDCB;
	
//	HANDLE               hFTDISer;
	DWORD                modemWakeup;  
	DWORD                priority256;
	DWORD                isModemOn;
	HANDLE               pAccessOwner;
	BOOL                 bModemWakeupOn;
	HANDLE               hWakeupEvent;
	HANDLE               hWakeupThread; 
    HANDLE               hModemOnOffEvent; // ##### Michael. workaround for WLAN/BT power down mode
	BOOL                 quit;

	HANDLE               hGpio;
	BOOL                 dwLowPowerFlag;
	
	CRITICAL_SECTION     OpenCS;

	BOOL				 isControlledByMux;
	HANDLE				 hMuxDeactivatingThread;
	TCHAR				 szFtdiPort[8];
	DWORD				 enableExternalModem;

} VCOM_INIT_CONTEXT, *PVCOM_INIT_CONTEXT;


//------------------------------------------------------------------------------
typedef struct __VCOM_OPEN_CONTEXT {

    PVCOM_INIT_CONTEXT	 pHead;					// @field Pointer back to our VCOM_INIT_CONTEXT
	DWORD                dwAccess;
	DWORD                dwShareMode;
	DCB                  lastCommDCB;
	HANDLE               hFTDISer;
	DWORD				 lastCommMask;
	COMMTIMEOUTS		 lastCommTimeouts;
} VCOM_OPEN_CONTEXT, *PVCOM_OPEN_CONTEXT;



//==============================================================================================
//            Device registry parameters
//==============================================================================================


static const DEVICE_REGISTRY_PARAM s_deviceInitRegParams[] = {  
	{
        L"PowerMask", PARAM_DWORD, TRUE, offset(VCOM_INIT_CONTEXT, powerMask),
        fieldsize(VCOM_INIT_CONTEXT, powerMask), NULL
    },
	{
        L"ModemWakeup", PARAM_DWORD, TRUE, offset(VCOM_INIT_CONTEXT, modemWakeup),
        fieldsize(VCOM_INIT_CONTEXT, modemWakeup), NULL
    },
	{
        L"Priority256", PARAM_DWORD, TRUE, offset(VCOM_INIT_CONTEXT, priority256),
        fieldsize(VCOM_INIT_CONTEXT, priority256), (VOID*)150
    },
    {
        L"EnableExternalModem", PARAM_DWORD, FALSE, offset(VCOM_INIT_CONTEXT, enableExternalModem),
        fieldsize(VCOM_INIT_CONTEXT, enableExternalModem), (VOID*)0
    }

};
//==============================================================================================
//           Local Functions
//==============================================================================================

BOOL   COM_Deinit ( DWORD dwContext );
BOOL   COM_Close  ( DWORD Handle );
BOOL   SetComPort(PVCOM_OPEN_CONTEXT pCxt);
DWORD  WaitIfPowerSaveMode(PVCOM_INIT_CONTEXT pInitCxt);
BOOL   SetPowerState(PVCOM_INIT_CONTEXT pCxt,CEDEVICE_POWER_STATE    power);
//BOOL   SetModemPowerSaveMode(PVCOM_INIT_CONTEXT pInitCxt,BOOL bSaveMode);
//BOOL   SendSaveModeCmd(PVCOM_INIT_CONTEXT pInitCxt);
//BOOL   SendSaveModeOffCmd(PVCOM_INIT_CONTEXT pInitCxt);

DWORD  CmdGetResponse(HANDLE    hDevice);
BOOL   SendCommand(HANDLE hDev,CHAR const *pszCommand);

DWORD SetModemInitConfig(PVCOM_INIT_CONTEXT pCxt);
BOOL  ModemONProc(PVCOM_INIT_CONTEXT pInitCtx);
BOOL  ModemOFFProc(PVCOM_INIT_CONTEXT pInitCtx);
DWORD WaitForCOMPort(DWORD dwAttempts, PVCOM_INIT_CONTEXT pInitCtx);

BOOL  ModemResetProc(PVCOM_INIT_CONTEXT pCxt);

BOOL SetModemWakeup(PVCOM_INIT_CONTEXT pInitCtx, BOOL  flag);
DWORD GetModemPowerState(HANDLE hHandle, DWORD* state);
DWORD SetModemPowerState(HANDLE hHandle, DWORD state);
DWORD WakeupThread(VOID *pContext);
DWORD SetupPDD(HANDLE hHandle, DWORD state);

BOOL ModemON(PVCOM_INIT_CONTEXT pInitCtx);
BOOL ModemOFF(PVCOM_INIT_CONTEXT pInitCtx);


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
//                              the GPS_OPEN and GPS_DEINIT entry points as a device handle.
DWORD COM_Init ( LPCTSTR pContext )
{
	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "GPRS: +COM_Init, pContext=%s\r\n" ), pContext ) );
	RETAILMSG(GPRS_MSG, (L"GPRS:+%S, pContext=%s\r\n",__FUNCTION__, pContext));
	
	VCOM_INIT_CONTEXT   *pCxt =0;
	card_ver			 modem_cv;
	UINT32               in;
	
	// allocate VCOM context object
	if( (pCxt = (VCOM_INIT_CONTEXT *)LocalAlloc(LPTR, sizeof(VCOM_INIT_CONTEXT))) == NULL )
	{
		return 0;
	}

	memset(pCxt, 0, sizeof(VCOM_INIT_CONTEXT));

     // Set cookie
    pCxt->cookie = MODEM_DEVICE_COOKIE;

	// Read device parameters
    if (GetDeviceRegistryParams(pContext, pCxt, dimof(s_deviceInitRegParams), s_deviceInitRegParams) != ERROR_SUCCESS)
	{
        RETAILMSG(GPRS_MSG, (L"ERROR: VCOM_Init: Failed read VCOM_INIT_CONTEXT driver registry parameters\r\n"));
        LocalFree(pCxt);
		return 0;
	}

	if (pCxt->enableExternalModem)
	{
		RETAILMSG(GPRS_MSG, (L"VCOM_Init: external modem is enabled, exit\r\n"));
        LocalFree(pCxt);
		return 0;
	}
	else
	{
		wcscpy(pCxt->szFtdiPort, TEXT("MOM0:"));
		in = OAL_ARGS_QUERY_MODEM_CFG;
		if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &modem_cv, sizeof(modem_cv), 0))
		{
			RETAILMSG(GPRS_MSG, (L"GPS_Init: IOCTL_HAL_QUERY_CARD_CFG calling failure..\r\n"));
			LocalFree(pCxt);
			return 0;
		}

		if( modem_cv.ver == -1 )
		{
			RETAILMSG(GPRS_MSG, (L"VCOM_Init: internal modem is not present\r\n"));
			LocalFree(pCxt);
			return 0;
		}

		pCxt->dwModemVersion = modem_cv.ver;
	}

	pCxt->dwModemPowerSaveMode = 0;
	pCxt->isModemOn=0;
	pCxt->pAccessOwner = NULL;
	pCxt->bModemWakeupOn = FALSE;
	pCxt->dwLowPowerFlag = FALSE;



	pCxt->hWakeupEvent=CreateEvent(NULL, FALSE, FALSE, NULL);
		 
    if(NULL == pCxt->hWakeupEvent)
    {
		RETAILMSG(GPRS_MSG, (TEXT("Error: Modem.CreateEvent failed. error = %x\r\n"), GetLastError()));
		LocalFree(pCxt);
        return 0;
    }

	pCxt->quit=FALSE;
	pCxt->hWakeupThread = CreateThread(0, 0, WakeupThread, (LPVOID)pCxt, 0, 0);
	if(pCxt->hWakeupThread == 0)
	{
		RETAILMSG(GPRS_MSG, (L"ERROR: Modem: CreateThread failed 0x%x\r\n",GetLastError()));
		CloseHandle(pCxt->hWakeupEvent);
		LocalFree(pCxt);
		return 0;
	}


	// Open gpio driver
    pCxt->hGpio = GPIOOpen();
    if (pCxt->hGpio == NULL)
    {
        RETAILMSG(GPRS_MSG, (L"ERROR: COM_Init: " L"Failed to open Gpio driver \r\n"));
		COM_Deinit((DWORD)pCxt);
        return NULL;
    }

 	
   if(pCxt->dwModemVersion!=-1)
   {
	   SetModemInitConfig(pCxt);
   }
  
	

	// set all the open contexts to be null

	for(int i = 0; i < (sizeof(pCxt->OpenContexts)/sizeof(pCxt->OpenContexts[0])); i++)
	{
		pCxt->OpenContexts[i] = 0;
	}



	RETAILMSG(GPRS_MSG, (L"GPRS:%S - VCOM_Init modem version = %d\r\n",__FUNCTION__, pCxt->dwModemVersion));

	pCxt->hModemOnOffEvent = CreateEvent(0, 1, 0, L"ModemOnOffEvent");
	if( pCxt->hModemOnOffEvent == NULL )
      return 0;

	InitializeCriticalSection(&(pCxt->OpenCS));

	RETAILMSG(GPRS_MSG, (L"GPRS:-%S\r\n",__FUNCTION__));
	return (DWORD)pCxt;

}


//------------------------------------------------------------------------------
// Function name	: COM_Deinit
// Description	    : DeInitialization of all used resources
// Return type		: BOOL  
// Argument         : DWORD dwContext
//------------------------------------------------------------------------------
BOOL COM_Deinit ( DWORD dwContext )
{
	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "+COM_Deinit, dwContext=%d\r\n" ), dwContext ) );
	RETAILMSG(GPRS_MSG, (L"GPRS:+%S, dwContext=%s\r\n",__FUNCTION__, dwContext));
    VCOM_INIT_CONTEXT *pInitCtx = (VCOM_INIT_CONTEXT *)dwContext;
	PVCOM_OPEN_CONTEXT pOpenCtx;


	 // Check if we get correct context
    if(!pInitCtx || (pInitCtx->cookie != MODEM_DEVICE_COOKIE))
	{
        RETAILMSG(GPRS_MSG, (L"ERROR: COM_Deinit: Incorrect context parameter\r\n"));
        return 0;
	}

	if (pInitCtx->hMuxDeactivatingThread)
	{
		DWORD dwReason = WaitForSingleObject(pInitCtx->hMuxDeactivatingThread, 2000);
		if (dwReason != WAIT_OBJECT_0)
			TerminateThread(pInitCtx->hMuxDeactivatingThread, 0);

		CloseHandle(pInitCtx->hMuxDeactivatingThread);
		pInitCtx->hMuxDeactivatingThread = NULL;
	}

	pInitCtx->quit = TRUE;

	if (pInitCtx->hWakeupEvent)
		CloseHandle(pInitCtx->hWakeupEvent);

	EnterCriticalSection(&(pInitCtx->OpenCS));
	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		pOpenCtx = (PVCOM_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
		if (pOpenCtx)
		{
			COM_Close((DWORD)pOpenCtx);
		}
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	DeleteCriticalSection(&(pInitCtx->OpenCS));




	  // Close Gpio 
	if(pInitCtx->hGpio)
		CloseHandle(pInitCtx->hGpio);
	// Terminate Thread


	if (pInitCtx->hWakeupThread)
	{

		DWORD dwReason = WaitForSingleObject(pInitCtx->hWakeupThread, 100);
		if( dwReason != WAIT_OBJECT_0 )
		{
			TerminateThread(pInitCtx->hWakeupThread, 0);
			RETAILMSG(GPRS_MSG, (L"COM_Deinit: WaitForSingleObject %d\r\n", dwReason));
		}
		CloseHandle(pInitCtx->hWakeupThread);
		pInitCtx->hWakeupThread = NULL;
	}


	CloseHandle(pInitCtx->hModemOnOffEvent);
	pInitCtx->hModemOnOffEvent = NULL;

   // if (pInitCtx->hFTDISer && pInitCtx->hFTDISer!=INVALID_HANDLE_VALUE) 
	//	CloseHandle(pInitCtx->hFTDISer);
	LocalFree(pInitCtx);


	RETAILMSG(GPRS_MSG, (L"GPRS:-%S\r\n",__FUNCTION__));
	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "GPRS: -COM_Deinit\r\n" ) ) );

    return TRUE;
}


//------------------------------------------------------------------------------
// Function name	: COM_IOControl
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
BOOL COM_IOControl ( DWORD  hHandle, 
					 DWORD  dwIoControlCode, 
					 PBYTE  pInBuf, 
					 DWORD  nInBufSize, 
					 PBYTE  pOutBuf,
					 DWORD  nOutBufSize,
					 PDWORD pBytesReturned )

{
    BOOL rc = FALSE;
    VCOM_OPEN_CONTEXT *pOpenCtx = (VCOM_OPEN_CONTEXT*)hHandle;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pOpenCtx->pHead;


    RETAILMSG(GPRS_MSG, (
        L"GPRS:+%S(0x%08x, 0x%08x, 0x%08x, %d, 0x%08x, %d, 0x%08x)\r\n",__FUNCTION__,
        hHandle, dwIoControlCode, pInBuf, nInBufSize, pOutBuf, nOutBufSize, pBytesReturned
        ));
        
    // Check if we get correct context
    if ((pInitCtx == NULL) || (pInitCtx->cookie != MODEM_DEVICE_COOKIE))
        {
        RETAILMSG(GPRS_MSG, (L"ERROR: COM_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        return FALSE;
        }

	

	switch (dwIoControlCode)
        {
        case IOCTL_POWER_CAPABILITIES: 
            RETAILMSG(GPRS_MSG, (L"GPRS:%S: Received IOCTL_POWER_CAPABILITIES\r\n",__FUNCTION__));
            if (pOutBuf && nOutBufSize >= sizeof (POWER_CAPABILITIES) && pBytesReturned) 
                {
                    __try 
                        {
                        PPOWER_CAPABILITIES PowerCaps;
                        PowerCaps = (PPOWER_CAPABILITIES)pOutBuf;
         
                        // Only supports D0 (permanently on) and D4(off.         
                        memset(PowerCaps, 0, sizeof(*PowerCaps));
                        PowerCaps->DeviceDx = (UCHAR)pInitCtx->powerMask;
                        *pBytesReturned = sizeof(*PowerCaps);

						RETAILMSG(GPRS_MSG, (L"GPRS:%S:PowerCaps->DeviceDx = %d\r\n",__FUNCTION__,PowerCaps->DeviceDx));
                        
                        rc = TRUE;
                        }

                    __except(EXCEPTION_EXECUTE_HANDLER) 
                        {
							RETAILMSG(GPRS_MSG, (L"GPRS:%S:exception in ioctl\r\n",__FUNCTION__));
                        }
                }
            break;

        // determines whether changing power state is feasible
        case IOCTL_POWER_QUERY: 
            RETAILMSG(GPRS_MSG,(L"GPRS:%S: Received IOCTL_POWER_QUERY\r\n",__FUNCTION__));
            if (pOutBuf && nOutBufSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                // Return a good status on any valid query, since we are 
                // always ready to change power states (if asked for state 
                // we don't support, we move to next highest, eg D3->D4).
                __try 
                    {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
 
                    if (VALID_DX(ReqDx)) 
                        {
                        // This is a valid Dx state so return a good status.
                        rc = TRUE;
                        }
 
                    RETAILMSG(GPRS_MSG, (L"GPRS:%S IOCTL_POWER_QUERY %d\r\n",__FUNCTION__));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
						RETAILMSG(GPRS_MSG, (L"GPRS:%S: Exception in ioctl\r\n",__FUNCTION__));
                    }
                }
            break;

        // requests a change from one device power state to another
        case IOCTL_POWER_SET: 
            RETAILMSG(GPRS_MSG,(L"GPRS:%S: Received IOCTL_POWER_SET...\r\n",__FUNCTION__));
            if (pOutBuf && nOutBufSize >= sizeof(CEDEVICE_POWER_STATE)&& pBytesReturned) 
                {
                __try 
                    {
                    CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pOutBuf;
 
                    if (SetPowerState(pInitCtx, ReqDx))
                        {   
                        *(PCEDEVICE_POWER_STATE)pOutBuf = pInitCtx->powerState;
                        *pBytesReturned = sizeof(CEDEVICE_POWER_STATE);
 
                        rc = TRUE;
                        RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_POWER_SET to D%u \r\n",__FUNCTION__,pInitCtx->powerState));
                        }
                    else 
                        {
                        RETAILMSG(GPRS_MSG, (L"GPRS:%S:Invalid state request D%u \r\n",__FUNCTION__, ReqDx ));
                        }
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
						RETAILMSG(GPRS_MSG, (L" GPRS:%S: Exception in ioctl\r\n",__FUNCTION__));
                    }
            }
            break;

        // gets the current device power state
        case IOCTL_POWER_GET: 
            RETAILMSG(GPRS_MSG, (L"GPRS:%S: Received IOCTL_POWER_GET\r\n",__FUNCTION__));
            if (pOutBuf && nOutBufSize >= sizeof(CEDEVICE_POWER_STATE)) 
                {
                __try 
                    {
                    *(PCEDEVICE_POWER_STATE)pOutBuf = pInitCtx->powerState;
 
                    rc = TRUE;

                     RETAILMSG(GPRS_MSG, (L"GPRS:%S:IOCTL_POWER_GET to D%u \r\n",__FUNCTION__,pInitCtx->powerState));
                    }
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
					 RETAILMSG(GPRS_MSG, (L"GPRS:%S: Exception in ioctl\r\n",__FUNCTION__));
                    }
                }     
            break;
		
		case IOCTL_SET_MODEM_POWER_STATE:
			 RETAILMSG(GPRS_MSG,(L"GPRS:%S: Modem: Received IOCTL_SET_MODEM_POWER_STATE.\r\n",__FUNCTION__));
            if (pInBuf && nInBufSize >= sizeof(DWORD)) 
                {
                __try 
                    {
						DWORD dwPowerState = *(DWORD*)pInBuf;
						DWORD ret = SetModemPowerState(pOpenCtx,dwPowerState);
						if (pOutBuf && nOutBufSize >= sizeof(DWORD))
						{
							*(DWORD*)pOutBuf = ret;
							rc = TRUE;
						}
						else
							rc = FALSE;
						
					}
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
						RETAILMSG(1, (L"GPRS:%S: Exception in ioctl IOCTL_SET_MODEM_POWER_STATE.\r\n",__FUNCTION__));
                    }
                }  
 
			break;
		case IOCTL_GET_MODEM_POWER_STATE:
			RETAILMSG(GPRS_MSG,(L"GPRS:%S: Modem: Received IOCTL_GET_MODEM_POWER_STATE.\r\n",__FUNCTION__));
            if (pOutBuf && nOutBufSize >= sizeof(DWORD)) 
                {
#if 1
                __try 
                    {
						DWORD ret = GetModemPowerState(pOpenCtx,(DWORD*)pOutBuf);
						rc = (ret) ? FALSE:TRUE;
						
					}
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
						RETAILMSG(1, (L"GPRS:%S: Exception in ioctl IOCTL_SET_MODEM_POWER_STATE.\r\n",__FUNCTION__));
                    }
#endif

                }  
 
			break;
		case IOCTL_SET_MODEM_CONFIG:
			 RETAILMSG(GPRS_MSG,(L"GPRS:%S: Modem: Received IOCTL_SET_MODEM_CONFIG.\r\n",__FUNCTION__));
            if (pInBuf && nInBufSize >= sizeof(DWORD)) 
                {
                __try 
                    {
						EnterCriticalSection(&(pInitCtx->OpenCS));
						DWORD dwFlags = *(DWORD*)pInBuf;
						if(dwFlags & MODEM_CONFIG_LOW_POWER_ENABLED)
						{
							pInitCtx->dwLowPowerFlag = TRUE;
						}
						else 
						{
							pInitCtx->dwLowPowerFlag = FALSE;
						}
						rc = TRUE;
						
						LeaveCriticalSection(&(pInitCtx->OpenCS));
					}
                __except(EXCEPTION_EXECUTE_HANDLER) 
                    {
						RETAILMSG(GPRS_MSG, (L"GPRS:%S: Exception in ioctl IOCTL_SET_MODEM_CONFIG.\r\n",__FUNCTION__));
                    }
                }  
 
			break;


		case IOCTL_SERIAL_WAIT_ON_MASK :
			{
				RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_SERIAL_WAIT_ON_MASK started.pOpenCtx = %x \r\n",__FUNCTION__,pOpenCtx));
				if(pInitCtx->isModemOn)
				{

					WaitIfPowerSaveMode(pInitCtx);
					SetComPort(pOpenCtx);
					rc = DeviceIoControl(pOpenCtx->hFTDISer,dwIoControlCode, 
													 pInBuf, 
													 nInBufSize, 
													 pOutBuf,
													 nOutBufSize,
													 pBytesReturned,
													 NULL);

					if(*pOutBuf ==0)
					{
						RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_SERIAL_WAIT_ON_MASK:  *pOutBuf = %x, rc = %x error - %x \r\n",__FUNCTION__, (*(DWORD *)pOutBuf)&0xFFFF,rc,GetLastError()));
					}
					// if during wait on mask modem turned off or entered to power save modem then we need 
					// to send disconnect notification
					if(!pInitCtx->isModemOn || pInitCtx->dwModemPowerSaveMode)
					{
						DWORD dwValue = EV_RLSD | EV_DSR;
						CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
						dwValue = sizeof(DWORD);
						CeSafeCopyMemory( pBytesReturned, &dwValue, sizeof(DWORD) );
						RETAILMSG(1, (L"GPRS:%S: Set RLSD event. pOpenCtx = %x \r\n",__FUNCTION__,pOpenCtx));
						SetLastError(ERROR_SUCCESS);
						rc = TRUE;
					}
				}
				else
				{
						*pOutBuf =0;
						RETAILMSG(1, (L"GPRS:%S: Set RLSD event. pOpenCtx = %x \r\n",__FUNCTION__,pOpenCtx));
						DWORD dwValue = EV_RLSD | EV_DSR;
						CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
						dwValue = sizeof(DWORD);
						CeSafeCopyMemory( pBytesReturned, &dwValue, sizeof(DWORD) );

						SetLastError(ERROR_SUCCESS);
					RETAILMSG(1, (L"GPRS:%S: Modem not ready. pOpenCtx = %x \r\n",__FUNCTION__,pOpenCtx));
						rc = TRUE;
				}
			}
			break;
		case IOCTL_SERIAL_GET_MODEMSTATUS :
			{
				DWORD dwValue;
				RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_SERIAL_GET_MODEMSTATUS started. pOpenCtx = %x\r\n",__FUNCTION__,pOpenCtx));
				if(pInitCtx->isModemOn)
				{

					// if the modem is in power save mode then send "disconnected" modem status
					if (pInitCtx->dwModemPowerSaveMode )
					{
						RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_SERIAL_GET_MODEMSTATUS: ~MS_RLSD_ON returned. pOpenCtx - %x\r\n",__FUNCTION__ ,pOpenCtx));
						dwValue = 0;//~MS_RLSD_ON;
						CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
						dwValue = sizeof(DWORD);
						CeSafeCopyMemory( pBytesReturned, &dwValue, sizeof(DWORD));
						rc = TRUE;
					}
					else
					{
						WaitIfPowerSaveMode(pInitCtx);
						SetComPort(pOpenCtx);
						rc = DeviceIoControl(pOpenCtx->hFTDISer,dwIoControlCode, 
														 pInBuf, 
														 nInBufSize, 
														 pOutBuf,
														 nOutBufSize,
														 pBytesReturned,
														 NULL);
						// if during wait on mask modem turned off then we need 
						// to put disconnect error
						RETAILMSG(pOutBuf, (L"GPRS:%S: IOCTL_SERIAL_GET_MODEMSTATUS pOpenCtx = %x, %X\r\n",__FUNCTION__,pOpenCtx, (*(DWORD *)pOutBuf)&0xFFFF));
						if(!pInitCtx->isModemOn)
						{
							dwValue = 0;//~MS_RLSD_ON;
							CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
							dwValue = sizeof(DWORD);
							CeSafeCopyMemory( pBytesReturned, &dwValue, sizeof(DWORD));
							SetLastError(ERROR_SUCCESS);
							rc = 1;
						}
					}
				}
				else
				{
					RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_SERIAL_GET_MODEMSTATUS: Modem off. pOpenCtx - %x\r\n",__FUNCTION__ ,pOpenCtx));
					dwValue = 0;//~MS_RLSD_ON;
					CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
					dwValue = sizeof(DWORD);
					CeSafeCopyMemory( pBytesReturned, &dwValue, sizeof(DWORD));
					SetLastError(ERROR_SUCCESS);
					rc = TRUE;	
				}
			}
			break;
		case IOCTL_SET_MUX_STATUS:
			{
				if (pInBuf && nInBufSize >= sizeof(BOOL)) 
                {
					pOpenCtx->pHead->isControlledByMux = (BOOL)(*pInBuf);
					rc = TRUE;
				}
			}
			break;
		case IOCTL_GET_EXTERNAL_MODEM_STATE:
			{
				RETAILMSG(GPRS_MSG, (L"GPRS:%S: IOCTL_GET_EXTERNAL_MODEM_STATE: External modem enable = %d\r\n", __FUNCTION__ , pInitCtx->enableExternalModem));
				if (pOutBuf && nOutBufSize >= sizeof(pInitCtx->enableExternalModem))
				{
					CeSafeCopyMemory(pOutBuf, &(pInitCtx->enableExternalModem), sizeof(pInitCtx->enableExternalModem));
					SetLastError(ERROR_SUCCESS);
					rc = TRUE;	
				}
			}
			break;
		default:
			{
				DWORD dwValue = 0;

				RETAILMSG(GPRS_MSG, (L"GPRS:%S: 'default' IOCTL code = %X,  pOpenCtx = %x\r\n",__FUNCTION__, dwIoControlCode, pOpenCtx));
				if(pInitCtx->isModemOn)
				{
                    RETAILMSG(GPRS_MSG, (L"GPRS:%S: 'default' IOCTL code = %X, call FTDISer driver - %x\r\n",__FUNCTION__, dwIoControlCode,pOpenCtx->hFTDISer));

					if (IOCTL_SERIAL_SET_WAIT_MASK == dwIoControlCode)
					{
						RETAILMSG(pInBuf, (L"GPRS:%S: IOCTL_SERIAL_SET_WAIT_MASK: %x\r\n",__FUNCTION__ ,  *(DWORD *)pInBuf&0xFFFF));
						pOpenCtx->lastCommMask = *(DWORD *)pInBuf;
					}
					else if (IOCTL_SERIAL_SET_TIMEOUTS == dwIoControlCode)
					{
						RETAILMSG(pInBuf, (L"GPRS:%S: IOCTL_SERIAL_SET_TIMEOUTS\r\n",__FUNCTION__ ));
						CeSafeCopyMemory( &pOpenCtx->lastCommTimeouts, pInBuf, sizeof(pOpenCtx->lastCommTimeouts) );
					}

					WaitIfPowerSaveMode(pInitCtx);
					SetComPort(pOpenCtx);
					rc = DeviceIoControl(pOpenCtx->hFTDISer,dwIoControlCode, 
													 pInBuf, 
													 nInBufSize, 
													 pOutBuf,
													 nOutBufSize,
													 pBytesReturned,
													 NULL);
				}
				// if during wait on mask modem turned off then we need 
				// to put disconnect error
				if(!pInitCtx->isModemOn)
				{
					if(IOCTL_SERIAL_GET_WAIT_MASK == dwIoControlCode)
					{
						dwValue = 0;
						CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
						dwValue = sizeof(DWORD);
						CeSafeCopyMemory( pBytesReturned, &dwValue, sizeof(DWORD));
						SetLastError(ERROR_SUCCESS);
						rc = 1;
					}
					else if(IOCTL_SERIAL_SET_WAIT_MASK	== dwIoControlCode	||
							IOCTL_SERIAL_PURGE			== dwIoControlCode	)
					{
						SetLastError(ERROR_SUCCESS);
						rc = 1;
					}
					else
					{
					RETAILMSG(GPRS_MSG, (L"GPRS:%S: 'default' IOCTL code = %X, Modem is Off\r\n",__FUNCTION__, dwIoControlCode));
						SetLastError(ERROR_NOT_READY);
						rc = FALSE;
					}
				}
				

			}
			break;

        }
		

	return rc;
}




//------------------------------------------------------------------------------
// Function name	: COM_Open
// Description	    : DeInitialization of all used resources
// Return type		: DWORD  
// Argument         : 
//------------------------------------------------------------------------------

DWORD COM_Open ( DWORD dwContext, DWORD dwAccess, DWORD dwShareMode )
{
	VCOM_INIT_CONTEXT *pInitCtx = (VCOM_INIT_CONTEXT*)dwContext;
	PVCOM_OPEN_CONTEXT pOpenCtx;

    RETAILMSG(0, (L"+COM_Open. dwAccess=%x,  dwShareMode=%x\r\n",dwAccess,dwShareMode));
	DWORD    rc = 0;
    RETAILMSG(1, (L"GPRS:+%S, dwAccess=%x,  dwShareMode=%x\r\n",__FUNCTION__, dwAccess, dwShareMode));
        
    // Check if we get correct context
    if ((pInitCtx == NULL) || (pInitCtx->cookie != MODEM_DEVICE_COOKIE))
        {
        RETAILMSG(GPRS_MSG, (L"ERROR: COM_IOControl: "
            L"Incorrect context parameter\r\n"
            ));
        return 0;
        }

	if (dwAccess & DEVACCESS_BUSNAMESPACE ) {
        dwAccess &=~(GENERIC_READ |GENERIC_WRITE|GENERIC_EXECUTE|GENERIC_ALL);
    }

    // Return NULL if opening with access & someone else already has
    if ( (dwAccess & (GENERIC_READ | GENERIC_WRITE)) && pInitCtx->pAccessOwner ) {
        RETAILMSG(GPRS_MSG,(TEXT("Open requested access %x, handle x%X already has read/write permissions\r\n"),
                   dwAccess, pInitCtx->pAccessOwner));
        SetLastError(ERROR_INVALID_ACCESS);
        return(NULL);
    }

	if(!pInitCtx->isModemOn && (dwAccess & (GENERIC_READ | GENERIC_WRITE)))
	{
		RETAILMSG (GPRS_MSG,(TEXT("GPRS: COM_Open: device was powered off\r\n")));
		SetLastError(ERROR_NOT_READY);
		return(FALSE);
	}

	EnterCriticalSection(&(pInitCtx->OpenCS));

	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		if(pInitCtx->OpenContexts[i] == 0)
		{
			// Create device handle

			 // OK, lets allocate an open structure
			pOpenCtx    =  (PVCOM_OPEN_CONTEXT)LocalAlloc(LPTR, sizeof(VCOM_OPEN_CONTEXT));
			
			if(!pOpenCtx)
			{
				RETAILMSG(GPRS_MSG, (L"GPRS ERROR: COM_Open: Failed allocate open context driver structure\r\n"));

				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				rc = 0;
				break;
			}

			memset(pOpenCtx, 0, sizeof(VCOM_OPEN_CONTEXT));
			pOpenCtx->hFTDISer = INVALID_HANDLE_VALUE;
			memset (&pOpenCtx->lastCommDCB,0,sizeof(DCB));

			pOpenCtx->dwAccess    = dwAccess;
			pOpenCtx->dwShareMode = dwShareMode;
			// if we have access permissions, note it in pAccessOwner
			if ( pOpenCtx->dwAccess & (GENERIC_READ | GENERIC_WRITE) ) 
			{
				RETAILMSG(GPRS_MSG,(TEXT("COM_Open: Access permission handle granted x%X\n\r"), pOpenCtx));
				pInitCtx->pAccessOwner = pOpenCtx;
			}
			pOpenCtx->pHead = pInitCtx;  // pointer back to our parent
	     
			pInitCtx->OpenContexts[i] = (UINT32)pOpenCtx;
			rc = (DWORD)pOpenCtx;


			//RETAILMSG(GPRS_MSG, (L"COM_Open: create modem COM virtual port dwAccess = %x\r\n",dwAccess));

			//if modem is already turned on, then we need to associate this handle with ftdi virtual com driver
			if(pInitCtx->isModemOn)
			{
				pOpenCtx->hFTDISer = CreateFile(pInitCtx->szFtdiPort, dwAccess, dwShareMode, NULL,
									OPEN_EXISTING,0,NULL);

				RETAILMSG(GPRS_MSG, (L"COM_Open: create modem COM virtual port dwAccess = %x,pOpenCtx->hFTDISer - %x \r\n",dwAccess,pOpenCtx->hFTDISer));
				if(pOpenCtx->hFTDISer==INVALID_HANDLE_VALUE)
				{
					RETAILMSG(GPRS_MSG, (L"GPRS ERROR: COM_Open: Failed to create pOpenCtx->hFTDISer\r\n"));

					COM_Close((DWORD)pOpenCtx);
					SetLastError(ERROR_OPEN_FAILED);
					rc = 0;
				}
			}

			
			break;
		}
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	RETAILMSG(GPRS_MSG, (L"GPRS: -%S, rc=%x\r\n",__FUNCTION__, rc));
	return rc;
 }



 
//------------------------------------------------------------------------------
// Function name	: COM_Close
// Description	    : Close of all used resources
// Return type		: DWORD  
// Argument         : 
//------------------------------------------------------------------------------

BOOL COM_Close( DWORD Handle )
{
	PVCOM_OPEN_CONTEXT pOpenCtx = (PVCOM_OPEN_CONTEXT)Handle;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pOpenCtx->pHead;
	BOOL rc                 =  FALSE;

    RETAILMSG(GPRS_MSG, (L"GPRS:+%S, Handle - %x access - %x\r\n",__FUNCTION__, Handle, pOpenCtx->dwAccess));
        
    // Check if we get correct context
    if ((pInitCtx == NULL) || !pOpenCtx ||(pInitCtx->cookie != MODEM_DEVICE_COOKIE))
        {
			RETAILMSG(GPRS_MSG, (L"ERROR: COM_Close:Incorrect context parameter\r\n"));
			SetLastError(ERROR_INVALID_HANDLE);
			return FALSE;
        }

//	if(pOpenCtx && pInitCtx->isModemOn && pOpenCtx->hFTDISer && pOpenCtx->hFTDISer!=INVALID_HANDLE_VALUE)
	if(pOpenCtx && pOpenCtx->hFTDISer!=INVALID_HANDLE_VALUE)
	{
		RETAILMSG(GPRS_MSG, (L"%S: before closing   pOpenCtx->hFTDISer- %x\r\n",__FUNCTION__ ,pOpenCtx->hFTDISer));
		rc = CloseHandle(pOpenCtx->hFTDISer);
		pOpenCtx->hFTDISer=INVALID_HANDLE_VALUE;
		RETAILMSG(GPRS_MSG, (L"%S: after closing   pOpenCtx->hFTDISer- %x\r\n",__FUNCTION__,pOpenCtx->hFTDISer));
	}

	EnterCriticalSection(&(pInitCtx->OpenCS));

	for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
	{
		
		if(pInitCtx->OpenContexts[i] == (UINT32)pOpenCtx)
		{
			pOpenCtx->pHead = NULL;
			LocalFree(pOpenCtx);
			pInitCtx->OpenContexts[i] = 0;
			// If this was the handle with access permission, remove pointer
			if ( pOpenCtx == pInitCtx->pAccessOwner )
				pInitCtx->pAccessOwner = NULL;
			break;
		}
	}

	LeaveCriticalSection(&(pInitCtx->OpenCS));
	RETAILMSG(GPRS_MSG, (L"GPRS:-%S\r\n",__FUNCTION__));
	return rc;
}



//------------------------------------------------------------------------------
// Function name	: COM_Read
// Description	    : ...
// Return type		: DWORD COM_Read 
// Argument         : DWORD Handle
// Argument         : LPVOID pBuffer
// Argument         : DWORD dwNumBytes
//------------------------------------------------------------------------------
DWORD COM_Read ( DWORD Handle, LPVOID pBuffer, DWORD dwNumBytes )
{
	PVCOM_OPEN_CONTEXT pOpenCtx = (PVCOM_OPEN_CONTEXT)Handle;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pOpenCtx->pHead;
	
	DWORD rc                =  -1;

	RETAILMSG(GPRS_MSG, (L"GPRS:+%S\r\n",__FUNCTION__));
        
    // Check if we get correct context
    if ((pOpenCtx == NULL) ||(pInitCtx == NULL) || (pInitCtx->cookie != MODEM_DEVICE_COOKIE))
        {
        RETAILMSG(GPRS_MSG, (L"ERROR: COM_Read: "
            L"Incorrect context parameter\r\n"
            ));
        return -1;
        }

    if(pInitCtx->isModemOn)
	{

	WaitIfPowerSaveMode(pInitCtx);

	SetComPort(pOpenCtx);
	DWORD  dwNumBytesRead;

	BOOL   ret  = ReadFile(pOpenCtx->hFTDISer,pBuffer,dwNumBytes,&dwNumBytesRead,NULL);
	if(ret)
		rc=dwNumBytesRead;

	}
	else
	{
     SetLastError(ERROR_NOT_READY);
     rc = -1;
	}

	RETAILMSG(GPRS_MSG, (L"GPRS:-%S, rc = %d\r\n",__FUNCTION__, rc));
    return rc;


}
	                            


//------------------------------------------------------------------------------
// Function name	COM_Write {
// Description	    : ...
// Return type		: DWORD { 
// Argument         : DWORD Handle
// Argument         : LPCVOID pBuffer
// Argument         : DWORD dwNumBytes
//------------------------------------------------------------------------------
DWORD COM_Write ( DWORD Handle, LPCVOID pBuffer, DWORD dwNumBytes )
{
	PVCOM_OPEN_CONTEXT pOpenCtx = (PVCOM_OPEN_CONTEXT)Handle;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pOpenCtx->pHead;
	
	DWORD rc                =  -1;

	RETAILMSG(GPRS_MSG, (L"GPRS:+%S\r\n",__FUNCTION__));
        
    // Check if we get correct context
    if ((pOpenCtx == NULL) || (pInitCtx == NULL) || (pInitCtx->cookie != MODEM_DEVICE_COOKIE))
        {
        RETAILMSG(GPRS_MSG, (L"ERROR: COM_Write: "
            L"Incorrect context parameter\r\n"
            ));
        return -1;
        }

	if(pInitCtx->isModemOn)
	{
		WaitIfPowerSaveMode(pInitCtx);
		SetComPort(pOpenCtx);
		DWORD  dwNumBytesWritten;

		BOOL   ret  = WriteFile(pOpenCtx->hFTDISer,pBuffer,dwNumBytes,&dwNumBytesWritten,NULL);
		if(ret)
			rc=dwNumBytesWritten;
	}
	else
	{
		SetLastError(ERROR_NOT_READY);
		rc = -1;
	}

	RETAILMSG(GPRS_MSG, (L"GPRS:-%S, rc = %d\r\n",__FUNCTION__, rc));
    return rc;
}


//------------------------------------------------------------------------------
// Function name	: COM_Seek
// Description	    : ...
// Return type		: DWORD AIO_Seek 
// Argument         : DWORD Handle
// Argument         : long lDistance
// Argument         : DWORD dwMoveMethod
//------------------------------------------------------------------------------
DWORD COM_Seek ( DWORD Handle, long lDistance, DWORD dwMoveMethod )
{
	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "GPS: +GPS_Seek\r\n" ) ) );

	// ???

	DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "GPS: -GPS_Seek\r\n" ) ) );


    return (DWORD) -1;
}


//------------------------------------------------------------------------------
// Function name	: COM_PowerUp
// Description	    : ...
// Return type		: void AIO_PowerUp 
// Argument         : DWORD hDeviceContext
//------------------------------------------------------------------------------
void COM_PowerUp ( DWORD hDeviceContext )
{

//	PVCOM_OPEN_CONTEXT pOpenCtx;
//	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)hDeviceContext;

	RETAILMSG ( GPRS_MSG, ( TEXT ( "GPRS: +COM_PowerUp\r\n" ) ) );
}


//------------------------------------------------------------------------------
// Function name	: COM_PowerDown
// Description	    : ...
// Return type		: void AIO_PowerDown 
// Argument         : DWORD hDeviceContext
//------------------------------------------------------------------------------
void COM_PowerDown ( DWORD hDeviceContext )
{

	//PVCOM_OPEN_CONTEXT pOpenCtx;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)hDeviceContext;

	RETAILMSG (GPRS_MSG, ( TEXT ( "GPRS: +COM_PowerDown\r\n" ) ) );

	

	 RETAILMSG(GPRS_MSG,(TEXT("GPS: clear modem interrupt if occured \r\n")));
	 // clear status interrupt
	 //GPIOGetBit(pInitCtx->hGpio, PCA_GPIO_GPRS_0+7);
}







// Function name	: DllEntry 
// Description	    : ...
// Return type		: BOOL WINAPI 
// Argument         : HANDLE hInstDll
// Argument         : DWORD dwReason
// Argument         : LPVOID lpvReserved
BOOL WINAPI DllEntry ( HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved )
{
	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "GPRS: +DllEntry\r\n" ) ) );


    switch ( dwReason ) 
	{
    case DLL_PROCESS_ATTACH:

		DisableThreadLibraryCalls ( ( HINSTANCE )hInstDll );

		DEBUGREGISTER ( ( HINSTANCE )hInstDll );

        DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "GPRS: DLL_PROCESS_ATTACH\r\n" ) ) );

        break;

    case DLL_PROCESS_DETACH:
    
        DEBUGMSG ( ZONE_FUNCTION, ( TEXT ( "GPRS: DLL_PROCESS_DETACH\r\n" ) ) );

        break;
    }


	DEBUGMSG ( ZONE_FUNCTION, ( _T ( "GPRS: -DllEntry\r\n" ) ) );

	return TRUE;
}



//------------------------------------------------------------------------------
// Function name	: WakeupThread
// Description	    : Thread to handle wakeup event
// Return type		: DWORD  0 - failure; 1 - success 
// Argument         : 
//------------------------------------------------------------------------------

DWORD WakeupThread(VOID *pContext)
{
	DWORD rc = TRUE;
	DWORD dwReason;

	VCOM_INIT_CONTEXT *pCxt =  (VCOM_INIT_CONTEXT*)pContext;

	while(!pCxt->quit)
	{
		dwReason = WaitForSingleObject(pCxt->hWakeupEvent, INFINITE);
		if( dwReason == WAIT_OBJECT_0 )
		{
			
			RETAILMSG(GPRS_MSG, (L"MODEM. WakeupThread: WaitForSingleObject %d\r\n", dwReason));
		}
	}
	
	return rc;
}
//==============================================================================================
//
//  Function:       WaitForCOMPort
//
//  Description:    Wait till COM0 will appear 
//==============================================================================================
DWORD WaitForCOMPort(DWORD dwAttempts, PVCOM_INIT_CONTEXT pInitCtx)
{
	BOOL   quit      = FALSE;
	DWORD  nAttempts = 0;
	HANDLE hFTDISer;
	DWORD  rc = GSM_OK;
	USB_SERIAL_DEVICE_TYPE dwDeviceType;

	RETAILMSG(GPRS_MSG, (L"+WaitForCOMPort. \r\n"));

	do
	{
		hFTDISer = CreateFile(pInitCtx->szFtdiPort, DEVACCESS_BUSNAMESPACE, 0, NULL, OPEN_EXISTING, 0, NULL);
		if(hFTDISer!=INVALID_HANDLE_VALUE)
		{
			if (DeviceIoControl(hFTDISer, IOCTL_FT_GET_SET_DEVICE_TYPE, NULL, 0, &dwDeviceType, sizeof(dwDeviceType), NULL, NULL))
			{
				if (dwDeviceType == USB_SERIAL_DEVICE_TYPE_INTERNAL_MODEM)
				{
					// COM0 appeared
					quit = TRUE;
				}
			}

			CloseHandle(hFTDISer);
		}
		else
		{
			nAttempts++;
			RETAILMSG(GPRS_MSG, (L"ERROR: WaitForCOMPort: the %d attempt is failed\r\n",nAttempts ));
			Sleep(1000);
		}
	}
	while (!quit  && nAttempts < dwAttempts );

	if(quit == FALSE)
	{
		RETAILMSG(GPRS_MSG, (L"ERROR: WaitForCOMPort: " L"Modem USB Error \r\n" ));
		rc = GSM_DRIVER_ERROR;
	}	

	RETAILMSG(GPRS_MSG, (L"- WaitForCOMPort: rc = %d dwAttempts - %d \r\n",rc, nAttempts));
	return rc;
}


//==============================================================================================
//
//  Function:    GetModemPowerState
//
//  Description: Get Modem Power State 
//==============================================================================================
DWORD GetModemPowerState(HANDLE hHandle, DWORD* state)
{
	DWORD rc = 0;

	VCOM_OPEN_CONTEXT *pOpenCtx = (VCOM_OPEN_CONTEXT*)hHandle;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pOpenCtx->pHead;

	EnterCriticalSection(&(pInitCtx->OpenCS));
	*state = (pInitCtx->isModemOn)?1:0;
	LeaveCriticalSection(&(pInitCtx->OpenCS));

	return rc;
}



//==============================================================================================
//
//  Function:  SetModemPower
//
//  Set  Modem Power 
//==============================================================================================
DWORD SetModemPowerState(HANDLE hHandle, DWORD state)
{
	DWORD rc = GSM_OK;
	SYSTEM_POWER_STATUS_EX2 psex2 = {0};

	VCOM_OPEN_CONTEXT *pOpenCtx = (VCOM_OPEN_CONTEXT*)hHandle;
	PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pOpenCtx->pHead;

	 // Check if we get correct context
    if ( (pOpenCtx == NULL) || (pInitCtx == NULL) || (pInitCtx->cookie != MODEM_DEVICE_COOKIE))
        {
			RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"Incorrect context parameter\r\n" ));
			SetLastError(ERROR_INVALID_PARAMETER);
			return GSM_ERROR_INVALID_PARAMETER;
        }

	if (pInitCtx->enableExternalModem)
		return GSM_ERROR_ACCESS_DENIED;

	// turn the modem on
	if(state)
	{
		// TBD: put critical section
		if(!pInitCtx->isModemOn)
		{
			// check if battery capacity is sufficient to turn on the modem HW
			GetSystemPowerStatusEx2(&psex2, sizeof(psex2), TRUE);
			if(psex2.BatteryLifePercent < MIN_MODEM_BATTERY_LEVEL)
			{
				RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"The battery level is not sufficient to turn on the modem\r\n" ));
				return GSM_ERROR_LOW_BATTERY;
			}
			else
			{
				// turn on the modem
				if(ModemON(pInitCtx)==FALSE)
				{
					RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"Modem HW Error \r\n" ));
					SetLastError(ERROR_NOT_READY);
					return GSM_HARDWARE_ERROR;
				}
				// wait till COM0 will appear

				rc = WaitForCOMPort(5, pInitCtx);
				// if COM0 didn't appear then it's an error
				if(rc != GSM_OK)
				{
					RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"Modem HW Error \r\n" ));
					SetLastError(ERROR_NOT_READY);
					return rc;
				}

				// now when the modem is already is turned on and FTDI USB virtual port appeared
				// we can match between our COM ports and FTDI COM ports

				if (SetupPDD(pInitCtx,TRUE))
				{
					RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"Modem USB Error \r\n" ));
					SetLastError(ERROR_NOT_READY);
					return GSM_DRIVER_ERROR;
				}
				pInitCtx->isModemOn = TRUE;

				// ##### Michael. Notify that Modem is On
				if( pInitCtx->hModemOnOffEvent != NULL )
				{
				 SetEvent(pInitCtx->hModemOnOffEvent);
				 RETAILMSG(GPRS_MSG, (L"GPRS:%S, Modem On notifying\r\n",__FUNCTION__));
				}
			}
		}
		// modem is already turned on
		else
		{
			RETAILMSG(GPRS_MSG, (L"SetModemPowerState: " L"Modem is already is turned on \r\n" ));
			return GSM_ERROR_MODEM_ALREADY_ON;
		}
					
	}
	// to turn off the modem
	else
	{
		if(pInitCtx->isModemOn)
		{
			pInitCtx->isModemOn = FALSE;

			// ##### Michael. Notify that Modem is Off
			if( pInitCtx->hModemOnOffEvent != NULL )
			{
             RETAILMSG(GPRS_MSG, (L"GPRS:%S, Modem Off notifying\r\n",__FUNCTION__));
			 ResetEvent(pInitCtx->hModemOnOffEvent);
			}

			// close FTDI drivers
			if (SetupPDD(pInitCtx,FALSE))
			{
				RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"Modem USB Error \r\n" ));
				SetLastError(ERROR_NOT_READY);
				return GSM_DRIVER_ERROR;
			}
			
			// turn off the modem
			if(ModemOFF(pInitCtx)==FALSE)
			{
				RETAILMSG(GPRS_MSG, (L"ERROR: SetModemPowerState: " L"Modem HW Error \r\n" ));
				SetLastError(ERROR_NOT_READY);
				return GSM_HARDWARE_ERROR;
			}


			//pInitCtx->isModemOn = FALSE;
		}
		else
		{
			RETAILMSG(GPRS_MSG, (L"SetModemPowerState: " L"Modem is already is turned off \r\n" ));
			return GSM_ERROR_MODEM_ALREADY_OFF;
		}
		
	}

	RETAILMSG(GPRS_MSG, (L"SetModemPowerState: " L"ended state - %x,rc - %x\r\n",state,rc ));
	return rc;


}


//==============================================================================================
//
//  Function:  SetupPDD
//
//  Set  Modem Power 
//==============================================================================================
DWORD SetupPDD(HANDLE hHandle, DWORD state)
{
	DWORD rc = 0;

	VCOM_INIT_CONTEXT *pInitCtx = (VCOM_INIT_CONTEXT*)hHandle;
	PVCOM_OPEN_CONTEXT pOpenCtx;

	// 

	if(state)
	{
		for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
		{

			pOpenCtx = (PVCOM_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
			if(pOpenCtx)
			{
				RETAILMSG(GPRS_MSG, (L"SetupPDD:pOpenCtx = %x, pOpenCtx->hFTDISer-%x dwAccess-%x , dwShareMode - %x. state - %x\r\n",
					pOpenCtx, pOpenCtx->hFTDISer,pOpenCtx->dwAccess,pOpenCtx->dwShareMode,state));
			}

			if(pOpenCtx && /*!(pOpenCtx->dwAccess & DEVACCESS_BUSNAMESPACE )&&*/(pOpenCtx->hFTDISer == INVALID_HANDLE_VALUE))
			{
				pOpenCtx->hFTDISer = CreateFile(pInitCtx->szFtdiPort, pOpenCtx->dwAccess, pOpenCtx->dwShareMode, NULL,
										OPEN_EXISTING,0,NULL);

				RETAILMSG(GPRS_MSG, (L"SetupPDD: pOpenCtx = %x, pOpenCtx->hFTDISer-%x dwAccess-%x , dwShareMode - %x.\r\n",
					pOpenCtx, pOpenCtx->hFTDISer,pOpenCtx->dwAccess,pOpenCtx->dwShareMode));

				if(pOpenCtx->hFTDISer==INVALID_HANDLE_VALUE)
				{
					RETAILMSG(GPRS_MSG, (L"ERROR: SetupPDD: FTDI port is not ready .\r\n" ));
					rc = GSM_DRIVER_ERROR;
					break;
					
				}
			}
		} 
	}
	else
	{
		for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
		{
			pOpenCtx = (PVCOM_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
			if(pOpenCtx && pOpenCtx->hFTDISer != INVALID_HANDLE_VALUE)
			{
				RETAILMSG(GPRS_MSG, (L"SetupPDD: pOpenCtx = %x, pOpenCtx->hFTDISer-%x dwAccess-%x , dwShareMode - %x,state - %x\r\n",
					pOpenCtx, pOpenCtx->hFTDISer,pOpenCtx->dwAccess,pOpenCtx->dwShareMode,state));
				CloseHandle(pOpenCtx->hFTDISer);
				pOpenCtx->hFTDISer=INVALID_HANDLE_VALUE;
			}
						
		} 
	}

	return rc;
}
//==============================================================================================
//  ##### Michael
//  Function:  ModemONProc
//
//  Turn on the modem
//==============================================================================================
BOOL ModemON(PVCOM_INIT_CONTEXT pInitCtx)
{
  BOOL modemON = FALSE;
/*
    if(!GPIOGetBit(pInitCtx->hGpio, RESET_GE864_OUT))
	{
		if(GPIOGetBit(pInitCtx->hGpio, PWRMON_IN))
		{
			RETAILMSG(GPRS_MSG, (L"GPRS power monitor bit already set.\r\n"));
			return TRUE;
		}
	}

	if( pInitCtx->dwModemVersion > 1 )
	{
     RETAILMSG(GPRS_MSG, (L"%S: Start NBOARD816V3 board Power On sequence\r\n",__FUNCTION__));
     GPIOSetBit(pInitCtx->hGpio, GPRS_LOW_CHARGE);
	 Sleep(20);
     GPIOSetBit(pInitCtx->hGpio, GPRS_V4V0V_EN);
	 GPIOClrBit(pInitCtx->hGpio, GPRS_LOW_CHARGE);
	}

	// Anyway, ensure that Modem digital pins are disconnected from while
	// Modem is Off or during power on process

	// Set USB2UART FT232RQ in Reset state.
	GPIOSetBit(pInitCtx->hGpio, nRESET_FT232RQ_OUT);

#ifdef BSP_VOICE_PATH
	// Disable FXL4TD245 Signal Translator
	GPIOSetBit(pInitCtx->hGpio, PCM_ENABLE_OUT);
#endif

    // Turn on Modem power
	if(pInitCtx->dwModemVersion>0)
	  GPIOSetBit(pInitCtx->hGpio, GPRS_POWER_EN_OUT);
	else
	  GPIOClrBit(pInitCtx->hGpio, GPRS_POWER_EN_OUT);

	Sleep(400);
    // release Reset
	GPIOClrBit(pInitCtx->hGpio, RESET_GE864_OUT);

    //"The RESET# is internally controlled on start-up to achieve always a proper power-on
    //reset sequence, so there's no need to control this pin on start-up. It may only be used
    //to reset a device already on that is not responding to any command."

	// so, let modem 200 ms delay as needed for 'RESET procedure'
	Sleep(505);

	// Check PWRMON bit, if it is HIGH, Modem is already turned On,
	// If low - process 'Modem On procedure'.
	if(GPIOGetBit(pInitCtx->hGpio, PWRMON_IN))
	{
		RETAILMSG(GPRS_MSG, (L"GPRS Modem is ON just after Power Enabling!\r\n"));
		Sleep(1005);
		//return TRUE;
	}
	else
	{
	 // Process 'Modem On procedure'
	  modemON = ModemONProc(pInitCtx);
      if( modemON == FALSE ) // Process 'RESET procedure'
	  {
		  RETAILMSG(GPRS_MSG, (L"GPRS Modem is yet not ON, call Reset procedure!\r\n"));
		  modemON = ModemResetProc(pInitCtx);
		  if( modemON == FALSE )
           modemON = ModemONProc(pInitCtx);
	  }
	  else
	  {
        RETAILMSG(GPRS_MSG, (L"GPRS Modem is ON after Modem On procedure\r\n"));
	  }

	  if( modemON == FALSE )
	  {
       RETAILMSG(GPRS_MSG, (L"GPRS Modem ON Failure!\r\n"));  
	   // Turn Off Modem Power
  	   if(pInitCtx->dwModemVersion>0)
	    GPIOClrBit(pInitCtx->hGpio, GPRS_POWER_EN_OUT);
	   else
	    GPIOSetBit(pInitCtx->hGpio, GPRS_POWER_EN_OUT);

	   return(FALSE);
	  }
	}

	// Now, after Modem is succesfully ON, enable its digital pins
	// Clear USB2UART FT232RQ in Reset state.
	GPIOClrBit(pInitCtx->hGpio, nRESET_FT232RQ_OUT);

	// Enable FXL4TD245 Signal Translator
	// No, let Audio Codec PCM Voice deal with it!
//#ifdef BSP_VOICE_PATH
	//GPIOClrBit(pInitCtx->hGpio, PCM_ENABLE_OUT);
//#endif

	Sleep(10);
*/
	return(TRUE);

}
BOOL ModemOFF(PVCOM_INIT_CONTEXT pInitCtx)
{
  BOOL modemOff = FALSE;
/*
	// if PWRMON == 0, means modem is already turned off
	if(!GPIOGetBit(pInitCtx->hGpio, PWRMON_IN))
	{
		RETAILMSG(GPRS_MSG, (L"GPRS power monitor bit already turned off\r\n"));
		modemOff = TRUE;
	}
	else
	{

		// Set USB2UART FT232RQ in Reset state.
		GPIOSetBit(pInitCtx->hGpio, nRESET_FT232RQ_OUT);

		// Disable FXL4TD245 Signal Translator
		// Audio Codec PCM Voice deal with it,
		// but anyway ensure that it is disabled
#ifdef BSP_VOICE_PATH
		GPIOSetBit(pInitCtx->hGpio, PCM_ENABLE_OUT);
#endif

		// Call Modem Off procedure
		modemOff = ModemOFFProc(pInitCtx);
		if( modemOff == FALSE)
		{
          RETAILMSG(GPRS_MSG, (L"GPRS Modem Off procedure Failure, call Reset procedure\r\n"));
		  if( ModemResetProc(pInitCtx) ) // Return status of PWRMON bit
		  {
			modemOff = ModemOFFProc(pInitCtx); 
		  }
		}

		if( modemOff == FALSE )
		{
         RETAILMSG(GPRS_MSG, (L"GPRS Modem Off procedure total Failure\r\n"));
		}
		else
		{ 
         RETAILMSG(GPRS_MSG, (L"GPRS Modem Off procedure finished\r\n"));
		}
	}

	//Anyway switch Modem power off
    if(pInitCtx->dwModemVersion>0)
	 GPIOClrBit(pInitCtx->hGpio, GPRS_POWER_EN_OUT);
    else
  	 GPIOSetBit(pInitCtx->hGpio, GPRS_POWER_EN_OUT);

	Sleep(500);

	// set Reset
	GPIOSetBit(pInitCtx->hGpio, RESET_GE864_OUT);

	if( pInitCtx->dwModemVersion > 1 )
	{
     RETAILMSG(GPRS_MSG, (L"%S: End NBOARD816V3 board Power Off sequence\r\n",__FUNCTION__));
     GPIOClrBit(pInitCtx->hGpio, GPRS_V4V0V_EN);
	 GPIOClrBit(pInitCtx->hGpio, GPRS_LOW_CHARGE);
	}
*/
   return(modemOff);
}


 //==============================================================================================
//
//  Function:     SetModemInitConfig
//  Description:  Set initial configuration for modem module
//
//==============================================================================================

 DWORD SetModemInitConfig(PVCOM_INIT_CONTEXT pCxt)
 {
	 DWORD ret = 1;

	 RETAILMSG(GPRS_MSG, (L"SetGPRSInitConfig: started. \r\n"));
/*

	 if (pCxt->hGpio == NULL)
	 {
         RETAILMSG(GPRS_MSG, (L"ERROR: SetInitConfig: " L"Failed to open Gpio driver \r\n"));
         return 0;
	 }


      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_0, GPIO_DIR_INPUT );
      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_1, GPIO_DIR_INPUT );
      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_2, GPIO_DIR_INPUT );
      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_3, GPIO_DIR_INPUT );

      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_4, GPIO_DIR_INPUT );
      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_5, GPIO_DIR_INPUT );
      GPIOSetMode(pCxt->hGpio, MODEM_ID_IN_6, GPIO_DIR_INPUT );
	  if(pCxt->dwModemVersion>0)
        GPIOSetMode(pCxt->hGpio, UART_RING_2V8_IN, GPIO_DIR_INPUT ); // Interrupt? Which edge ?
	  else
        GPIOSetMode(pCxt->hGpio, UART_RING_2V8_IN, GPIO_DIR_OUTPUT ); 

      GPIOSetMode(pCxt->hGpio, GPRS_POWER_EN_OUT, GPIO_DIR_OUTPUT );
      GPIOSetMode(pCxt->hGpio, TEST_OUT, GPIO_DIR_OUTPUT );
      GPIOSetMode(pCxt->hGpio, nRESET_FT232RQ_OUT, GPIO_DIR_OUTPUT );
      GPIOSetMode(pCxt->hGpio, ON_OFF_PULSE_OUT, GPIO_DIR_OUTPUT );

      GPIOSetMode(pCxt->hGpio, RESET_GE864_OUT, GPIO_DIR_OUTPUT );
#ifdef BSP_VOICE_PATH
      GPIOSetMode(pCxt->hGpio, PCM_ENABLE_OUT, GPIO_DIR_OUTPUT );
      GPIOSetMode(pCxt->hGpio, PCM_MASTER_SLAVE_OUT, GPIO_DIR_OUTPUT );
#endif
      GPIOSetMode(pCxt->hGpio, PWRMON_IN, GPIO_DIR_INPUT );

	  if( pCxt->dwModemVersion > 1 )
	  {
       GPIOSetMode(pCxt->hGpio, GPRS_LOW_CHARGE, GPIO_DIR_OUTPUT );
	   GPIOSetMode(pCxt->hGpio, GPRS_V4V0V_EN, GPIO_DIR_OUTPUT );
	   RETAILMSG(GPRS_MSG, (L"%S: NBOARD816V3 board\r\n",__FUNCTION__));
	  }
*/
	 return ret;
 }


 BOOL  ModemResetProc(PVCOM_INIT_CONTEXT pCxt)
 {
   BOOL powerMonRet;
/*
     // Set reset pulse
 	 GPIOSetBit(pCxt->hGpio, RESET_GE864_OUT);
	 Sleep(205);
	 GPIOClrBit(pCxt->hGpio, RESET_GE864_OUT);

     powerMonRet = GPIOGetBit(pCxt->hGpio, PWRMON_IN);
	 Sleep(1005);
*/
     return(powerMonRet);
 }

BOOL  ModemONProc(PVCOM_INIT_CONTEXT pInitCtx)
{
  BOOL modemON = FALSE;
/*
	 // Process 'Modem On procedure'
	  GPIOSetBit(pInitCtx->hGpio, ON_OFF_PULSE_OUT);
	  // At least 1 sec
	  Sleep(1100);
	  GPIOClrBit(pInitCtx->hGpio, ON_OFF_PULSE_OUT);
       
	   if( GPIOGetBit(pInitCtx->hGpio, PWRMON_IN) )
	   {
        modemON = TRUE;
	   }
	   else // 2 sec timeout
	   {
         Sleep(2100);
		 if( GPIOGetBit(pInitCtx->hGpio, PWRMON_IN) )
          modemON = TRUE;
	   }
*/
   return(modemON);
}


BOOL  ModemOFFProc(PVCOM_INIT_CONTEXT pInitCtx)
{
  BOOL modemOff = FALSE;
/*
  // If Modem is already Off
  if(GPIOGetBit(pInitCtx->hGpio, PWRMON_IN) == 0)
   return(TRUE); 

  // Process 'Modem On procedure'
   GPIOSetBit(pInitCtx->hGpio, ON_OFF_PULSE_OUT);
   // At least 2 sec!
   Sleep(2100);
   GPIOClrBit(pInitCtx->hGpio, ON_OFF_PULSE_OUT);

	// wait for max 15 sec to get modem turned off
	for(int i = 0;i<15;i++)
	{
		Sleep(1010);
		if(GPIOGetBit(pInitCtx->hGpio, PWRMON_IN)==0)
		{
			modemOff = TRUE;
			break;
		}
	}
*/
	if( modemOff == TRUE )
	{
      Sleep(1600);
	  return(TRUE);
	}
	else // will need 'Reset procedure'
	{
     return(FALSE);
	}

}

//==============================================================================================
//
//  Function:  WaitIfPowerSaveMode
//
//  Sets the device power state
//==============================================================================================

DWORD WaitIfPowerSaveMode(PVCOM_INIT_CONTEXT pInitCtx)
{
	DWORD rc = 1;

	BOOL   quit = FALSE;
	DWORD  nAttempts =0;

	if(!pInitCtx->dwModemPowerSaveMode)
		return rc;

	while(!quit && nAttempts<MAX_POWER_SAVE_ATTEMPTS)
	{
		nAttempts++;
		if(pInitCtx->dwModemPowerSaveMode)
			Sleep(1000);
		else
			quit=TRUE;
	};

	 RETAILMSG(GPRS_MSG, (L"+WaitIfPowerSaveMode nAttempts = %d\r\n",nAttempts));

	 return rc;
}


//==============================================================================================
//
//  Function:       SetModemWakeup
//
//  Description:    Enable/Disable Modem Wakeup pin according to the argument
//==============================================================================================
BOOL SetModemWakeup(PVCOM_INIT_CONTEXT pInitCtx, BOOL  flag)
{
	BOOL rc = TRUE;
/*
	if(!pInitCtx || !pInitCtx->hGpio)
	{
		RETAILMSG(GPRS_MSG, (L"ERROR: SetModemWakeup invalid arguments\r\n"));
		return FALSE;

	}

	if(flag)
	{
		if(!pInitCtx->bModemWakeupOn && pInitCtx->isModemOn)
		{

		// // enable modem wakeup source

			if (!GPIOInterruptInitialize(pInitCtx->hGpio, PCA_GPIO_GPRS_0+7, pInitCtx->hWakeupEvent))
			{
				RETAILMSG(GPRS_MSG,(TEXT("SetModemWakeup: GPIOInterruptInitialize: failed \r\n")));
				return FALSE;
			}

			
			DWORD buffer[3] = {PCA_GPIO_GPRS_0+7,(PCA_GPIO_GPRS_0+7 - PCA_GPIO_0),1};
			 if (!GPIOIoControl(pInitCtx->hGpio,IOCTL_GPIO_SET_WAKEUP,(BYTE*)buffer,sizeof(buffer), 0, 0, NULL, NULL))
			 {
				RETAILMSG(GPRS_MSG,(TEXT("SetModemWakeup: IOCTL_GPIO_SET_WAKEUP: failed \r\n")));
				return FALSE;
			 }
			 pInitCtx->bModemWakeupOn = TRUE;
		}
	}
	else
	{
		if(pInitCtx->bModemWakeupOn)
		{
			if(!GPIOGetBit(pInitCtx->hGpio, PCA_GPIO_GPRS_0+7))
			{
				RETAILMSG(GPRS_MSG,(TEXT("GPS:  modem interrupt is a wakeup event. \r\n")));
			}

			 // disable modem wakeup source


			 DWORD buffer[3] = {PCA_GPIO_GPRS_0+7,(PCA_GPIO_GPRS_0+7 - PCA_GPIO_0),0};
			 if (!GPIOIoControl(pInitCtx->hGpio,IOCTL_GPIO_SET_WAKEUP,(BYTE*)buffer,sizeof(buffer), 0, 0, NULL, NULL))
			 {
				RETAILMSG(GPRS_MSG,(TEXT("GPS: IOCTL_GPIO_SET_WAKEUP: failed \r\n")));
				return FALSE;
			 }

			 if (!GPIOInterruptDisable(pInitCtx->hGpio, PCA_GPIO_GPRS_0+7))
			 {
				RETAILMSG(GPRS_MSG,(TEXT("GPS: GPIOInterruptInitialize: failed \r\n")));
				return FALSE;
			 }
			 pInitCtx->bModemWakeupOn = FALSE;
		}
		
	}
*/
	return rc;

}

DWORD MuxDeactivatingThread(VOID *param)
{
	HANDLE hMuxHandles[2] = {0};
	HANDLE hCtrlCom = INVALID_HANDLE_VALUE;
	HKEY hKey = NULL;
	DWORD ret = GSM710ERROR_SUCCESS;
	DEVMGR_DEVICE_INFORMATION di = {0};

	LONG lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_MUX07_10_REG_KEY1, 0, 0, &hKey) ;
    if (ERROR_SUCCESS != lStatus)
    {
		ret = GSM710ERROR_INVALID_REGISTRY_KEY;
        goto error;
    }

	DWORD dwPortIndex;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwValType = REG_DWORD;
    lStatus = RegQueryValueEx(hKey, TEXT("Index"), NULL, &dwValType, (LPBYTE)&dwPortIndex, &dwSize);
    if (ERROR_SUCCESS != lStatus)
    {
		ret = GSM710ERROR_INVALID_REGISTRY_VALUE;
        goto error;
    }

	RegCloseKey(hKey);
	hKey = NULL;

	TCHAR szCtrlPort[16];
	wsprintf(szCtrlPort, TEXT("COM%d:"), dwPortIndex);

	hCtrlCom = CreateFile(szCtrlPort, DEVACCESS_BUSNAMESPACE, 0, 0, 0, 0, 0);
	if (hCtrlCom == INVALID_HANDLE_VALUE)
	{
		ret = GSM710ERROR_ALREADY_DEACTIVATED;
		goto error;
	}

	if (!DeviceIoControl(hCtrlCom, IOCTL_MUX07_10_GET_SET_HANDLE, 0, 0, hMuxHandles, sizeof(HANDLE) * 2, 0, 0))
	{
		ret = GSM710ERROR_DRIVER_NOT_RESPONDING;
		goto error;
	}
		
	if ((HANDLE)0 == hMuxHandles[1] || (HANDLE)0 == hMuxHandles[0])
	{
		di.dwSize = sizeof(di);
		
		ret = GetDeviceInformationByFileHandle(hCtrlCom, &di);
		if (!ret)
		{
			ret = GSM710ERROR_SYSTEM_FAILED;
			goto error;
		}
		RETAILMSG(GPRS_MSG, (L"GetDeviceInformationByFileHandle: ret - %x, HANDLE - %x, err - %d\r\n", ret, di.hDevice, GetLastError()));
		
		hMuxHandles[0] = di.hDevice;
		CloseHandle(hCtrlCom);

		lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, DRIVER_MUX07_10_REG_KEY2, 0, 0, &hKey) ;
		if (ERROR_SUCCESS != lStatus)
		{
			ret = GSM710ERROR_INVALID_REGISTRY_KEY;
			goto error;
		}
		lStatus = RegQueryValueEx(hKey, TEXT("Index"), NULL, &dwValType, (LPBYTE)&dwPortIndex, &dwSize);
		if (ERROR_SUCCESS != lStatus)
		{
			ret = GSM710ERROR_INVALID_REGISTRY_VALUE;
			goto error;
		}

		wsprintf(szCtrlPort, TEXT("COM%d:"), dwPortIndex);

		RegCloseKey(hKey);
		hKey = NULL;

		hCtrlCom = CreateFile(szCtrlPort, DEVACCESS_BUSNAMESPACE, 0, 0, 0, 0, 0);
		if (INVALID_HANDLE_VALUE == hCtrlCom)
		{
			RETAILMSG(GPRS_MSG,(L"CreateFile failed  error - %d\r\n", GetLastError()));
			ret = GSM710ERROR_SYSTEM_FAILED;
			goto error;
		}
		memset(&di, 0, sizeof(di));
		di.dwSize = sizeof(di);

		ret = GetDeviceInformationByFileHandle(hCtrlCom, &di);
		if (!ret)
		{
			ret = GSM710ERROR_SYSTEM_FAILED;
			goto error;
		}
		RETAILMSG(GPRS_MSG, (L"GetDeviceInformationByFileHandle: ret - %x, HANDLE - %x, err - %d\r\n", ret, di.hDevice, GetLastError()));
		hMuxHandles[1] = di.hDevice;
		ret = GSM710ERROR_SUCCESS;
	}

	CloseHandle(hCtrlCom);
	hCtrlCom = INVALID_HANDLE_VALUE;

	if (!DeactivateDevice(hMuxHandles[1]))
	{
		RETAILMSG(GPRS_MSG, (L"GSM710 API:-%S, DeactivateDevice(MUX1) error = %d\r\n",__FUNCTION__, GetLastError()));
	}

	if (!DeactivateDevice(hMuxHandles[0]))
	{
		RETAILMSG(GPRS_MSG, (L"GSM710 API:-%S, DeactivateDevice(MUX0) error = %d\r\n",__FUNCTION__, GetLastError()));
	}

error:;
	if (hCtrlCom != INVALID_HANDLE_VALUE)
		CloseHandle(hCtrlCom);

	if (hKey)
        RegCloseKey(hKey) ;

	return ret;
}

//==============================================================================================
//
//  Function SetPowerState
//
//  Sets the device power state
//==============================================================================================
BOOL SetPowerState(PVCOM_INIT_CONTEXT pInitCtx,CEDEVICE_POWER_STATE    power)
{
    BOOL rc = FALSE;
	DWORD ret = 0;
	PVCOM_OPEN_CONTEXT pOpenCtx;


    
    RETAILMSG(GPRS_MSG, (L"+SetPowerState(0x%08X, 0x%08x)\r\n", pInitCtx, power));
	DWORD dwReason =0;

    switch (power)
        {
        case D0:
			pInitCtx->dwModemPowerSaveMode=0;

			// turn off the modem power save mode
			if(pInitCtx->dwModemVersion!=-1)
			{
				rc = SetModemWakeup(pInitCtx, FALSE);
			}

			//pInitCtx->dwModemPowerSaveMode=0;

			break;
        case D1:
			rc = TRUE;
			break;
        case D2: 
			//pInitCtx->dwModemPowerSaveMode=1;

			rc = TRUE;
			break;
        case D3:

			if(pInitCtx->dwModemVersion!=-1 && pInitCtx->isModemOn)
			{
				if (pInitCtx->isControlledByMux)
				{
					pInitCtx->hMuxDeactivatingThread = CreateThread(NULL,0, MuxDeactivatingThread, 0x0, 0,NULL);
					RETAILMSG(1,(L"GPRS: wait for mux deactivation %d\r\n", GetTickCount() ));
					ret = WaitForSingleObject(pInitCtx->hMuxDeactivatingThread, 2000);
					RETAILMSG(1,(L"GPRS: wait for mux deactivated %d %d\r\n", ret, GetTickCount()));
					CloseHandle(pInitCtx->hMuxDeactivatingThread);
					pInitCtx->hMuxDeactivatingThread = 0;
				}
				pInitCtx->dwModemPowerSaveMode=1;

				for(int i = 0; i < (sizeof(pInitCtx->OpenContexts)/sizeof(pInitCtx->OpenContexts[0])); i++)
				{
					pOpenCtx = (PVCOM_OPEN_CONTEXT )pInitCtx->OpenContexts[i];
					if(pOpenCtx)
					{
						if((pOpenCtx->dwAccess & (GENERIC_READ | GENERIC_WRITE)))
						{
							DCB dcb;
							pOpenCtx->lastCommDCB.DCBlength = sizeof(pOpenCtx->lastCommDCB);
							if(GetCommState(pOpenCtx->hFTDISer,&pOpenCtx->lastCommDCB))
							{	
								RETAILMSG(GPRS_MSG, (L"D3/D4: the last DCB is saved\r\n"));
				
							}
							memcpy(&dcb, &pOpenCtx->lastCommDCB, sizeof(dcb));
							dcb.fDtrControl = DTR_CONTROL_ENABLE;
							dcb.fRtsControl = RTS_CONTROL_ENABLE;
							if (!SetCommState(pOpenCtx->hFTDISer, &dcb))
							{
								RETAILMSG(GPRS_MSG, (L"D3/D4: Failed SetCommState.\r\n"));
							}
							if (!EscapeCommFunction(pOpenCtx->hFTDISer, CLRDTR))
							{
								RETAILMSG(GPRS_MSG, (L"D3/D4: Failed EscapeCommFunction.\r\n"));
							}

						}
						CloseHandle(pOpenCtx->hFTDISer);
						pOpenCtx->hFTDISer=INVALID_HANDLE_VALUE;
						
					}
				}
				//pInitCtx->dwModemPowerSaveMode=1;

				rc = SetModemWakeup(pInitCtx, TRUE);



			}
			else
			{
				RETAILMSG(GPRS_MSG, (L"+SetPowerState: D3: do nothing.\r\n"));
				rc = TRUE;
			}
            break;

	case D4:
		RETAILMSG(GPRS_MSG, (L"SetPowerState:Invalid power state (%d)\r\n", power )); 
		if (pInitCtx->isControlledByMux)
		{
			pInitCtx->hMuxDeactivatingThread = CreateThread(NULL,0, MuxDeactivatingThread, 0x0, 0,NULL);
			RETAILMSG(1,(L"GPRS: wait for mux deactivation %d\r\n", GetTickCount() ));
			ret = WaitForSingleObject(pInitCtx->hMuxDeactivatingThread, 2000);
			RETAILMSG(1,(L"GPRS: wait for mux deactivated %d %d\r\n", ret, GetTickCount()));
			CloseHandle(pInitCtx->hMuxDeactivatingThread);
			pInitCtx->hMuxDeactivatingThread = 0;
		}
		if(pInitCtx->dwModemVersion!=-1 && pInitCtx->isModemOn)
		{
			RETAILMSG(GPRS_MSG, (L"Turning modem off)\r\n")); 
			rc = ModemOFF(pInitCtx);
		}
		else
		{
			RETAILMSG(GPRS_MSG, (L"+SetPowerState: D4: do nothing.\r\n"));
			rc = TRUE;
		}
		break;


        default:
            RETAILMSG(GPRS_MSG, (L"WARN: GPS::SetPowerState:Invalid power state (%d)\r\n", power ));            
            goto cleanUp;
        }

    pInitCtx->powerState = power;

   // rc = TRUE;
    
cleanUp:
    RETAILMSG(GPRS_MSG, (L"-SetPowerState(0x%08X, 0x%08x)\r\n", pInitCtx, power));
        
    return rc;
}
//==============================================================================================
//
//  Function:     SetComPort
//  Description:  Set valid serial port handle 
//
//==============================================================================================

 BOOL SetComPort(PVCOM_OPEN_CONTEXT pCxt)
 {
	// DWORD NumBytesWritten,NumOfReadBytes;
	// char	buffer [100];
	 DWORD rc;


	 PVCOM_INIT_CONTEXT pInitCtx = (PVCOM_INIT_CONTEXT)pCxt->pHead;

	// RETAILMSG(GPRS_MSG, (L"SetComPort: started1. Mode = %d\r\n",pInitCtx->dwModemPowerSaveMode));

	 EnterCriticalSection(&(pInitCtx->OpenCS));

	  if (pInitCtx->dwModemPowerSaveMode==FALSE && pCxt->hFTDISer==INVALID_HANDLE_VALUE) 
	  {
		  // workaround
		 Sleep(4000);


		 // wait for COM0 port to appear
		  rc = WaitForCOMPort(5, pInitCtx);
				// if COM0 didn't appear then it's an error
		  if(rc){
			RETAILMSG(GPRS_MSG, (L"ERROR: SetComPort: " L"Modem HW Error \r\n" ));
			rc = FALSE;
			goto CleanUp;
		  }

		  RETAILMSG(GPRS_MSG, (L"SetComPort: Create FTDI port  dwAccess= %x, dwShareMode = %x\r\n",pCxt->dwAccess,pCxt->dwShareMode ));

		  pCxt->hFTDISer = CreateFile(pInitCtx->szFtdiPort, pCxt->dwAccess, pCxt->dwShareMode, NULL,
								OPEN_EXISTING,0,NULL);

		if(pCxt->hFTDISer==INVALID_HANDLE_VALUE)
		{
			RETAILMSG(GPRS_MSG, (L"SetComPort:Error: failed to create COM port error - %x\r\n",GetLastError()));
			rc = FALSE;
			goto CleanUp;
		}

		// set DTR to make sure it's not in power save mode

		if((pCxt->dwAccess & (GENERIC_READ | GENERIC_WRITE)))
		{
			if(!DeviceIoControl(pCxt->hFTDISer,IOCTL_SERIAL_SET_DTR,NULL,0,NULL,0,NULL,NULL))
			{
				RETAILMSG(GPRS_MSG, (L"D0:DeviceIoControl is failed, error - %x\r\n",GetLastError()));
				rc = FALSE;
				goto CleanUp;
			}
		}

		
		  // todo: set the already existed DCD

		if(pCxt->lastCommDCB.BaudRate)
		{
			RETAILMSG(GPRS_MSG, (L"SetComPort:Set BaudRate = %x.\r\n",pCxt->lastCommDCB.BaudRate));
			if(!SetCommState(pCxt->hFTDISer,&pCxt->lastCommDCB))
			{
				CloseHandle(pCxt->hFTDISer);
				pCxt->hFTDISer=INVALID_HANDLE_VALUE;
				RETAILMSG(GPRS_MSG, (L"SetComPort:Failed SetCommState.\r\n"));
				rc = FALSE;
				goto CleanUp;
			}
			memset (&pCxt->lastCommDCB,0,sizeof(DCB));
		}

		if (!SetCommTimeouts(pCxt->hFTDISer, &pCxt->lastCommTimeouts))
		{
			CloseHandle(pCxt->hFTDISer);
			pCxt->hFTDISer = INVALID_HANDLE_VALUE;
			RETAILMSG(GPRS_MSG, (L"SetComPort:Failed SetCommTimeouts.\r\n"));
			rc = FALSE;
			goto CleanUp;
		}

		if (!SetCommMask(pCxt->hFTDISer, pCxt->lastCommMask))
		{
			CloseHandle(pCxt->hFTDISer);
			pCxt->hFTDISer = INVALID_HANDLE_VALUE;
			RETAILMSG(GPRS_MSG, (L"SetComPort:Failed SetCommMask.\r\n"));
			rc = FALSE;
			goto CleanUp;
		}
	  }
	  else
	  {
		 // RETAILMSG(GPRS_MSG, (L"SetComPort:do nothing. \r\n"));
	  }
	 // return pCxt->hFTDISer;

	  rc = TRUE;

CleanUp:
	  LeaveCriticalSection(&(pInitCtx->OpenCS));
	  //RETAILMSG(GPRS_MSG, (L"SetComPort: ended. Mode = %d, rc = %x \r\n",pInitCtx->dwModemPowerSaveMode,rc));

	  return rc;


 }




BOOL SendCommand(HANDLE hDev, CHAR const *pszCommand)
{
    DWORD   dwLen;
    BOOL bRet = FALSE;
    
    //
    // This routine sends a Modem command, and then waits for
    // a valid response from the modem (or timeout).
    //

   // try
  //  {
        if(  (HANDLE)INVALID_HANDLE_VALUE != hDev )
        {    
            RETAILMSG(GPRS_MSG, (TEXT("SendCommand '%a'\r\n"), pszCommand));
            

            // Purge any old responses sitting in serial buffer
            if (PurgeComm( hDev, PURGE_RXCLEAR|PURGE_TXCLEAR )) {
				if(!pszCommand)
					return TRUE;

                // Now, lets send this new string to the modem
                bRet = WriteFile(hDev, (LPVOID)pszCommand, strlen(pszCommand), &dwLen, 0 );
                if ((FALSE == bRet) || (dwLen != strlen(pszCommand))) {
                    bRet = FALSE;
                    RETAILMSG(GPRS_MSG,
                               (TEXT("UNIMODEM:!!!MdmSendCommand wrote %d of %d byte dial string\r\n"),
                                dwLen, strlen(pszCommand)) );
  
                }
            }
        }
  //  }
   // except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
   //         EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        // Just return of handle is bad
        bRet = FALSE;
 //   }


    return bRet;
}



DWORD CmdGetResponse(HANDLE    hDevice)
{
	BOOL    bStatus;
	UCHAR   InBuf[100];
	DWORD   dwLen,ModemStat;
	DWORD   ret = 0;

	SetCommMask(hDevice, EV_RXCHAR);

	bStatus = WaitCommEvent(hDevice, &ModemStat, NULL);
	if(ModemStat & EV_RXCHAR)
	{
		 if (!ReadFile (hDevice, (LPVOID)InBuf, sizeof(InBuf)-1, &dwLen, 0)) {
               RETAILMSG(GPRS_MSG, (TEXT("CmdGetResponse - ReadFile failed %d\n"), GetLastError()));
		 }
		 else
		 {
			  InBuf[dwLen]=NULL;
			  RETAILMSG(GPRS_MSG, (L"CmdGetResponse: buffer = %S\r\n",InBuf));
			  ret = 1;
		 }

	}
	RETAILMSG(GPRS_MSG, (L"CmdGetResponse: ret = %x\r\n",ret));

	 return ret;

}

