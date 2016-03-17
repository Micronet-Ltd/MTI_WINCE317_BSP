// Copyright 2010 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//   File:  GPSDrvr.cpp
//
//   GPS Driver
//  
//
//   Created by Anna Rayer  May 2010
//-----------------------------------------------------------------------------/*



#include <windows.h>
#include <memory.h>
#include <linklist.h>
#include <serdbg.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>
#include <gpio.h>
#include <bsp.h>
#include <args.h>

#include <winbase.h>
#include <pegdser.h>
#include <devload.h>
#include <omap_pmext.h>
#include "gps.h"


#include <ceddkex.h>
#include <omap_pmext.h>

// disable PREFAST warning for use of EXCEPTION_EXECUTE_HANDLER
#pragma warning (disable: 6320)

// disable PREFAST warning for empty _except block
#pragma warning (disable: 6322)

/* Debug Zones.
 */
#ifdef DEBUG

    #define DBG_INIT    0x0001
    #define DBG_OPEN    0x0002
    #define DBG_READ    0x0004
    #define DBG_WRITE   0x0008
    #define DBG_CLOSE   0x0010
    #define DBG_IOCTL   0x0020
    #define DBG_THREAD  0x0040
    #define DBG_EVENTS  0x0080
    #define DBG_CRITSEC 0x0100
    #define DBG_FLOW    0x0200
    #define DBG_IR      0x0400
    #define DBG_NOTHING 0x0800
    #define DBG_ALLOC   0x1000
    #define DBG_FUNCTION 0x2000
    #define DBG_WARNING 0x4000
    #define DBG_ERROR   0x8000

DBGPARAM dpCurSettings = {
    TEXT("Serial"), {
        TEXT("Init"),TEXT("Open"),TEXT("Read"),TEXT("Write"),
        TEXT("Close"),TEXT("Ioctl"),TEXT("Thread"),TEXT("Events"),
        TEXT("CritSec"),TEXT("FlowCtrl"),TEXT("Infrared"),TEXT("User Read"),
        TEXT("Alloc"),TEXT("Function"),TEXT("Warning"),TEXT("Error")},
    0
};
#endif
//==============================================================================================
//            Local Definitions
//==============================================================================================

#define GPS_MSG             0

#define GPS_POWER_EN		GPIO_55
#define PCA_GPIO_GPS_0		PCA_GPIO_96 
#define	USB_PWR_EN			PCA_GPIO_GPS_0+9

#ifndef MIN
    #define MIN(x,y)            ((x) < (y) ? (x) : (y))
#endif

// Forward declare for use below
//

//==============================================================================================
//            Device registry parameters
//==============================================================================================
static const DEVICE_REGISTRY_PARAM s_deviceInitRegParams[] = 
{
	{
		L"Priority256", PARAM_DWORD, FALSE, offset(GPS_INIT_CONTEXT, priority256),
		fieldsize(GPS_INIT_CONTEXT, priority256), (VOID*)250
	}, 
	{
		L"PowerMask", PARAM_DWORD, TRUE, offset(GPS_INIT_CONTEXT, powerMask),
		fieldsize(GPS_INIT_CONTEXT, powerMask), 0
	},
	{
		L"StartupOn", PARAM_DWORD, FALSE, offset(GPS_INIT_CONTEXT, StartupOn),
		fieldsize(GPS_INIT_CONTEXT, StartupOn), 0
	},
	{
		L"EnableExternalGPS", PARAM_DWORD, FALSE, offset(GPS_INIT_CONTEXT, fEnableExternalGPS),
		fieldsize(GPS_INIT_CONTEXT, fEnableExternalGPS), 0
	}
};

//==============================================================================================
//           Local Functions
//==============================================================================================

BOOL	GpsDeinit ( DWORD dwContext );
DWORD	SetInitConfig(PGPS_INIT_CONTEXT pCxt);
BOOL	COM_Close(PCV_OPEN_INFO    pOpenHead);
BOOL	COM_Deinit(PCV_INDEP_INFO pSerialHead);
BOOL	ComReOpen(PCV_INDEP_INFO pSerialHead);
BOOL	CloseHandles(PCV_INDEP_INFO pSerialHead);
BOOL	CloseAll(PCV_INDEP_INFO pSerialHead);
BOOL	IsDevReady(PCV_INDEP_INFO pSerialHead);
BOOL	IsReady(PCV_INDEP_INFO pSerialHead);
void	ChangeError(PCV_INDEP_INFO pSerialHead);
BOOL	GpsOnOff(PGPS_INIT_CONTEXT	pGpsContext, DWORD Flag);
BOOL	SetState(PGPS_INIT_CONTEXT	pGpsContext, DWORD Act);

BOOL	GpsOnOff(PGPS_INIT_CONTEXT	pGpsContext, DWORD Flag)
{
	if(Flag)//on
	{
		GPIOSetBit(pGpsContext->hGpio, GPIO_164); // PWR_EN
		GPIOSetBit(pGpsContext->hGpio, GPIO_163); // USBVDD
	}
	else
	{
		GPIOClrBit(pGpsContext->hGpio, GPIO_163); // USBVDD
		GPIOClrBit(pGpsContext->hGpio, GPIO_164); // PWR_EN
	}
	RETAILMSG(GPS_MSG,(L"GPS: Powered %s\r\n", (Flag ? L"On" : L"Off")));
	return 1;
}

DWORD SetState(PCV_INDEP_INFO pSerialHead, DWORD act)
{
	DWORD ret = 1;

	EnterCriticalSection(&(pSerialHead->StatCritSec));
	RETAILMSG(GPS_MSG,(L"GPS: SetState %d\r\n", act));
	switch(act)
	{
		case	DEV_OFF:
		{
			RETAILMSG(GPS_MSG,(L""));
			if(pSerialHead->pGpsContext->fOn)
			{
				InterlockedExchange((LONG*)&pSerialHead->pGpsContext->fOn, 0);
				if(NOTREADY_ST != pSerialHead->DevState)
					GpsOnOff(pSerialHead->pGpsContext, 0);
			}
			SetEvent(pSerialHead->hSerialEventOff);
			CloseHandles(pSerialHead);
			InterlockedExchange((LONG*)&pSerialHead->DevState, OFF_ST);
			RETAILMSG(GPS_MSG,(L"GPS: SetState OFF_ST\r\n"));
		}
		break;
		case	DEV_ON:
		{
			if(!pSerialHead->pGpsContext->fOn)
			{
				if(NOTREADY_ST != pSerialHead->DevState)
				{
					GpsOnOff(pSerialHead->pGpsContext, TRUE);
					if(WAIT_OBJECT_0 != WaitForSingleObject(pSerialHead->hSerialEventAct, 3000))//try to wait
						ret = 0;
					InterlockedExchange((LONG*)&pSerialHead->DevState, REOPEN_ST);
					RETAILMSG(GPS_MSG,(L"GPS: SetState REOPEN_ST\r\n"));
				}
				InterlockedExchange((LONG*)&pSerialHead->pGpsContext->fOn, 1);
			}
			ResetEvent(pSerialHead->hSerialEventOff);			
		}
		break;
		case 	PWR_OFF:
		{
			ResetEvent(pSerialHead->hSerialEventAct);
			if(	pSerialHead->pGpsContext->fOn		&&
				NOTREADY_ST != pSerialHead->DevState)
			{
				GpsOnOff(pSerialHead->pGpsContext,0);
				InterlockedExchange((LONG*)&pSerialHead->DevState, NOTREADY_ST);
				RETAILMSG(GPS_MSG,(L"GPS: SetState NOTREADY_ST\r\n"));
			}
			CloseHandles(pSerialHead);
		}
		break;
		case 	PWR_ON:
		{
			if(pSerialHead->pGpsContext->fOn) 
			{
				if(NOTREADY_ST == pSerialHead->DevState)
				{
					GpsOnOff(pSerialHead->pGpsContext, 1);
					InterlockedExchange((LONG*)&pSerialHead->DevState, REOPEN_ST);
					RETAILMSG(GPS_MSG,(L"GPS: SetState REOPEN_ST\r\n"));
				}
			}
		}
		break;
		case NORMAL_ON:
		{	
			if(	pSerialHead->pGpsContext->fOn			&&
				NOTREADY_ST != pSerialHead->DevState	)
			{	
				InterlockedExchange((LONG*)&pSerialHead->DevState, NORMAL_ST);
				RETAILMSG(GPS_MSG,(L"GPS: SetState NORMAL_ST\r\n"));
			}
		}
		break;
		default:
			ret = 0;
		break;
	}

	LeaveCriticalSection(&(pSerialHead->StatCritSec));
	return ret;
}


void SetDefaultDCB(PCV_INDEP_INFO pSerialHead)
{
    pSerialHead->DCB.DCBlength  = sizeof(DCB);
    pSerialHead->DCB.BaudRate   = 9600;
    pSerialHead->DCB.fBinary    = TRUE;
    pSerialHead->DCB.fParity    = FALSE;

    pSerialHead->DCB.fOutxCtsFlow = FALSE;
    pSerialHead->DCB.fOutxDsrFlow = FALSE;
    pSerialHead->DCB.fDtrControl = DTR_CONTROL_ENABLE;
    pSerialHead->DCB.fDsrSensitivity = FALSE;
    pSerialHead->DCB.fTXContinueOnXoff = FALSE;
    pSerialHead->DCB.fOutX      = FALSE;
    pSerialHead->DCB.fInX       = FALSE;
    pSerialHead->DCB.fErrorChar = FALSE; //NOTE: ignored
    pSerialHead->DCB.fNull      = FALSE; //NOTE: ignored
    pSerialHead->DCB.fRtsControl = RTS_CONTROL_ENABLE;
    pSerialHead->DCB.fAbortOnError = FALSE; //NOTE: ignored

    pSerialHead->DCB.XonLim     = (WORD)(RX_BUFFER_SIZE >> 1) ; // Line up with XP code.
    pSerialHead->DCB.XoffLim    = (WORD)(RX_BUFFER_SIZE >> 3) ;
    if( RX_BUFFER_SIZE - pSerialHead->DCB.XoffLim <= pSerialHead->DCB.XonLim ) { // Checking for illgle value
        pSerialHead->DCB.XonLim  = (WORD)(RX_BUFFER_SIZE - pSerialHead->DCB.XoffLim -1)  ;
    }

    pSerialHead->DCB.ByteSize   = 8;
    pSerialHead->DCB.Parity     = NOPARITY;
    pSerialHead->DCB.StopBits   = ONESTOPBIT;

    pSerialHead->DCB.XonChar    = X_ON_CHAR;
    pSerialHead->DCB.XoffChar   = X_OFF_CHAR;
    pSerialHead->DCB.ErrorChar  = ERROR_CHAR;
    pSerialHead->DCB.EofChar    = E_OF_CHAR;
    pSerialHead->DCB.EvtChar    = EVENT_CHAR;

    pSerialHead->StopXmit = 0;
    pSerialHead->SentXoff = 0;
    pSerialHead->DtrFlow = 0;
    pSerialHead->RtsFlow = 0;

	pSerialHead->SetResetDTR = 0;
	pSerialHead->SetResetRTS = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
DWORD GpsInit ( ULONG pContext )
{
	DEBUGMSG ( DBG_FUNCTION, ( _T ( "GPS: +GpsInit, pContext=%s\r\n" ), pContext ) );
	
	GPS_INIT_CONTEXT *pCxt =0;
	card_ver			 gps_cv;
	UINT32               in;
	
	// allocate context object
	if((pCxt = (GPS_INIT_CONTEXT *)LocalAlloc(LPTR, sizeof(GPS_INIT_CONTEXT))) == NULL )
	{
		RETAILMSG(1, (L"GPS: LocalAlloc failed\r\n"));
		return 0;
	}

	// Read device parameters
    if(GetDeviceRegistryParams((LPCWSTR)pContext, pCxt, dimof(s_deviceInitRegParams), s_deviceInitRegParams) != ERROR_SUCCESS)
	{
        RETAILMSG(1, (L"GPS: ERROR: GpsInit: Failed read GPS_INIT_CONTEXT driver registry parameters\r\n"));
        GpsDeinit((DWORD)pCxt);
		return 0;
	}

	in = OAL_ARGS_QUERY_GPS_CFG;

	if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &gps_cv, sizeof(gps_cv), 0))
    {
		RETAILMSG(1, (L"GPS: GpsInit: IOCTL_HAL_QUERY_CARD_CFG calling failure.\r\n"));
		return 0;
	}

	if (!pCxt->fEnableExternalGPS && gps_cv.ver == -1)
    {
		RETAILMSG(GPS_MSG, (L"GPS: GpsInit: GPS card  is not connected\r\n"));
		return 0;
	}

	wcscpy(pCxt->szPortName, pCxt->fEnableExternalGPS ? TEXT("TLT7:") : TEXT("COV1:"));
	pCxt->dwGPSVersion = gps_cv.ver;

	RETAILMSG(GPS_MSG, (L"GPS: GpsInit modem version = %d\r\n", pCxt->dwGPSVersion  ));
	
	if (!pCxt->fEnableExternalGPS)
	{
		// Open gpio driver
		pCxt->hGpio = GPIOOpen();
		if(pCxt->hGpio == NULL)
		{
			RETAILMSG(1, (L"GPS: ERROR: GpsInit: " L"Failed to open Gpio driver \r\n"));
			GpsDeinit((DWORD)pCxt);
			return 0;
		}

		if(!SetInitConfig(pCxt))
		{
			RETAILMSG(GPS_MSG, (L"GPS: SetInitConfig failed\r\n"));
			GpsDeinit((DWORD)pCxt);
			return 0;
		}
		pCxt->powerState = D4;
	}

    RETAILMSG(GPS_MSG, (_T("GPS: -GpsInit\r\n")));
	return (DWORD)pCxt;
}


//------------------------------------------------------------------------------
// Function name	: GpsDeinit
// Description	    : DeInitialization of all used resources
// Return type		: BOOL  
// Argument         : DWORD dwContext
//------------------------------------------------------------------------------
BOOL GpsDeinit ( DWORD dwContext )
{
	GPS_INIT_CONTEXT *pInitCtx = (GPS_INIT_CONTEXT *)dwContext;

	DEBUGMSG ( DBG_FUNCTION, ( _T ( "GPS: +GpsDeinit, dwContext=%d\r\n" ), dwContext ) );
	RETAILMSG(GPS_MSG, (_T("GPS: +GpsDeinit, dwContext=%d\r\n" ), dwContext ) );

	// Check if we get correct context
    if(!pInitCtx )//|| (pInitCtx->cookie != GTSR_DEVICE_COOKIE))
	{
        RETAILMSG(GPS_MSG, (L"GPS: ERROR: Deinit: Incorrect context parameter\r\n"));
        return 0;
	}

    if(pInitCtx->hGpio) 
		GPIOClose(pInitCtx->hGpio);
	
	LocalFree(pInitCtx);

	DEBUGMSG(DBG_FUNCTION, (_T("GPS: -Deinit\r\n")) );
    return TRUE;
}

DWORD SetInitConfig(PGPS_INIT_CONTEXT pCxt)
{
	DWORD ret = 1;

	RETAILMSG(GPS_MSG, (L"GPS: SetInitConfig: started.. \r\n"));

	GPIOSetMode(pCxt->hGpio, GPIO_163, GPIO_DIR_OUTPUT); // USBVDD
	GPIOSetMode(pCxt->hGpio, GPIO_164, GPIO_DIR_OUTPUT); // PWR_EN

	return ret;
 }
//////////////////////////////////////////////////////////////////////////////////////////
BOOL DllEntry(
              HINSTANCE   hinstDll,             /*@parm Instance pointer. */
              DWORD   dwReason,                 /*@parm Reason routine is called. */
              LPVOID  lpReserved                /*@parm system parameter. */
              )
{
    if( dwReason == DLL_PROCESS_ATTACH ) {
        DEBUGREGISTER(hinstDll);
        DEBUGMSG (ZONE_INIT, (TEXT("serial port process attach\r\n")));
        DisableThreadLibraryCalls((HMODULE) hinstDll);
    }

    if( dwReason == DLL_PROCESS_DETACH ) {
        DEBUGMSG (ZONE_INIT, (TEXT("process detach called\r\n")));
    }

    return(TRUE);
}

HANDLE COM_Init(ULONG   Identifier) 
{
    PCV_INDEP_INFO  pSerialHead = NULL;

	DEBUGMSG (ZONE_INIT | ZONE_FUNCTION, (TEXT("GPS: +COM_Init\r\n")));
	RETAILMSG(GPS_MSG, (TEXT("GPS: +COM_Init\r\n")));

    // Allocate our control structure.
    pSerialHead  =  (PCV_INDEP_INFO)LocalAlloc(LPTR, sizeof(CV_INDEP_INFO));
	
	if(!pSerialHead)
	{
		RETAILMSG(1, (TEXT("GPS: -COM_Init allocation error\r\n")));
		return 0;
	}

	pSerialHead->pGpsContext = (GPS_INIT_CONTEXT*)GpsInit(Identifier);
    if(!pSerialHead->pGpsContext)
	{
		LocalFree(pSerialHead);
		RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_Init GpsInit error\r\n")));
		return 0;
	}

	GpsOnOff(pSerialHead->pGpsContext, 0);

	if(pSerialHead->pGpsContext->StartupOn && !pSerialHead->pGpsContext->fEnableExternalGPS)//temp!!!
	{
		SetState(pSerialHead, DEV_ON);
		pSerialHead->pGpsContext->powerState = D0;
	}
	// Initially, open list is empty.
    InitializeListHead( &pSerialHead->OpenList );
    InitializeCriticalSection(&(pSerialHead->OpenCS));
    InitializeCriticalSection(&(pSerialHead->StatCritSec));
    InitializeCriticalSection(&(pSerialHead->ReceiveCritSec1));
    InitializeCriticalSection(&(pSerialHead->TransmitCritSec1));

	pSerialHead->hSerialEventOff	= CreateEvent(0, 1, 0, 0);
	pSerialHead->hSerialEventAct	= CreateEvent(0, 1, 0, L"usbser.attach.event");
	pSerialHead->hReadEvent			= CreateEvent(0, 0, 0, 0);
	pSerialHead->hTransmitEvent		= CreateEvent(0, 0, 0, 0);

    if( !pSerialHead->hSerialEventOff	|| !pSerialHead->hSerialEventAct || 
		!pSerialHead->hReadEvent		|| !pSerialHead->hTransmitEvent	)
	{
        DEBUGMSG(ZONE_ERROR | ZONE_INIT,  (TEXT("GPS: Error creating event, COM_Init failed\n\r")));
 		RETAILMSG(1, (TEXT("GPS: -COM_Init failed. Error creating event\r\n")));
		COM_Deinit(pSerialHead);
        return (NULL);
    }

	pSerialHead->CommTimeouts.ReadIntervalTimeout			= READ_TIMEOUT;
    pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier	= READ_TIMEOUT_MULTIPLIER;
    pSerialHead->CommTimeouts.ReadTotalTimeoutConstant		= READ_TIMEOUT_CONSTANT;
    pSerialHead->CommTimeouts.WriteTotalTimeoutMultiplier	= 0;
    pSerialHead->CommTimeouts.WriteTotalTimeoutConstant		= 0;

	DEBUGMSG (ZONE_INIT | ZONE_FUNCTION, (TEXT("GPS: -COM_Init\r\n")));
	return(pSerialHead);
}

BOOL ApplyDCB (PCV_INDEP_INFO pSerialHead, DCB *pDCB, BOOL fOpen)
{
//    if(!(pDCB->XoffLim < pSerialHead->RxBufferInfo.Length &&
//         pDCB->XonLim < pSerialHead->RxBufferInfo.Length - pDCB->XoffLim )) 
    
    if(pDCB->fOutX || pDCB->fInX ) 
	{
        if(pDCB->XonChar == pDCB->XoffChar)
			return FALSE;
    } 

	return(TRUE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HANDLE COM_Open(HANDLE  pHead,          // @parm Handle returned by COM_Init.
				DWORD   AccessCode,     // @parm access code.
				DWORD   ShareMode	)   // @parm share mode - Not used in this driver.
{
    PCV_INDEP_INFO  pSerialHead = (PCV_INDEP_INFO)pHead;
    PCV_OPEN_INFO   pOpenHead;
	
    DEBUGMSG(ZONE_OPEN|ZONE_FUNCTION, (TEXT("GPS: +COM_Open handle x%X, access x%X, share x%X\r\n"), pHead, AccessCode, ShareMode));
    RETAILMSG(GPS_MSG, (TEXT("GPS: +COM_Open handle x%X, access x%X, share x%X\r\n"), pHead, AccessCode, ShareMode));

    // Return NULL if SerialInit failed.
    if( !pSerialHead ) 
	{
        DEBUGMSG (ZONE_OPEN|ZONE_ERROR,(TEXT("Open attempted on uninited device!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return (NULL);
    }

    if(AccessCode & DEVACCESS_BUSNAMESPACE ) 
	{
        AccessCode &=~(GENERIC_READ |GENERIC_WRITE|GENERIC_EXECUTE|GENERIC_ALL);
    }

    // Return NULL if opening with access & someone else already has
    if( (AccessCode & (GENERIC_READ | GENERIC_WRITE)) && pSerialHead->pAccessOwner )
	{
		DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("Open requested access %x, handle x%X already has x%X!\r\n"),
				   AccessCode, pSerialHead->pAccessOwner,
				   pSerialHead->pAccessOwner->AccessCode));
		SetLastError(ERROR_INVALID_ACCESS);
		return(NULL);
	}
	IsReady(pSerialHead);//for ReOpen only

	// OK, lets allocate an open structure
    pOpenHead    =  (PCV_OPEN_INFO)LocalAlloc(LPTR, sizeof(CV_OPEN_INFO));
    if(!pOpenHead) 
	{
        DEBUGMSG(ZONE_INIT | ZONE_ERROR, (TEXT("Error allocating memory for pOpenHead, COM_Open failed\n\r")));
        return(NULL);
    }

	pOpenHead->hPort = (HANDLE)-1;
	    // We do special for Power Manger and Device Manager.
    if( !((AccessCode &  DEVACCESS_BUSNAMESPACE) || (ShareMode & (DEVACCESS_PMEXT_MODE | DEVACCESS_PM_MODE))) ) 
	{
 		if ((pSerialHead->pGpsContext->fOn && IsDevReady(pSerialHead)) || pSerialHead->pGpsContext->fEnableExternalGPS)
		{
			pOpenHead->hPort = CreateFile( pSerialHead->pGpsContext->szPortName, AccessCode, ShareMode, 0, OPEN_EXISTING, 0, NULL);
			if((HANDLE)-1 == pOpenHead->hPort) 
			{
				DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("GPS: Open failed err %d\r\n"), GetLastError()));
				RETAILMSG(GPS_MSG, (TEXT("GPS: Open failed err %d\r\n"), GetLastError()));
				return (NULL);
			}
			
			if(AccessCode & (GENERIC_READ | GENERIC_WRITE))
			{
				if(	!GetCommState(pOpenHead->hPort, &pSerialHead->DCB)					||
					!SetCommTimeouts(pOpenHead->hPort, &pSerialHead->CommTimeouts)		)
				{
					CloseHandle(pOpenHead->hPort);
					pOpenHead->hPort = (HANDLE)-1;
					DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("GPS: Com_Open GetComm parameters failed err %d\r\n"), GetLastError()));
					RETAILMSG(GPS_MSG, (TEXT("GPS: Com_Open GetComm parameters failed err %d\r\n"), GetLastError()));
					return (NULL);
				}
				pSerialHead->fOpened	 = 1;
				pSerialHead->SetResetDTR = 0;
				pSerialHead->SetResetRTS = 0;
			}
			SetState(pSerialHead, NORMAL_ON);
		}
		else if(!pSerialHead->OpenCnt)
			SetDefaultDCB(pSerialHead);
	}
	// Init the structure
    pOpenHead->pSerialHead			 = pSerialHead;  // pointer back to our parent
    pOpenHead->AccessCode			 = AccessCode;
    pOpenHead->ShareMode			 = ShareMode;
    pOpenHead->CommEvents.hCommEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    pOpenHead->CommEvents.fEventMask = 0;
    pOpenHead->CommEvents.fEventData = 0;
    pOpenHead->CommEvents.fAbort	 = 0;
    InitializeCriticalSection(&(pOpenHead->CommEvents.EventCS));

    // if we have access permissions, note it in pSerialhead
    if( AccessCode & (GENERIC_READ | GENERIC_WRITE) ) 
	{
        DEBUGMSG(ZONE_INIT|ZONE_CLOSE, (TEXT("COM_Open: Access permission handle granted x%X\n\r"), pOpenHead));
        pSerialHead->pAccessOwner = pOpenHead;
    }
	
	if( (AccessCode &  DEVACCESS_BUSNAMESPACE) || (ShareMode & (DEVACCESS_PMEXT_MODE | DEVACCESS_PM_MODE)) ) 
	{
	    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("GPS: -COM_Open handle x%X, x%X, Ref x%X\r\n"), pOpenHead, pOpenHead->pSerialHead, pSerialHead->OpenCnt));
		RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_Open handle x%X, x%X, Ref x%X\r\n"), pOpenHead, pOpenHead->pSerialHead, pSerialHead->OpenCnt));
    
		return(pOpenHead);
	}
    // add this open entry to list of open entries.
    EnterCriticalSection(&(pSerialHead->OpenCS));
  
	InsertHeadList(&pSerialHead->OpenList, &pOpenHead->llist);

    ++(pSerialHead->OpenCnt);

    // OK, we are finally back in a stable state.  Release the CS.
    LeaveCriticalSection(&(pSerialHead->OpenCS));

    DEBUGMSG (ZONE_OPEN|ZONE_FUNCTION, (TEXT("GPS: -COM_Open handle x%X, x%X, Ref x%X\r\n"), pOpenHead, pOpenHead->pSerialHead, pSerialHead->OpenCnt));
	RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_Open handle x%X, x%X, Ref x%X\r\n"), pOpenHead, pOpenHead->pSerialHead, pSerialHead->OpenCnt));
    
	return(pOpenHead);
}
// ****************************************************************
//      @func BOOL      | COM_PreClose | pre-close the serial device.
//      @parm DWORD | pHead             | Context pointer returned from COM_Open
//      @rdesc TRUE if success; FALSE if failure
//      @remark This routine is called by the device manager to close the device.
//
BOOL COM_PreClose(PCV_OPEN_INFO pOpenHead)
{
    PCV_INDEP_INFO  pSerialHead;

    RETAILMSG (GPS_MSG, (TEXT("GPS: +COM_PreClose %X\r\n"), pOpenHead));
    if(!pOpenHead || !pOpenHead->pSerialHead) 
	{
        DEBUGMSG (ZONE_ERROR, (TEXT("!!COM_PreClose: pSerialHead == NULL!!\r\n")));
        return FALSE;
    }
	pSerialHead = pOpenHead->pSerialHead;

    // Use the OpenCS to make sure we don't collide with an in-progress open.
    EnterCriticalSection(&(pSerialHead->OpenCS));

	if(!(pOpenHead->AccessCode & DEVACCESS_BUSNAMESPACE) && !(pOpenHead->ShareMode & (DEVACCESS_PMEXT_MODE | DEVACCESS_PM_MODE))) 
	{

		if( (HANDLE)-1 != pOpenHead->hPort)
		{
			SetCommMask(pOpenHead->hPort,0);
			CloseHandle(pOpenHead->hPort);
			pOpenHead->hPort =(HANDLE)-1;
		}
        if( pSerialHead->OpenCnt ) 
		{
            // For any open handle, we must free pending waitcommevents
            EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
            pOpenHead->CommEvents.fEventMask	= 0;
            pOpenHead->CommEvents.fAbort		= 1;
            SetEvent(pOpenHead->CommEvents.hCommEvent);
            LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

			// And only for the handle with access permissions do we have to worry about read, write, etc being blocked.
            if( pOpenHead->AccessCode & (GENERIC_READ | GENERIC_WRITE) ) 
			{
                pSerialHead->fAbortRead	=	1;
                SetEvent(pSerialHead->hReadEvent);
                pSerialHead->fAbortTransmit = 1;
                SetEvent(pSerialHead->hTransmitEvent);
            }
            DEBUGMSG(ZONE_CLOSE|ZONE_INIT|ZONE_ERROR, (TEXT("COM_PreClose: serial users to exit\n\r")));
        }
		while(pOpenHead->WaitUsers)
			Sleep(0);
   }

	LeaveCriticalSection(&(pSerialHead->OpenCS));

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("GPS: -COM_PreClose\r\n")));
    RETAILMSG (GPS_MSG, (TEXT("GPS: -COM_PreClose %X\r\n"), pOpenHead));
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL COM_Close(PCV_OPEN_INFO pOpenHead)
{
    PCV_INDEP_INFO  pSerialHead = pOpenHead->pSerialHead;
    BOOL            RetCode = TRUE;

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("GPS: +COM_Close\r\n")));
    RETAILMSG(GPS_MSG, (TEXT("GPS: +COM_Close %x %x\r\n"), pOpenHead, pOpenHead->hPort));

    if( !pSerialHead ) 
	{
        DEBUGMSG (ZONE_ERROR, (TEXT("!!COM_Close: pSerialHead == NULL!!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
	if( (HANDLE)-1 != pOpenHead->hPort)
	{
		CloseHandle(pOpenHead->hPort);
		pOpenHead->hPort =(HANDLE)-1;
	}

    // Use the OpenCS to make sure we don't collide with an in-progress open.
    EnterCriticalSection(&(pSerialHead->OpenCS));

	if(!(pOpenHead->AccessCode & DEVACCESS_BUSNAMESPACE) && !(pOpenHead->ShareMode & (DEVACCESS_PMEXT_MODE | DEVACCESS_PM_MODE))) 
	{
		if( pSerialHead->OpenCnt ) 
		{
			--(pSerialHead->OpenCnt);

			// If this was the handle with access permission, remove pointer
			if( pOpenHead == pSerialHead->pAccessOwner ) 
			{
				DEBUGMSG(ZONE_INIT|ZONE_CLOSE,(TEXT("COM_Close: Closed access owner handle\n\r"), pOpenHead));

				pSerialHead->pAccessOwner	= 0;
			}

			// Remove the entry from the linked list
			RemoveEntryList(&pOpenHead->llist);
			DeleteCriticalSection(&(pOpenHead->CommEvents.EventCS));

			if( pOpenHead->CommEvents.hCommEvent )
				CloseHandle(pOpenHead->CommEvents.hCommEvent);

			if(!pSerialHead->OpenCnt)
			{
				SetDefaultDCB(pSerialHead);
				pSerialHead->fOpened = 0;
			}
		}
	}
    LocalFree( pOpenHead );

    LeaveCriticalSection(&(pSerialHead->OpenCS));

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("GPS: -COM_Close\r\n")));
    RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_Close %x\r\n"), pOpenHead));
    
	return(RetCode);
}

/*
 @doc EXTERNAL
 @func  BOOL | COM_PreDeinit | Pre-De-initialize serial port.
 @parm DWORD | pSerialHead | Context pointer returned from COM_Init
 *
 @rdesc None.
 */
BOOL COM_PreDeinit(PCV_INDEP_INFO pSerialHead)
{
	DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("GPS: +COM_PreDeinit\r\n")));

    if( !pSerialHead ) 
	{
        DEBUGMSG (ZONE_INIT|ZONE_ERROR,(TEXT("COM_PreDeinit can't find pSerialHead\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }
    
	SetState(pSerialHead, DEV_OFF);

   // Call PreClose, if we have a user.  Note that this call will ensure that
    // all users are out of the serial routines before it returns.

     if( pSerialHead->OpenCnt ) 
	 {
		PLIST_ENTRY     pEntry;
        PCV_OPEN_INFO   pOpenHead;

        EnterCriticalSection(&(pSerialHead->OpenCS));
        
		pEntry = pSerialHead->OpenList.Flink;
        while ( pEntry != &pSerialHead->OpenList ) 
		{
            pOpenHead = CONTAINING_RECORD( pEntry, CV_OPEN_INFO, llist);
            pEntry = pEntry->Flink;  // advance to next

            DEBUGMSG (ZONE_INIT | ZONE_CLOSE, (TEXT(" PreDeinit -Pre Closing Handle 0x%X\r\n"), pOpenHead ));

			COM_PreClose((PCV_OPEN_INFO)pOpenHead->hPort);
		}
   
		LeaveCriticalSection(&(pSerialHead->OpenCS));
	}

    return TRUE;
}
/*
 @doc EXTERNAL
 @func  BOOL | COM_Deinit | De-initialize serial port.
 @parm DWORD | pSerialHead | Context pointer returned from COM_Init
 *
 @rdesc None.
 */
BOOL COM_Deinit(PCV_INDEP_INFO pSerialHead)
{
    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("GPS: +COM_Deinit\r\n")));
    RETAILMSG(GPS_MSG, (TEXT("GPS: +COM_Deinit\r\n")));

    if( !pSerialHead ) 
	{
        DEBUGMSG (ZONE_INIT|ZONE_ERROR, (TEXT("COM_Deinit can't find pSerialHead\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

	CloseAll(pSerialHead);
 
    // Free our resources
    if( pSerialHead->hSerialEventOff )
        CloseHandle(pSerialHead->hSerialEventOff);

    if( pSerialHead->hSerialEventAct )
        CloseHandle(pSerialHead->hSerialEventAct);

    if( pSerialHead->hReadEvent )
        CloseHandle(pSerialHead->hReadEvent);
    if( pSerialHead->hReadEvent )
        CloseHandle(pSerialHead->hTransmitEvent);

	DeleteCriticalSection(&(pSerialHead->OpenCS));
	DeleteCriticalSection(&(pSerialHead->StatCritSec));
	DeleteCriticalSection(&(pSerialHead->ReceiveCritSec1));
	DeleteCriticalSection(&(pSerialHead->TransmitCritSec1));

	GpsDeinit((DWORD)pSerialHead->pGpsContext);

    LocalFree(pSerialHead);

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("GPS: -COM_Deinit\r\n")));
    RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_Deinit\r\n")));
    return(TRUE);
}

ULONG COM_Read(HANDLE pHead, PUCHAR pTargetBuffer, ULONG BufferLength)
{
	PCV_OPEN_INFO   pOpenHead = (PCV_OPEN_INFO)pHead;
	PCV_INDEP_INFO  pSerialHead;// = pOpenHead->pSerialHead;
	BOOL			ret;
    ULONG           Ticks;
    ULONG           Timeout;
    ULONG           BytesRead = 0;
    ULONG           IntervalTimeout;    // The interval timeout
    ULONG           AddIntervalTimeout;
    ULONG           TotalTimeout;       // The Total Timeout
    ULONG           TimeSpent = 0;      // How much time have we been waiting?
	HANDLE			hEv[3];

	DEBUGMSG (ZONE_USR_READ|ZONE_FUNCTION, (TEXT("GPS: +COM_READ(0x%X,0x%X,%d)\r\n"), pHead, pTargetBuffer, BufferLength));
	RETAILMSG(GPS_MSG, (TEXT("GPS: +COM_READ(0x%X,0x%X,%d)\r\n"), pHead, pTargetBuffer, BufferLength));
    
    // Check to see that the call is valid.
    if( !pOpenHead || !pOpenHead->pSerialHead || !pOpenHead->pSerialHead->OpenCnt ) 
	{
        DEBUGMSG (ZONE_USR_READ|ZONE_ERROR, (TEXT("COM_READ, device not open\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return	(ULONG)-1;
    }
    pSerialHead = pOpenHead->pSerialHead;

	if(!pSerialHead->pGpsContext->fOn && !pSerialHead->pGpsContext->fEnableExternalGPS)
	{
        DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT("GPS: COM_Read: device was powerd off\r\n")));
        SetLastError(ERROR_NOT_READY);
        return	(ULONG)-1;
	}
   // Make sure the caller has access permissions
    if(!(pOpenHead->AccessCode & GENERIC_READ)) 
	{
        DEBUGMSG(ZONE_USR_READ|ZONE_ERROR, (TEXT("COM_Read: Access permission failure x%X\n\r"), pOpenHead->AccessCode));
        SetLastError (ERROR_INVALID_ACCESS);
		return	(ULONG)-1;
    }
    InterlockedIncrement(&pOpenHead->WaitUsers);

	hEv[0] = pSerialHead->hSerialEventAct;
	hEv[1] = pSerialHead->hSerialEventOff;
	hEv[2] = pSerialHead->hReadEvent;

	EnterCriticalSection(&(pSerialHead->ReceiveCritSec1));
    pSerialHead->fAbortRead = 0;

	Ticks = GetTickCount();
	// Compute total time to wait. Take product and add constant.
    if(MAXDWORD != pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier) 
	{
        TotalTimeout = pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier * BufferLength +
                       pSerialHead->CommTimeouts.ReadTotalTimeoutConstant;
        // Because we are using FIFO and water level is set to 8, we have to do following
        AddIntervalTimeout = pSerialHead->CommTimeouts.ReadTotalTimeoutMultiplier * 8;
    } 
	else 
	{
        TotalTimeout		= pSerialHead->CommTimeouts.ReadTotalTimeoutConstant;
        AddIntervalTimeout	= 0;
    }
    IntervalTimeout = pSerialHead->CommTimeouts.ReadIntervalTimeout;
    if((IntervalTimeout < MAXDWORD - AddIntervalTimeout) && (IntervalTimeout != 0)) 
	{
        IntervalTimeout += AddIntervalTimeout;
    };
/////////////////////////////////////////////////////
    while(BufferLength) 
	{
 		if(IsReady(pSerialHead))
		{
			if((HANDLE)-1 == pOpenHead->hPort)
			{
				SetLastError(ERROR_INVALID_HANDLE);
				BytesRead = (ULONG)-1;
				break;
			}
            Ticks = GetTickCount();

			ret = ReadFile(pOpenHead->hPort, pTargetBuffer, BufferLength, &BytesRead, 0);

			if(pSerialHead->fAbortRead)
			{
				RETAILMSG(GPS_MSG, (TEXT("GPS: -Abort COM_READ err %d\r\n"), GetLastError()));
				BytesRead = -1;
				break;
			}
			if(IsDevReady(pSerialHead) || ret || !pSerialHead->pGpsContext->fOn) 
			{
				RETAILMSG(GPS_MSG, (TEXT("GPS: -- COM_READ err %d\r\n"), GetLastError()));
				break; // COV is working or device closed	
			}
			BytesRead = 0;//error on COV not ready - continue

			Ticks = GetTickCount() - Ticks;
			TimeSpent += Ticks;
		} 
		else 
		{
            // Wait for a serial event?
            if(IntervalTimeout == MAXDWORD)
			{
				// Special Case see Remarks of COMMTIMEOUTS
                if(TotalTimeout == 0)
                    // For some reason this means don't wait.
                    break;
                //else if(BytesRead != 0) // There is data in the buffer or has been readed.
                //    break;
            }

            Timeout = (TotalTimeout != 0 ? TotalTimeout : MAXDWORD);
            // Total timeout is valid
            if( TimeSpent >= Timeout ) 
			{
                // Timed out.
                break;
            }
            Timeout -= TimeSpent;
            // On first byte we only use interval timeout
            // on subsequent we use minimum of Interval and Timeout
            //if( BytesRead) 
			//{
            //    Timeout = MIN(Timeout, (IntervalTimeout!=0?IntervalTimeout:MAXDWORD));
            //}
            Ticks = GetTickCount();
            DEBUGMSG (ZONE_USR_READ, (TEXT("About to wait %dms\r\n"), Timeout));

			// Wait for an event from COV, or from SetCommMask
			ret = WaitForMultipleObjects(3, hEv, 0, Timeout); 
			

			// Since ticks is a ULONG this handles wrap.
            Ticks		= GetTickCount() - Ticks;
            TimeSpent	+= Ticks;

            // In the absense of WaitForMultipleObjects, we use flags to handle errors/aborts. Check for aborts or asynchronous closes.
            if(pSerialHead->fAbortRead) 
			{
                DEBUGMSG(ZONE_USR_READ,(TEXT("COM_Read - Aborting read\r\n")));
                break;
            }

			if(!pSerialHead->pGpsContext->fOn && !pSerialHead->pGpsContext->fEnableExternalGPS)
			{
				DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT("GPS: COM_Read: device was powerd off\r\n")));
				SetLastError(ERROR_NOT_READY);
				BytesRead = -1;
				break;
			}

			if(!pSerialHead->OpenCnt)//already closed
			{
                DEBUGMSG(ZONE_USR_READ|ZONE_ERROR, (TEXT("COM_Read - device was closed\n\r")));
                SetLastError(ERROR_INVALID_HANDLE);
                break;
            }
			if(WAIT_OBJECT_0 == ret)//COV is ready
				continue;
			if(WAIT_TIMEOUT == ret)
				break;
        }
    }

	RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_READ err %d\r\n"), GetLastError()));
	
	ChangeError(pSerialHead);
    
	LeaveCriticalSection(&(pSerialHead->ReceiveCritSec1));
    InterlockedDecrement(&pOpenHead->WaitUsers);

	return BytesRead;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
ULONG COM_Write(HANDLE pHead, PUCHAR pSourceBytes, const ULONG  NumberOfBytes)
{
    PCV_OPEN_INFO   pOpenHead		= (PCV_OPEN_INFO)pHead;
    PCV_INDEP_INFO  pSerialHead		= pOpenHead->pSerialHead;
    ULONG           TotalWritten	= 0;
    ULONG			TotalTimeout;   // The Total Timeout
    ULONG			Timeout, Ticks, TimeSpent = 0;        // The Timeout value actually used
	HANDLE			hEv[3];
	ULONG			ret;

	DEBUGMSG (ZONE_WRITE|ZONE_FUNCTION, (TEXT("GPS: +COM_WRITE(0x%X, 0x%X, %d)\r\n"), pHead, pSourceBytes, NumberOfBytes));
    RETAILMSG(GPS_MSG, (TEXT("GPS: +COM_WRITE(0x%X, 0x%X, %d)\r\n"), pHead, pSourceBytes, NumberOfBytes));

    // Check validity of handle
    if(!pSerialHead) 
	{
        DEBUGMSG (ZONE_WRITE|ZONE_ERROR, (TEXT("COM_WRITE, device not open\r\n") ));
        SetLastError (ERROR_INVALID_HANDLE);
        return (ULONG)-1;
    }

	if(!pSerialHead->pGpsContext->fOn && !pSerialHead->pGpsContext->fEnableExternalGPS)
	{
        DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT("GPS: COM_Write: device was powerd off\r\n")));
        SetLastError(ERROR_NOT_READY);
        return (ULONG)-1;
	}
    // Make sure the caller has access permissions
    if( !(pOpenHead->AccessCode & GENERIC_WRITE) ) 
	{
		DEBUGMSG(ZONE_USR_READ|ZONE_ERROR, (TEXT("GPS: COM_Write: Access permission failure x%X\n\r"), pOpenHead->AccessCode));
		RETAILMSG(GPS_MSG, (TEXT("GPS: COM_Write: Access permission failure x%X\n\r"), pOpenHead->AccessCode));
        SetLastError (ERROR_INVALID_ACCESS);
        return (ULONG)-1;
    }
    InterlockedIncrement(&pOpenHead->WaitUsers);

	hEv[0] = pSerialHead->hSerialEventAct;
	hEv[1] = pSerialHead->hSerialEventOff;
	hEv[2] = pSerialHead->hTransmitEvent;

    EnterCriticalSection(&(pSerialHead->TransmitCritSec1));
    pSerialHead->fAbortTransmit = 0;

	// Clear any pending event
    WaitForSingleObject(pSerialHead->hTransmitEvent, 0);
   // Make sure an event isn't hanging around from a previous write time out.
    ResetEvent( pSerialHead->hTransmitEvent );
////////
    TotalTimeout = pSerialHead->CommTimeouts.WriteTotalTimeoutMultiplier * NumberOfBytes +
                   pSerialHead->CommTimeouts.WriteTotalTimeoutConstant;

    if( !TotalTimeout )
        Timeout = INFINITE;
    else
        Timeout = TotalTimeout;

	DEBUGMSG (ZONE_WRITE, (TEXT("GPS: COM_Write wait for transmission complete event %x.\r\n"),
                           pSerialHead->hTransmitEvent));

////////////
	while(1)
	{
		if(IsReady(pSerialHead))
		{
			if((HANDLE)-1 == pOpenHead->hPort)
			{
				SetLastError(ERROR_INVALID_HANDLE);
				TotalWritten = (ULONG)-1;
				break;
			}

			Ticks = GetTickCount();
			ret = WriteFile(pOpenHead->hPort, pSourceBytes, NumberOfBytes, &TotalWritten, 0);

			if(pSerialHead->fAbortTransmit) 
			{
				DEBUGMSG(ZONE_USR_READ,(TEXT("GPS: COM_Write - Aborting write\r\n")));
				TotalWritten = -1;
				break;
			}
			
			if( ret || IsDevReady(pSerialHead))
				break;

			Ticks = GetTickCount() - Ticks;
            TimeSpent += Ticks;
			
			if(Timeout != INFINITE)
				Timeout -= TimeSpent;
		}
		else
		{
			Ticks = GetTickCount();
			ret = WaitForMultipleObjects(3, hEv, 0, Timeout); 
			

            // Since ticks is a ULONG this handles wrap.
            Ticks = GetTickCount() - Ticks;
            TimeSpent += Ticks;
			
			if(Timeout != INFINITE)
				Timeout -= TimeSpent;

			// In the absense of WaitForMultipleObjects, we use flags to
			// handle errors/aborts. Check for aborts or asynchronous closes.
			if(pSerialHead->fAbortTransmit) 
			{
				DEBUGMSG(ZONE_USR_READ,(TEXT("GPS: COM_Write - Aborting write\r\n")));
				break;
			}

			if(!pSerialHead->pGpsContext->fOn && !pSerialHead->pGpsContext->fEnableExternalGPS)
			{
				DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT("GPS: COM_Write: device was powerd off\r\n")));
				SetLastError(ERROR_NOT_READY);
				TotalWritten = (ULONG)-1;
				break;
			}
			if(!pSerialHead->OpenCnt) 
			{
				DEBUGMSG(ZONE_WRITE|ZONE_ERROR,	 (TEXT("GPS: COM_Write - device was closed\n\r")));
				SetLastError(ERROR_INVALID_HANDLE);
				TotalWritten = (ULONG)-1;
				break;
			}
			if(WAIT_OBJECT_0 == ret)//COV is ready
				continue;
			if(WAIT_TIMEOUT == ret)
				break;
		}
	}

	LeaveCriticalSection(&(pSerialHead->TransmitCritSec1));

	RETAILMSG(GPS_MSG, (TEXT("GPS: -COM_WRITE err %d)\r\n"), GetLastError()));
	
	ChangeError(pSerialHead);
    InterlockedDecrement(&pOpenHead->WaitUsers);
	
	return TotalWritten;
}
ULONG COM_Seek(	HANDLE  pHead,
				LONG    Position,
				DWORD   Type     )
{
    return(ULONG)-1;
}

/////////////////////////////////////////////////////////////
BOOL COM_PowerUp(HANDLE pHead)
{
//	SetState((PCV_INDEP_INFO)pHead, PWR_ON);

	RETAILMSG(GPS_MSG,(L"GPS: COM_PowerUp\r\n"));
	return FALSE;
}
/////////////////////////////////////////////////////////////////////
BOOL COM_PowerDown(HANDLE      pHead)
{
	RETAILMSG(GPS_MSG,(L"GPS: COM_PowerDown\r\n"));
	return FALSE;
}
/////////////////////////////////////////////////////////////////////
BOOL ProcessExiting(PCV_OPEN_INFO pOpenHead)
{
    PCV_INDEP_INFO  pSerialHead = pOpenHead->pSerialHead;

    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("GPS: +ProcessExiting\r\n")));

    if(!pSerialHead) 
	{
        DEBUGMSG (ZONE_ERROR, (TEXT("GPS: !!ProcessExiting: pSerialHead == NULL!!\r\n")));
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

	COM_PreClose(pOpenHead);
    DEBUGMSG (ZONE_CLOSE|ZONE_FUNCTION, (TEXT("GPS: -ProcessExiting\r\n")));
    return (TRUE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI WaitCommEvent_mdd(
             PCV_OPEN_INFO   pOpenHead,      // @parm Handle to device.
             PULONG          pfdwEventMask,  // @parm Pointer to ULONG to receive CommEvents.fEventMask.
             LPOVERLAPPED    Unused          // @parm Pointer to OVERLAPPED not used.
             )
{
    PCV_INDEP_INFO  pHWIHead	= pOpenHead->pSerialHead;
    DWORD           Ret			= 0;
	HANDLE			hEv[3];


	DEBUGMSG(ZONE_FUNCTION|ZONE_EVENTS,(TEXT("GPS: +COM: WaitCommEvent x%X x%X, Mask x%X\r\n"),
                                        pOpenHead, pHWIHead , pOpenHead->CommEvents.fEventMask));

    RETAILMSG(GPS_MSG,(TEXT("GPS: +COM: WaitCommEvent x%X x%X, Mask x%X\r\n"),pOpenHead, pHWIHead , pOpenHead->CommEvents.fEventMask));//temp!!!

    if(!pHWIHead->OpenCnt ) 
	{
        DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT("GPS: -COM: WaitCommEvent - device not open (x%X, %d) \r\n"),
                                           pHWIHead, (pHWIHead == NULL) ? 0 : pHWIHead->OpenCnt));
        *pfdwEventMask = 0;
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    // We should return immediately if mask is 0
    if( !pOpenHead->CommEvents.fEventMask ) 
	{
        DEBUGMSG (ZONE_ERROR|ZONE_EVENTS, (TEXT("GPS: -COM: WaitCommEvent - Mask already clear\r\n")));
        *pfdwEventMask = 0;
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

	hEv[0] = pHWIHead->hSerialEventAct;
	hEv[1] = pHWIHead->hSerialEventOff;
	hEv[2] = pOpenHead->CommEvents.hCommEvent;

	// Wait for an event from COV, or from SetCommMask
	Ret = WaitForMultipleObjects(3, hEv, 0, INFINITE);

	RETAILMSG(GPS_MSG, (TEXT("GPS: Wait Ret %d\r\n"),Ret ));
	// Check and see if device was closed while we were waiting
    if( !pHWIHead->OpenCnt )
	{
        // Device was closed.  Get out of here.
        DEBUGMSG (ZONE_EVENTS|ZONE_ERROR,(TEXT("GPS: -WaitCommEvent - device was closed\r\n")));
        *pfdwEventMask = 0;
        SetLastError (ERROR_INVALID_HANDLE);
        return 0;
    } 

	if(WAIT_OBJECT_0 == Ret)  
	{
        // We either got an event or a SetCommMask 0.
        DEBUGMSG (ZONE_EVENTS,(TEXT("GPS: -WaitCommEvent internal OK\r\n")));
		RETAILMSG(GPS_MSG, (TEXT("GPS: -WaitCommEvent internal OK\r\n")));
        return 1;
    }

	return 0;
}

//////////////////////////////////////////////////////////////////////
BOOL	WaitOnMaskIO(	PCV_OPEN_INFO pOpenHead,
						DWORD dwCode, PBYTE pBufIn,
						DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
						PDWORD pdwActualOut)
						//if(IOCTL_SERIAL_WAIT_ON_MASK == dwCode)
{
	DWORD			RetVal			= 0;
	DWORD			dwFlagEventMask = 0;
    PCV_INDEP_INFO  pHWIHead		= pOpenHead->pSerialHead;

    InterlockedIncrement(&pOpenHead->WaitUsers);
    // Abort should only affect us once we start waiting.  Ignore any old aborts
	EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));

	pOpenHead->CommEvents.fAbort = 0;
	ResetEvent(pOpenHead->CommEvents.hCommEvent);

	LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

	while(pHWIHead->pGpsContext->fOn || pHWIHead->pGpsContext->fEnableExternalGPS)
	{
		if(IsReady(pHWIHead))
		{
			if((HANDLE)-1 == pOpenHead->hPort)
			{
				SetLastError(ERROR_INVALID_HANDLE);
				break;
			}
			RetVal = DeviceIoControl(pOpenHead->hPort, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut, 0);
			RETAILMSG(GPS_MSG, (TEXT("GPS:  COM_IOControl - WaitCommEvent ret %d\r\n"), RetVal));
			
			if(	pOpenHead->CommEvents.fAbort		||
				IsDevReady(pHWIHead)				)
			{
				break;
			}
			
			EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));

			if( (0 == pOpenHead->CommEvents.fEventMask)								||
				(RetVal && (*((DWORD *)pBufOut) & pOpenHead->CommEvents.fEventMask)	) 
																)
			{
				LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));
				break;// return from COV
			}
			LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

		}
		else
		{
			if(!WaitCommEvent_mdd(pOpenHead, &dwFlagEventMask, NULL))//else - try again
			{
				*((DWORD *)pBufOut)	= 0;
				*pdwActualOut		= sizeof(DWORD);
				// Device may have been closed or removed while we were waiting
				DEBUGMSG (ZONE_IOCTL|ZONE_ERROR, (TEXT("GPS: COM_IOControl - Error in WaitCommEvent\r\n")));
				RETAILMSG(GPS_MSG, (TEXT("GPS:  COM_IOControl - Error in WaitCommEvent\r\n")));
				
				if(pOpenHead->CommEvents.fAbort)//hier CommMask = 0
				{
					RetVal = 1;
				}
				break;
			}
		}
	}
	if(!pHWIHead->pGpsContext->fOn && !pHWIHead->pGpsContext->fEnableExternalGPS)
	{
		SetLastError(ERROR_NOT_READY);
	}
    InterlockedDecrement(&pOpenHead->WaitUsers);
	return RetVal;
}
// ****************************************************************
BOOL GpsIOControl(	PCV_OPEN_INFO pOpenHead,
					DWORD dwCode, PBYTE pBufIn,
					DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
					PDWORD pdwActualOut)
{
	DWORD RetVal = 1;
    PCV_INDEP_INFO		pHWIHead	= pOpenHead->pSerialHead;
    GPS_INIT_CONTEXT*	pCtx		= pHWIHead->pGpsContext;

	switch(dwCode) 
	{	
		case IOCTL_POWER_CAPABILITIES: 
		{
			RETAILMSG(GPS_MSG, (L"GPS: Received IOCTL_POWER_CAPABILITIES\r\n"));
			if(pBufOut && dwLenOut >= sizeof (POWER_CAPABILITIES) && pdwActualOut) 
			{
                __try 
				{
                    PPOWER_CAPABILITIES PowerCaps;
                    PowerCaps = (PPOWER_CAPABILITIES)pBufOut;
     
                    // Only supports D0 (permanently on) and D4(off.         
                    memset(PowerCaps, 0, sizeof(*PowerCaps));
                    PowerCaps->DeviceDx = (UCHAR)pCtx->powerMask;
                    *pdwActualOut = sizeof(*PowerCaps);
                    
				}
                __except(EXCEPTION_EXECUTE_HANDLER) 
				{
					RETAILMSG(1, (L"GPS:exception in ioctl\r\n"));
				}
			}
			else
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = 0;
			}
		}
		break;
    // determines whether changing power state is feasible
		case IOCTL_POWER_QUERY: 
		{
			RETAILMSG(GPS_MSG,(L"GPS: Received IOCTL_POWER_QUERY\r\n"));
			if(pBufOut && dwLenOut >= sizeof(CEDEVICE_POWER_STATE)) 
			{
            // Return a good status on any valid query, since we are 
            // always ready to change power states (if asked for state 
            // we don't support, we move to next highest, eg D3->D4).
				__try 
				{
					CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pBufOut;

					if(VALID_DX(ReqDx)) 
					{
						// This is a valid Dx state so return a good status.
						RetVal = TRUE;
					}

					RETAILMSG(GPS_MSG, (L"GPS: IOCTL_POWER_QUERY %d\r\n"));
				}
				__except(EXCEPTION_EXECUTE_HANDLER) 
                {
					RETAILMSG(1, (L" GPS:Exception in ioctl\r\n"));
                }
			}
			else
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = 0;
			}
		}
		break;
		case IOCTL_POWER_SET: 
		{
			DWORD fUser = 0, act = 0;
			if (pHWIHead->pGpsContext->fEnableExternalGPS)
			{
				SetLastError (ERROR_ACCESS_DENIED);
				RetVal = 0;
			}
			else if(pBufOut && dwLenOut >= sizeof(CEDEVICE_POWER_STATE)&& pdwActualOut) 
			{
				__try 
				{
					CEDEVICE_POWER_STATE ReqDx = *(PCEDEVICE_POWER_STATE)pBufOut;
					pCtx->powerState = ReqDx;
					RETAILMSG(GPS_MSG,(L"GPS: Received IOCTL_POWER_SET %d\r\n", pCtx->powerState));
					*(PCEDEVICE_POWER_STATE)pBufOut = pCtx->powerState;
					*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);

					if(pBufIn && dwLenIn >= sizeof(DWORD))
					{
						fUser	= 1;
					}
					switch(ReqDx)
					{
						case D0:
						case D1:
						case D2:   
						{
							if(fUser)
								act = DEV_ON;
							else
								act = PWR_ON;
						}
						break;
						case D3:
						case D4:
						{	
							if(fUser)
								act = DEV_OFF;
							else
							{
								ResetEvent(pHWIHead->hSerialEventAct);
								act = PWR_OFF;
							}
						}
						break;
						default:
							RetVal = 0;
						break;
					}
					if(RetVal)
						RetVal = SetState(pHWIHead, act);
				}
				__except(EXCEPTION_EXECUTE_HANDLER) 
				{
					RETAILMSG(1, (L"GPS: Exception in ioctl\r\n"));
					RetVal = 0;
				}
			}
			else
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = 0;
			}
		}
		break;
		// gets the current device power state
		case IOCTL_POWER_GET: 
		{
			RETAILMSG(GPS_MSG, (L"GPS: Received IOCTL_POWER_GET\r\n"));
			if (pHWIHead->pGpsContext->fEnableExternalGPS)
			{
				SetLastError (ERROR_ACCESS_DENIED);
				RetVal = 0;
			}
			else if(pBufOut && dwLenOut >= sizeof(CEDEVICE_POWER_STATE)) 
			{
				if(pHWIHead->pGpsContext->fOn)
					*(PCEDEVICE_POWER_STATE)pBufOut = pCtx->powerState;
				else
					*(PCEDEVICE_POWER_STATE)pBufOut = D4;
 				RETAILMSG(GPS_MSG, (L"GPS:IOCTL_POWER_GET to D%u \r\n",pCtx->powerState));
			}     
			else
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = 0;
			}
		}     
		break;
		case IOCTL_REGISTER_POWER_RELATIONSHIP:
		case IOCTL_CONTEXT_RESTORE:
			break;//temp!!!! do something
	}
	return RetVal;
}
// ****************************************************************
BOOL NotReadyIOControl(	PCV_OPEN_INFO pOpenHead,
						DWORD dwCode, PBYTE pBufIn,
						DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
						PDWORD pdwActualOut)
{
	DWORD RetVal = 1;
    PCV_INDEP_INFO		pHWIHead	= pOpenHead->pSerialHead;

	switch(dwCode)
	{
		case IOCTL_SERIAL_SET_BREAK_ON :
		case IOCTL_SERIAL_SET_BREAK_OFF :
			break;
		case IOCTL_SERIAL_SET_DTR :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_DTR\r\n")));
	        if( pHWIHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE ) 
			{
	            SetLastError (ERROR_INVALID_PARAMETER);
		        RetVal = FALSE;
			}
			else
			{
				pHWIHead->SetResetDTR = IOCTL_SERIAL_SET_DTR;
				pHWIHead->fOpened = 1;
			}
        }
        break;
		break;
	    case IOCTL_SERIAL_CLR_DTR :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_CLR_DTR\r\n")));
			if( pHWIHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
			}
			else
			{
				pHWIHead->SetResetDTR = IOCTL_SERIAL_CLR_DTR;
				pHWIHead->fOpened = 1;
			}
		}
        break;
		case IOCTL_SERIAL_SET_RTS :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_RTS\r\n")));
	        if( pHWIHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
			}
			else
			{
				pHWIHead->SetResetDTR = IOCTL_SERIAL_SET_RTS;
				pHWIHead->fOpened = 1;
			}
		}
		break;
		case IOCTL_SERIAL_CLR_RTS :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_CLR_RTS\r\n")));
			if( pHWIHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
			}
			else
			{
				pHWIHead->SetResetDTR = IOCTL_SERIAL_CLR_RTS;
				pHWIHead->fOpened = 1;
			}
		}
		break;
		case IOCTL_SERIAL_SET_XOFF :
        {
			DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT("GPS: IOCTL_SERIAL_SET_XOFF\r\n")));
			if( pHWIHead->XFlow ) 
			{
				pHWIHead->StopXmit = 1;
				pHWIHead->SentXoff = 1;
			}
		}
        break;
		case IOCTL_SERIAL_SET_XON :
		{
			DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT("GPS: IOCTL_SERIAL_SET_XON\r\n")));
			if( pHWIHead->XFlow ) 
			{
				pHWIHead->StopXmit = 0;
				pHWIHead->SentXoff = 0;
			}
		}
        break;
		case IOCTL_SERIAL_GET_WAIT_MASK :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_GET_WAIT_MASK\r\n")));
			if( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				break;
			}
			// Set The Wait Mask
			*(DWORD *)pBufOut = pOpenHead->CommEvents.fEventMask;
			// Return the size
			*pdwActualOut = sizeof(DWORD);
		}
		break;
		case IOCTL_SERIAL_SET_WAIT_MASK:
		{
			if( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				RetVal = 0;
				break;
			}
			EnterCriticalSection(&(pOpenHead->CommEvents.EventCS));
			
			pOpenHead->CommEvents.fEventMask	= *(DWORD*)pBufIn;
			pOpenHead->CommEvents.fAbort		= 1;			
			SetEvent(pOpenHead->CommEvents.hCommEvent);
            
			LeaveCriticalSection(&(pOpenHead->CommEvents.EventCS));

			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_WAIT_MASK 0x%X on internal wait\r\n"),pOpenHead->CommEvents.fEventMask));
		}
		break;
//		    case IOCTL_SERIAL_WAIT_ON_MASK : already processed
		case IOCTL_SERIAL_GET_COMMSTATUS ://???
		{
			SERIAL_DEV_STATUS SerialDevStat;

            DEBUGMSG (ZONE_IOCTL, (TEXT(" IOCTL_SERIAL_GET_COMMSTATUS\r\n")));
            if( (dwLenOut < sizeof(SERIAL_DEV_STATUS)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
			{
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = FALSE;
                DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
                break;
            }
            // Clear the ComStat structure & get PDD related status
            memset ((char *) &(SerialDevStat.ComStat), 0, sizeof(COMSTAT));
            
            // Set The Error Mask
            SerialDevStat.Errors = 0;//InterlockedExchange((LPLONG)&pHWIHead->dwMddErrors,0);
            //SerialDevStat.Errors |= pFuncTbl->HWGetStatus(pHWHead, &(SerialDevStat.ComStat));

            // PDD set fCtsHold, fDsrHold, fRLSDHold, and fTXim.  The MDD then
            // needs to set fXoffHold, fXoffSent, cbInQue, and cbOutQue.
            SerialDevStat.ComStat.cbInQue	= 0;//RxBytesAvail(pHWIHead);
            SerialDevStat.ComStat.cbOutQue	= 0;//pHWIHead->TxBytesPending;
            SerialDevStat.ComStat.fXoffHold = pHWIHead->StopXmit;
            SerialDevStat.ComStat.fXoffSent = pHWIHead->SentXoff;

            *((PSERIAL_DEV_STATUS)pBufOut) = SerialDevStat;
            *pdwActualOut = sizeof(SERIAL_DEV_STATUS);
		}
		break;
		case IOCTL_SERIAL_GET_MODEMSTATUS : //???
		{
        DWORD dwModemStatus = 0 ;
        DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_GET_MODEMSTATUS\r\n")));
        if( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
		{
            SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
            break;
        }

        // Set the Modem Status dword
        //pFuncTbl->HWGetModemStatus(pHWHead, &dwModemStatus);
        // Return the size
        *pdwActualOut = sizeof(DWORD);
        *((PULONG)pBufOut) = dwModemStatus;
		}
        break;
		case IOCTL_SERIAL_GET_PROPERTIES ://???
		{
			COMMPROP commProp;
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_GET_PROPERTIES\r\n")));
			if( (dwLenOut < sizeof(COMMPROP)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT(" Invalid parameter\r\n")));
				break;
			}

			// Clear the ComMProp structure
			memset ((char *) &commProp, 0, sizeof(COMMPROP));

//        pFuncTbl->HWGetCommProperties(pHWHead, &commProp);

			*((LPCOMMPROP)pBufOut)	= commProp;
			*pdwActualOut			= sizeof(COMMPROP);
		}
        break;
		case IOCTL_SERIAL_SET_TIMEOUTS :
		{
			if( (dwLenIn < sizeof(COMMTIMEOUTS)) || (NULL == pBufIn) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				break;
			}
			memcpy(&pHWIHead->CommTimeouts, pBufIn, sizeof(COMMTIMEOUTS)) ;

  //      pFuncTbl->HWSetCommTimeouts(pHWHead,&pHWIHead->CommTimeouts);
		}
		break;
		case IOCTL_SERIAL_GET_TIMEOUTS :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_GET_TIMEOUTS\r\n")));
			if( (dwLenOut < sizeof(COMMTIMEOUTS)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				break;
			}

	        memcpy((LPCOMMTIMEOUTS)pBufOut, &(pHWIHead->CommTimeouts), sizeof(COMMTIMEOUTS));
			// Return the size
			*pdwActualOut = sizeof(COMMTIMEOUTS);
		}
		break;
		case IOCTL_SERIAL_PURGE ://???
		{
			if( (dwLenIn < sizeof(DWORD)) || (NULL == pBufIn) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				break;
			}
			DWORD dwFlags = *((PDWORD) pBufIn);

			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_PURGE 0x%X\r\n"),dwFlags));

			if( dwFlags & PURGE_RXCLEAR ) 
			{
 				// Clear any flow control state
				if( pHWIHead->DCB.fInX && pHWIHead->SentXoff ) 
				{
					pHWIHead->SentXoff = 0;
					if( !pHWIHead->DCB.fTXContinueOnXoff ) 
					{
						pHWIHead->StopXmit = 0;
					}
	 //               pFuncTbl->HWXmitComChar(pHWIHead->pHWHead,pHWIHead->DCB.XonChar);
				}
				if( pHWIHead->RtsFlow && (pHWIHead->DCB.fRtsControl == RTS_CONTROL_HANDSHAKE) ) 
				{
					DEBUGMSG (ZONE_IOCTL|ZONE_USR_READ|ZONE_FLOW, (TEXT("GPS: RTS_CONTROL_HANDSHAKE Setting RTS\r\n")));
					pHWIHead->RtsFlow = 0;
	 //               pFuncTbl->HWSetRTS(pHWIHead->pHWHead);
				}
				if( pHWIHead->DtrFlow && (pHWIHead->DCB.fDtrControl == DTR_CONTROL_HANDSHAKE) ) 
				{
					DEBUGMSG (ZONE_IOCTL|ZONE_USR_READ|ZONE_FLOW, (TEXT("GPS: DTR_CONTROL_HANDSHAKE Setting DTR\r\n")));
					pHWIHead->DtrFlow = 0;
	//                pFuncTbl->HWSetDTR(pHWHead);
				}
			}
			// Now, free up any threads blocked in MDD. Reads and writes are in loops, so they also need a flag to tell them to abort.
			if( dwFlags & PURGE_RXABORT ) 
			{
				pHWIHead->fAbortRead = 1;
				PulseEvent(pHWIHead->hReadEvent);
			}
			if( dwFlags & PURGE_TXABORT ) 
			{
				pHWIHead->fAbortTransmit      = 1;
				// COV_Write() clears event upon entry, so we can use SetEvent
				SetEvent(pHWIHead->hTransmitEvent);
			}
		}
		break;
		case IOCTL_SERIAL_SET_QUEUE_SIZE :
			SetLastError(ERROR_NOT_SUPPORTED);
			RetVal = FALSE;
        break;
		case IOCTL_SERIAL_IMMEDIATE_CHAR ://???
		break;
		case IOCTL_SERIAL_GET_DCB :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_GET_DCB\r\n")));
			if( (dwLenOut < sizeof(DCB)) || (NULL == pBufOut) || (NULL == pdwActualOut) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				break;
			}
			memcpy((char *)pBufOut, (char *)&(pHWIHead->DCB), sizeof(DCB));
			*pdwActualOut = sizeof(DCB);
		}
		break;
		case IOCTL_SERIAL_SET_DCB : //add default to timeouts and DCB!!!!!
		{
			if( (dwLenIn < sizeof(DCB)) || (NULL == pBufIn) ) 
			{
				SetLastError (ERROR_INVALID_PARAMETER);
				RetVal = FALSE;
				DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid parameter\r\n")));
				break;
			}
            DCB localDCB;
            DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_DCB\r\n")));
            localDCB = *((DCB *)pBufIn);
            if( !ApplyDCB (pHWIHead, &localDCB, TRUE) ) 
			{
                SetLastError (ERROR_INVALID_PARAMETER);
                RetVal = FALSE;
                DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: ApplyDCB failed\r\n")));
                break;
            }
			memcpy(&pHWIHead->DCB, &localDCB, sizeof(DCB));
			pHWIHead->fOpened = 1;
		}
        break;
		case IOCTL_SERIAL_ENABLE_IR ://???
		case IOCTL_SERIAL_DISABLE_IR :
		break;
		default :
		{
			SetLastError (ERROR_INVALID_PARAMETER);
            RetVal = FALSE;
            DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: Invalid ioctl 0x%X\r\n"), dwCode));
        }
        break;
	}
	return RetVal;
}
BOOL OkIOControl(	PCV_OPEN_INFO pOpenHead,
					DWORD dwCode, PBYTE pBufIn,
					DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
					PDWORD pdwActualOut)
{
    PCV_INDEP_INFO		pHWIHead	= pOpenHead->pSerialHead;
   
	switch( dwCode ) 
	{
		case IOCTL_SERIAL_SET_DTR :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_DTR\r\n")));
    
			if( pHWIHead->DCB.fDtrControl != DTR_CONTROL_HANDSHAKE )
				pHWIHead->SetResetDTR = IOCTL_SERIAL_SET_DTR;
		}
		break;
		case IOCTL_SERIAL_CLR_DTR :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_CLR_DTR\r\n")));

			if( pHWIHead->DCB.fDtrControl != DTR_CONTROL_HANDSHAKE ) 
				pHWIHead->SetResetDTR = IOCTL_SERIAL_CLR_DTR;
		}
		break;

		case IOCTL_SERIAL_SET_RTS :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_RTS\r\n")));
			if( pHWIHead->DCB.fRtsControl != RTS_CONTROL_HANDSHAKE ) 
				pHWIHead->SetResetRTS = IOCTL_SERIAL_SET_RTS;
		}
		break;
	    case IOCTL_SERIAL_CLR_RTS :
		{
			DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_CLR_RTS\r\n")));
	        if( pHWIHead->DCB.fRtsControl != RTS_CONTROL_HANDSHAKE )
				pHWIHead->SetResetRTS = IOCTL_SERIAL_CLR_RTS;
		}
		break;
		case IOCTL_SERIAL_SET_XOFF :
        {
			DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT("GPS: IOCTL_SERIAL_SET_XOFF\r\n")));
			if( pHWIHead->XFlow ) 
			{
				pHWIHead->StopXmit = 1;
				pHWIHead->SentXoff = 1;
			}
		}
        break;
		case IOCTL_SERIAL_SET_XON :
		{
			DEBUGMSG (ZONE_IOCTL|ZONE_FLOW, (TEXT("GPS: IOCTL_SERIAL_SET_XON\r\n")));
			if( pHWIHead->XFlow ) 
			{
				pHWIHead->StopXmit = 0;
				pHWIHead->SentXoff = 0;
			}
		}
        break;
		case IOCTL_SERIAL_SET_WAIT_MASK :
		{
			pOpenHead->CommEvents.fEventMask = *(DWORD *)pBufIn;
            DEBUGMSG (ZONE_IOCTL, (TEXT("GPS: IOCTL_SERIAL_SET_WAIT_MASK 0x%X\r\n"),pOpenHead->CommEvents.fEventMask));
		}
		break;
	    case IOCTL_SERIAL_SET_TIMEOUTS :
		{
			memcpy(&pHWIHead->CommTimeouts, pBufIn, sizeof(COMMTIMEOUTS)) ;
			DEBUGMSG (ZONE_IOCTL,
					  (TEXT("GPS: IOCTL_SERIAL_SET_COMMTIMEOUTS (%d,%d,%d,%d,%d)\r\n"),
					   pHWIHead->CommTimeouts.ReadIntervalTimeout,
					   pHWIHead->CommTimeouts.ReadTotalTimeoutMultiplier,
					   pHWIHead->CommTimeouts.ReadTotalTimeoutConstant,
					   pHWIHead->CommTimeouts.WriteTotalTimeoutMultiplier,
					   pHWIHead->CommTimeouts.WriteTotalTimeoutConstant));

		}
		break;

	    case IOCTL_SERIAL_SET_DCB :
			memcpy(&pHWIHead->DCB, pBufIn, sizeof(DCB));
        break;
    }

	return 1;
}
// ****************************************************************
BOOL COM_IOControl(	PCV_OPEN_INFO pOpenHead,
					DWORD dwCode, PBYTE pBufIn,
					DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut,
					PDWORD pdwActualOut)
{
    BOOL            RetVal           = TRUE;        // Initialize to success
    PCV_INDEP_INFO  pHWIHead; //= pOpenHead->pSerialHead;

	if(!pOpenHead || !pOpenHead->pSerialHead) 
	{
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }
	pHWIHead = pOpenHead->pSerialHead;

	if(	IOCTL_POWER_CAPABILITIES		== dwCode	||
		IOCTL_POWER_QUERY				== dwCode	||
		IOCTL_POWER_SET					== dwCode	||
		IOCTL_POWER_GET					== dwCode	||
		IOCTL_REGISTER_POWER_RELATIONSHIP	== dwCode	||
		IOCTL_CONTEXT_RESTORE			== dwCode		)
	{
		return GpsIOControl(pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	}

    if(!pHWIHead->OpenCnt) 
	{
        DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT(" COM_IOControl - device was closed\r\n")));
        SetLastError (ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    if(dwCode == IOCTL_PSL_NOTIFY) 
	{
        PDEVICE_PSL_NOTIFY pPslPacket = (PDEVICE_PSL_NOTIFY)pBufIn;
        if( (pPslPacket->dwSize == sizeof(DEVICE_PSL_NOTIFY)) && (pPslPacket->dwFlags == DLL_PROCESS_EXITING) ) 
		{
            DEBUGMSG(ZONE_IOCTL, (TEXT("Process is exiting.\r\n")));
            ProcessExiting(pOpenHead);
        }
    }
	if(!pHWIHead->pGpsContext->fOn && !pHWIHead->pGpsContext->fEnableExternalGPS)
	{
        DEBUGMSG (ZONE_IOCTL|ZONE_ERROR,(TEXT(" COM_IOControl - device was powerd off\r\n")));
        SetLastError(ERROR_NOT_READY);
        return(FALSE);
	}
    DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION,(TEXT("GPS: +COM_IOControl(0x%X, 0x%X, 0x%X, %d, 0x%X, %d, 0x%X)\r\n"),
               pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut));
    RETAILMSG(0,(TEXT("GPS: +COM_IOControl(0x%X, %d, 0x%X, 0x%X, 0x%X, %d, 0x%X)\r\n"), pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut));

   // Make sure the caller has access permissions
    // NOTE : Pay attention here.  I hate to make this check repeatedly
    // below, so I'll optimize it here.  But as you add new ioctl's be
    // sure to account for them in this if check.
    if( !( (dwCode == IOCTL_SERIAL_GET_WAIT_MASK) ||
            (dwCode == IOCTL_SERIAL_SET_WAIT_MASK) ||
            (dwCode == IOCTL_SERIAL_WAIT_ON_MASK) ||
            (dwCode == IOCTL_SERIAL_GET_MODEMSTATUS) || //not ready status 0
            (dwCode == IOCTL_SERIAL_GET_PROPERTIES) ||	//from CSerialPDD::SetDefaultConfiguration() ???
            (dwCode == IOCTL_SERIAL_GET_TIMEOUTS) ) )//||
 //           (dwCode == IOCTL_POWER_CAPABILITIES) ||//already 
 //           (dwCode == IOCTL_POWER_QUERY) ||
 //           (dwCode == IOCTL_POWER_SET) ||
 //           (dwCode == IOCTL_CONTEXT_RESTORE) ) ) 
	{
        // If not one of the above operations, then read or write access permissions are required.
        if( !(pOpenHead->AccessCode & (GENERIC_READ | GENERIC_WRITE) ) ) 
		{
			DEBUGMSG(ZONE_IOCTL|ZONE_ERROR, (TEXT("GPS:COM_Ioctl: Ioctl %x access permission failure x%X\n\r"), dwCode, pOpenHead->AccessCode));
            SetLastError (ERROR_INVALID_ACCESS);
            return(FALSE);
        }
    }

	if(IOCTL_SERIAL_WAIT_ON_MASK == dwCode)//special loop
	{
		if( (dwLenOut < sizeof(DWORD)) || (NULL == pBufOut) )
		{
			RETAILMSG(GPS_MSG, (TEXT("GPS:  COM_IOControl - WaitCommEvent closed\r\n")));
			SetLastError(ERROR_INVALID_PARAMETER);
			return 0;
		}
		return WaitOnMaskIO(pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	}	

	if(!IsReady(pHWIHead))
	{
		DEBUGMSG (ZONE_OPEN|ZONE_ERROR,(TEXT("GPS: COM_IOControl: device is not ready\r\n")));
  		RETAILMSG(GPS_MSG,(TEXT("GPS: COM_IOControl: device is not ready\r\n")));
        //
		return NotReadyIOControl(pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);
	}

	if((HANDLE)-1 == pOpenHead->hPort)
	{
  		RETAILMSG(GPS_MSG,(TEXT("GPS: COM_IOControl (HANDLE)-1 == pOpenHead->hPort\r\n")));
		SetLastError(ERROR_INVALID_HANDLE);
		return 0;
	}
	RetVal = DeviceIoControl(pOpenHead->hPort, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut, 0);
	if(!RetVal)
	{
	    RETAILMSG(GPS_MSG,(TEXT("GPS: +COM_IOControl failed err %d(0x%X, 0x%X, 0x%X, %d, 0x%X, %d, 0x%X)\r\n"),GetLastError(),pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut,dwLenOut, pdwActualOut));

		ChangeError(pHWIHead);
		return RetVal;
	}
 
	OkIOControl(pOpenHead, dwCode, pBufIn, dwLenIn, pBufOut, dwLenOut, pdwActualOut);

	DEBUGMSG (ZONE_IOCTL|ZONE_FUNCTION|(RetVal == FALSE?ZONE_ERROR:0), (TEXT("GPS: -COM_IOControl %s Ecode=%d (len=%d)\r\n"),
																	   (RetVal == TRUE) ? TEXT("Success") : TEXT("Error"),
																	   GetLastError(), (NULL == pdwActualOut) ? 0 : *pdwActualOut));

    RETAILMSG(0, (TEXT("GPS: -COM_IOControl %s Ecode=%d (len=%d)\r\n"),	(RetVal == TRUE) ? TEXT("Success") : TEXT("Error"),
																		GetLastError(), (NULL == pdwActualOut) ? 0 : *pdwActualOut));
    
	return(RetVal);
}
void ChangeError(PCV_INDEP_INFO pSerialHead)
{
	DWORD err = GetLastError();
	if(	ERROR_DEVICE_REMOVED	== err	||
		ERROR_INVALID_HANDLE	== err	)
	{
		if(!IsDevReady(pSerialHead))
			SetLastError(0);//(ERROR_NOT_READY);
	}
}
BOOL IsDevReady(PCV_INDEP_INFO pSerialHead)
{
	if(WAIT_OBJECT_0 ==  WaitForSingleObject(pSerialHead->hSerialEventAct, 0))
		return 1;
	return 0;
}
BOOL IsReady(PCV_INDEP_INFO pSerialHead)
{
	DWORD ret = 0;

	if((pSerialHead->pGpsContext->fOn && IsDevReady(pSerialHead)) || pSerialHead->pGpsContext->fEnableExternalGPS)
	{
		if(REOPEN_ST == pSerialHead->DevState)
		{
			ComReOpen(pSerialHead);
		}
		if(REOPEN_ST == pSerialHead->DevState)//DevState can be changed already
			SetState(pSerialHead, NORMAL_ON);
	}

	if ((pSerialHead->pGpsContext->fOn && IsDevReady(pSerialHead) && NORMAL_ST == pSerialHead->DevState) || pSerialHead->pGpsContext->fEnableExternalGPS)
	{
		ret = 1;	
	}
	return ret;
}
BOOL	CloseHandles(PCV_INDEP_INFO pSerialHead)
{
	PLIST_ENTRY     pEntry;
	PCV_OPEN_INFO   pOpenHead;

	RETAILMSG(GPS_MSG,(L"GPS: +COM CloseHandles\r\n"));
	EnterCriticalSection(&(pSerialHead->OpenCS));
	if( pSerialHead->OpenCnt ) 
	{
        pEntry = pSerialHead->OpenList.Flink;
    
		while ( pEntry != &pSerialHead->OpenList ) 
		{
            pOpenHead = CONTAINING_RECORD( pEntry, CV_OPEN_INFO, llist);
            pEntry = pEntry->Flink;  // advance to next

			DEBUGMSG (ZONE_INIT | ZONE_CLOSE, (TEXT("GPS: Closing Handle 0x%X\r\n"), pOpenHead ));

            if((HANDLE)-1 != pOpenHead->hPort)
			{
				RETAILMSG(GPS_MSG,(L"GPS: COM close pOpenHead %x phys port %x\r\n", pOpenHead, pOpenHead->hPort));
				CloseHandle(pOpenHead->hPort);
				pOpenHead->hPort = (HANDLE)-1;
			}
		}
		
	}
	LeaveCriticalSection(&(pSerialHead->OpenCS));
	
	RETAILMSG(GPS_MSG,(L"GPS: -COM CloseHandles\r\n"));

	return 1;
}
BOOL CloseAll(PCV_INDEP_INFO pSerialHead)
{
	PLIST_ENTRY     pEntry;
	PCV_OPEN_INFO   pOpenHead;

	EnterCriticalSection(&(pSerialHead->OpenCS));
    pEntry = pSerialHead->OpenList.Flink;
    
	while ( pEntry != &pSerialHead->OpenList ) 
	{
		pOpenHead = CONTAINING_RECORD( pEntry, CV_OPEN_INFO, llist);
		pEntry = pEntry->Flink;  // advance to next

		DEBUGMSG (ZONE_INIT | ZONE_CLOSE, (TEXT(" Deinit - Closing Handle 0x%X\r\n"), pOpenHead ));
		COM_Close(pOpenHead);
		
	}
	LeaveCriticalSection(&(pSerialHead->OpenCS));

	RETAILMSG(GPS_MSG,(L"GPS: -COM CloseAll\r\n"));

	return 1;
}

BOOL ComReOpen(PCV_INDEP_INFO pSerialHead)
{
	DWORD	Mask	= 0;
	HANDLE	hPort		= (HANDLE)-1;
	HANDLE	hTmpPort	= (HANDLE)-1;
	DWORD	ret		= 1; 
	DWORD	fReopen	= 0;
    PLIST_ENTRY     pEntry;
    PCV_OPEN_INFO   pOpenHead;

    DEBUGMSG (ZONE_INIT|ZONE_FUNCTION, (TEXT("GPS: +ComReOpen\r\n")));
    RETAILMSG(GPS_MSG, (TEXT("GPS: +ComReOpen\r\n")));
    
	EnterCriticalSection(&(pSerialHead->StatCritSec));
	EnterCriticalSection(&(pSerialHead->OpenCS));
     RETAILMSG(GPS_MSG, (TEXT("GPS: +ComReOpen++\r\n")));//temp!!!!!!!!!!!
   
	if( pSerialHead->OpenCnt ) 
	{
		pEntry = pSerialHead->OpenList.Flink;
        while ( pEntry != &pSerialHead->OpenList && IsDevReady(pSerialHead)) 
		{
            pOpenHead = CONTAINING_RECORD( pEntry, CV_OPEN_INFO, llist);
            pEntry = pEntry->Flink;  // advance to next

 			if( !(pOpenHead->AccessCode & DEVACCESS_BUSNAMESPACE)					&& 
				!(pOpenHead->ShareMode & (DEVACCESS_PMEXT_MODE | DEVACCESS_PM_MODE)) ) 
			{
				if((HANDLE)-1 == pOpenHead->hPort) 
				{
					fReopen = 1;
					pOpenHead->hPort = CreateFile( pSerialHead->pGpsContext->szPortName, pOpenHead->AccessCode, pOpenHead->ShareMode, 0, OPEN_EXISTING, 0, NULL);

					RETAILMSG(GPS_MSG, (TEXT("GPS: ComReOpen AccessCode %X ShareMode %X HANDLE %X\r\n"), pOpenHead->AccessCode, pOpenHead->ShareMode, pOpenHead->hPort ));

					if((HANDLE)-1 == pOpenHead->hPort) 
					{
						DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("GPS: ReOpen failed err %d\r\n"), GetLastError()));
						RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen failed err %d\r\n"), GetLastError()));
						
						ret = 0;
						break;
					}
					if(pOpenHead->CommEvents.fEventMask)
					{
						RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen SetCommmask\r\n")));
						ret = SetCommMask(pOpenHead->hPort, pOpenHead->CommEvents.fEventMask);
						if(!ret)
						{
							fReopen = 0;
							RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen SetCommmask failed\r\n")));
							break;
						}
					}
					if( pOpenHead == pSerialHead->pAccessOwner ) 
					{	
						hPort = pOpenHead->hPort;
					}
				}
			}   
		}
		
		if((HANDLE)-1 != hPort && fReopen)
		{
			hTmpPort = hPort;
		}
		else if(ret && fReopen && IsDevReady(pSerialHead))//temp port - open to restore params for the port with access == 0
		{
			hTmpPort = CreateFile( pSerialHead->pGpsContext->szPortName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, NULL);
			RETAILMSG(GPS_MSG, (TEXT("GPS: ComReOpen tmp HANDLE %X\r\n"), hTmpPort ));
			if((HANDLE)-1 == hTmpPort) 
			{
				DEBUGMSG (ZONE_OPEN|ZONE_ERROR, (TEXT("GPS: ReOpen failed err %d\r\n"), GetLastError()));
				RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen failed err %d\r\n"), GetLastError()));
						
				ret = 0;
			}
		}
		
		if(pSerialHead->fOpened && ret && fReopen && IsDevReady(pSerialHead))
		{
			RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen ReInit\r\n")));
			
			if(	!SetCommState(hTmpPort, &pSerialHead->DCB)					||
				!SetCommTimeouts(hTmpPort, &pSerialHead->CommTimeouts)		)
			{
				RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen Set params failed err %d, 0x%X\r\n"), GetLastError(), hTmpPort));
				ret = 0;
			}

			if( ret														&&
				pSerialHead->DCB.fDtrControl != DTR_CONTROL_HANDSHAKE	&&
				pSerialHead->SetResetDTR								&&
				IsDevReady(pSerialHead)											)
			{
				ret = DeviceIoControl(hTmpPort, pSerialHead->SetResetDTR, 0, 0, 0, 0, 0, 0);
			}
			if(	ret														&&
				pSerialHead->DCB.fRtsControl != RTS_CONTROL_HANDSHAKE	&&
				pSerialHead->SetResetRTS								&&
				IsDevReady(pSerialHead)											)
			{
				ret = DeviceIoControl(hTmpPort, pSerialHead->SetResetRTS, 0, 0, 0, 0, 0, 0);
			}
		}
		else if(ret && fReopen && IsDevReady(pSerialHead))
		{
			pSerialHead->SetResetDTR = 0;
			pSerialHead->SetResetRTS = 0;
			
			if(	!GetCommState(hTmpPort, &pSerialHead->DCB)					||
				!SetCommTimeouts(hTmpPort, &pSerialHead->CommTimeouts)		)
			{
				RETAILMSG(GPS_MSG, (TEXT("GPS: ReOpen Get params failed err %d, 0x%X\r\n"), GetLastError(), hTmpPort));
				ret = 0;
			}
		}
		if((HANDLE)-1 == hPort && (HANDLE)-1 != hTmpPort)
		{
			RETAILMSG(GPS_MSG, (TEXT("GPS: Close phys port\r\n")));
			CloseHandle(hTmpPort);
		}
	}

    if(!ret || !IsDevReady(pSerialHead))
	{
		CloseHandles(pSerialHead);
		ChangeError(pSerialHead);
	}
	LeaveCriticalSection(&(pSerialHead->OpenCS));
	LeaveCriticalSection(&(pSerialHead->StatCritSec));

	RETAILMSG(GPS_MSG, (TEXT("GPS: -ComReOpen\r\n")));
	return 1;
}
