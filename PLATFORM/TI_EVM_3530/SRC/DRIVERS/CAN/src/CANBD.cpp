// ================================================================================================
//
// Giant Steps Ltd.
//
// Microchip MCP2515 CAN Controller Stream Interface Driver for Micronet MXL Board
// 
// 
// ================================================================================================

#include <Windows.h>
#include <ceddk.h>
#include <ceddkex.h>
//#include <initguid.h>

#include "MISC.h"
#include <bsp.h>
//#include <initguid.h>
#include <gpio.h>
//#include <SPI.h>
#include <pm.h>
#include "MCP2515.h"
#include "Debug.h"
#include <args.h>


// Definitions and Globals ========================================================================

#ifdef DEBUG

DBGPARAM dpCurSettings =
{
	TEXT ("CANBD"),
	{
		TEXT ( "Init" ),
		TEXT ( "DeInit" ),
		TEXT ( "Power" ),
		TEXT ( "SPI" ),
		TEXT ( "SPIE" ),
		TEXT ( "MCP2515" ),
		TEXT ( "CAN" ),
		TEXT ( "Steps" ),
		TEXT ( "" ),
		TEXT ( "" ),
		TEXT ( "" ),
		TEXT ( "" ),
		TEXT ( "" ),
		TEXT ( "" ),
		TEXT ( "Warning" ),
		TEXT ( "Error" )
	},

//	ZONEMASK_INIT		| 
//	ZONEMASK_DEINIT		|
//	ZONEMASK_SPI		| 
//	ZONEMASK_SPIE		| 
//	ZONEMASK_MCP2515	| 
//	ZONEMASK_CAN		|
	ZONEMASK_ERROR 
};

#endif	// DEBUG

//{ Access synchronization
#define DESTROYED	0
#define CLOSED		1
#define OPENED		2
#define DEAD		-1


//canOpenStatus     g_CAN_err = CANOPEN_OK;

static CRITICAL_SECTION	g_Access;
static int				g_Status = DESTROYED;
//}


// Open contexts
CAN_INIT_CONTEXT    *g_pInitCtx = NULL;
HANDLE hAbortEvent = NULL;
HANDLE hQueryAccess = NULL;

// Interrupt Service Thread
HANDLE	g_hIntrServThread		= NULL;

volatile HANDLE	g_EchoHandle = NULL;

BOOL WINAPI DllEntry ( HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved );

//extern int InPowerHandler;

static void AbortAllPendingIO()
{
	CANBSetOpMode(CAN_OPMODE_CONFIG);
	if( hAbortEvent == NULL) return;
	SetEvent(hAbortEvent);
	CloseHandle(hAbortEvent);
	hAbortEvent  = NULL;
}

void EnterDeadState()
{
	g_Status = DEAD;
	AbortAllPendingIO();
}

// Stream Interface Functions =====================================================================

// Function name	: CAN_Init
// Description	    : Driver Initialization
// Return type		: DWORD CAN_Init 
// Argument         : DWORD dwContext

//#include <SystemInfo.h>

extern "C" DWORD CAN_Init(DWORD dwContext)
{
	CAN_INIT_CONTEXT    *pCxt =0;
	DWORD			    dwRetVal		= 0;
	CANREGSETTINGS	    tCANREGSETTINGS	= {0};
	manufacture_block   mb = {0};
	//int i;
    card_ver			 can_cv;
	UINT32               in = OAL_ARGS_QUERY_CAN_CFG;


	RETAILMSG(CAN_DRV_DBG, (L"+CAN_Init.\r\n"));

	if(!KernelIoControl(IOCTL_HAL_QUERY_CARD_CFG, &in, sizeof(in), &can_cv, sizeof(can_cv), 0))
    {
     RETAILMSG(1, (L"CAN_Init: IOCTL_HAL_QUERY_CARD_CFG calling failure\r\n"));
	 return dwRetVal;
	}

	if( can_cv.ver == (UINT32)~0 )
    {
     RETAILMSG(1, (L"CAN_Init: CAN card not connected\r\n"));
	 return dwRetVal;
	}


	/*
	pArgs->canbus_board.card_addr	= 3;
	pArgs->canbus_board.ver		= ((r & 0x7F) >> 5) + 1;
	pArgs->canbus_board.rev		= (r & 0x1F) >> 3;
	pArgs->canbus_board.config	= (r & 0x07) + 'A';
	*/

   RETAILMSG(CAN_DRV_DBG, (L"CAN_Init: Can bus %d.%d%c\r\n", can_cv.ver, can_cv.rev, (UINT8)can_cv.config )); 

	// allocate CAN context object
	if( (pCxt = (CAN_INIT_CONTEXT *)LocalAlloc(LPTR, sizeof(CAN_INIT_CONTEXT))) == NULL )
	{
		RETAILMSG(1, (L"CAN_Init: cannot allocate CAN context object\r\n"));
		return 0;
	}

    g_pInitCtx = pCxt;

	memset(pCxt, 0, sizeof(CAN_INIT_CONTEXT));

	 // Set cookie
    pCxt->cookie = CAN_DEVICE_COOKIE;

	pCxt->hSPI = NULL;
	pCxt->hGpio = NULL;
	//pCxt->csSPIMCP2515 = NULL;
	pCxt->hIntrServThread = NULL;



	__try
	{
		DEBUGMSG(ZONE_INIT | ZONE_STEPS, (L"CAN: +CAN_Init, Context=0x%08X\r\n", dwContext));

		InitializeCriticalSection(&g_Access);

		if(!RegGetParameters(&tCANREGSETTINGS))
			__leave;
	
		if(!SetTimeOuts(&tCANREGSETTINGS))
			__leave;
	
		if(!ValidateCNFParameters(&tCANREGSETTINGS))
			__leave;

		if(!ValidateMasksAndFilters(&tCANREGSETTINGS))
			__leave;

		if(!ValidateRXMParameters(&tCANREGSETTINGS))
			__leave;

		// Open GPIO driver
		pCxt->hGpio = GPIOOpen();
		if (pCxt->hGpio == INVALID_HANDLE_VALUE) 
		{
		  RETAILMSG(1, (L"CAN_Init: GPIOOpen FAILED!!! err#0x%04X\r\n", GetLastError())); 
		  __leave;
		}
	       
		GPIOSetMode(pCxt->hGpio , nCAN_RESET, GPIO_DIR_OUTPUT);
		GPIOSetMode(pCxt->hGpio , CAN_ON, GPIO_DIR_OUTPUT);
		//GPIOSetMode(pCxt->hGpio , CAN_TR_ON, GPIO_DIR_OUTPUT);
		pCxt->irqNum = nCAN_IRQ;
		GPIOSetMode(pCxt->hGpio , nCAN_IRQ, GPIO_DIR_INPUT|GPIO_INT_HIGH_LOW );

		//RETAILMSG(CAN_DRV_DBG, (L"CAN_Init: not calling MCP2515Init\r\n"));
		
		if(!MCP2515Init(pCxt, &tCANREGSETTINGS))
			__leave;
		

		hQueryAccess = CreateEvent(NULL, FALSE, FALSE, NULL);
		if( hQueryAccess == NULL )
		{
			RETAILMSG(1, (L"CAN: CAN_Init, CreateEvent FAILED!!! err#0x%04X\r\n", GetLastError()));
			__leave;
		}

        if ( FALSE == CANBCreateEvents(pCxt) )
		{
			RETAILMSG(1, (L"CAN: CAN_Init, CANBCreateEvents FAILED!!! err#0x%04X\r\n", GetLastError()));
			__leave;
		}


		if(!GPIOInterruptInitialize(pCxt->hGpio, nCAN_IRQ, pCxt->hIntrEvent))
		 {
		   RETAILMSG(1, (L"CAN_Init: GPIO InterruptInitialize failure\r\n"));
		   __leave;
		 }

	    rxqInit();

		pCxt->hIntrServThread = CreateThread(0, 0, CANBIntrServThread, pCxt, 0, 0);
		if(!pCxt->hIntrServThread )
		{
			RETAILMSG(1, (L"CAN: CAN_Init, CreateThread FAILED!!! err#0x%04X\r\n", GetLastError()));
			__leave;
		}

		pCxt->hEchoThread = CreateThread(0, 0, CANBEchoThread, pCxt, 0, 0);
		if(!pCxt->hEchoThread )
		{
			RETAILMSG(1, (L"CAN: CAN_Init, CreateThread hEchoThread FAILED!!! err#0x%04X\r\n", GetLastError()));
			__leave;
		}


		dwRetVal = (DWORD)pCxt ; //dwContext;
		g_Status = CLOSED;

	}
	__finally
	{
		if(!dwRetVal) 
		{
			// DeInitializes MCP2515 related Resources
			if(!MCP2515DeInit(pCxt))
			{
				RETAILMSG(1, (L"CAN: CAN_Deinit, MCP2515 DeInitialization FAILED!!!\r\n"));
			}
			DeleteCriticalSection(&g_Access);
		}

		DEBUGMSG(ZONE_INIT | ZONE_STEPS, (L"CAN: -CAN_Init, Return Value=0x%08X\r\n", dwRetVal));
		RETAILMSG(CAN_DRV_DBG, (L"CAN: -CAN_Init, Return Value=0x%08X\r\n", dwRetVal));
	}

	return dwRetVal;
}

// Function name	: CAN_Deinit
// Description	    : Driver DeInitialization
// Return type		: BOOL CAN_Deinit
// Argument         : DWORD hDeviceContext

extern "C" BOOL CAN_Deinit ( DWORD hDeviceContext )
{
  CAN_INIT_CONTEXT *pCtx = (CAN_INIT_CONTEXT *)hDeviceContext;

	if( g_Status == DESTROYED )
	{
		RETAILMSG(1, (L"CAN: CAN_Deinit is failed. Handle=0x%08X is illegal\r\n", hDeviceContext));
		return FALSE;
	}
	EnterCriticalSection(&g_Access);
	if( g_Status == OPENED )
	{
		AbortAllPendingIO();
	}
	// DeInitializes MCP2515 related Resources
	if(!MCP2515DeInit(pCtx))
	{
		DEBUGMSG(ZONE_ERROR, (L"CAN: CAN_Deinit, MCP2515 DeInitialization FAILED!!!\r\n"));
	}
	// Force IST completion by closing the interrupt event handle
	CANB_DismissIST(pCtx);
	// If IST does not exit, then terminate it
	if( WAIT_OBJECT_0 != WaitForSingleObject(pCtx->hIntrServThread, IST_TERMINATE_TIMEOUT) )
	{
		TerminateThread(pCtx->hIntrServThread, 0);
	}

	if( WAIT_OBJECT_0 != WaitForSingleObject(pCtx->hEchoThread, IST_TERMINATE_TIMEOUT) )
	{
		TerminateThread(pCtx->hEchoThread, 0);
	}

	// Close Query Access Handle
	CloseHandle(hQueryAccess);
	// Close IST Thread Handle
	CloseHandle(pCtx->hIntrServThread);

	CloseHandle(pCtx->hEchoThread);
		
	g_Status = DESTROYED;

	GPIOInterruptDisable(pCtx->hGpio, nCAN_IRQ);
    // release the SYSINTR value
	GPIOInterruptRelease(pCtx->hGpio, nCAN_IRQ);

#if 0
	if( pCtx->sysIntr != -1 )
	{
      if( KernelIoControl(IOCTL_HAL_RELEASE_SYSINTR, &pCtx->sysIntr, sizeof(DWORD), NULL, 0, NULL) )
		  RETAILMSG(CAN_DRV_DBG, (L"CAN_Deinit: # %d sysIntr released\r\n", pCtx->sysIntr ));
	  else
		RETAILMSG(1, (L"CAN_Deinit: # %d sysIntr release failure\r\n", pCtx->sysIntr ));
	}
	else
		RETAILMSG(1, (L"CAN_Deinit: Invalid s_sysIntr # %d\r\n", pCtx->sysIntr ));
#endif

   // Close GPIO ???

	LeaveCriticalSection(&g_Access);
	DeleteCriticalSection(&g_Access);

	LocalFree(pCtx);
	g_pInitCtx = NULL;

	return TRUE;
}

// Function name	: CAN_Open
// Description	    : Return Handle to User Application, by Specific Device Context
// Return type		: DWORD CAN_Open 
// Argument         : DWORD hDeviceContext
// Argument         : DWORD AccessCode
// Argument         : DWORD ShareMode

extern "C" DWORD CAN_Open(DWORD hDeviceContext, DWORD AccessCode, DWORD ShareMode)
{
	DWORD dwRetVal = 0;
	RETAILMSG(CAN_DRV_DBG, (L"CAN: +CAN_Open, Context=0x%08X\r\n", hDeviceContext));
	if (AccessCode == 0) // Query Access is requested
	{
		RETAILMSG(CAN_DRV_DBG, (L"CAN: CAN_Open: Query Access is requested\r\n"));
		dwRetVal = (DWORD)hQueryAccess;
		goto exit;
	}
	__try
	{
		EnterCriticalSection(&g_Access);
		if( g_Status != CLOSED )
		{
			RETAILMSG(1, (L"CAN: CAN_Open is failed. (Device was not closed.)\r\n"));
			__leave;
		}
		hAbortEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if( hAbortEvent == NULL )
		{
			RETAILMSG(1, (L"CAN: CAN_Open, CreateEvent FAILED!!! err#0x%04X\r\n", GetLastError()));
			__leave;
		}
		g_Status = OPENED;
		dwRetVal = (DWORD)hAbortEvent;
		CANBPowerOn(g_pInitCtx, D0);
		RETAILMSG(CAN_DRV_DBG, (L"CAN: CAN_Open Ok. Status = OPENED\r\n"));
	}
	__finally
	{
		LeaveCriticalSection(&g_Access);
	}
exit:
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -CAN_Open, Return Handle=0x%08X\r\n", dwRetVal));
	return dwRetVal;
}

// Function name	: CAN_Close
// Description	    : Close Handle
// Return type		: BOOL CAN_Close 
// Argument         : DWORD hOpenContext

extern "C" BOOL CAN_Close(DWORD hOpenContext)
{
	BOOL bRetVal = FALSE;

    if( hOpenContext == 0 )
	{
	 return FALSE;
	}

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +CAN_Close, Context=0x%08X\r\n", hOpenContext));

	if (hOpenContext == (DWORD)hQueryAccess)
	{
		RETAILMSG(CAN_DRV_DBG, (L"CAN: +CAN_Close, QueryAccess\r\n"));
		return TRUE;
	}
	__try
	{
		EnterCriticalSection(&g_Access);
		if( g_Status != OPENED )
		{
			RETAILMSG(1, (L"CAN: CAN_Close is failed. Handle=0x%08X is not opened\r\n", hOpenContext));
			__leave;
		}
		AbortAllPendingIO();
		g_Status = CLOSED;
		bRetVal = TRUE;
		CANBPowerOff(g_pInitCtx, D4, 0);
	}
	__finally
	{
		LeaveCriticalSection(&g_Access);
	}

	RETAILMSG(CAN_DRV_DBG, (L"CAN: -CAN_Close, Context=0x%08X\r\n", hOpenContext));
	return bRetVal;
}

// Function name	: CAN_PowerUp
// Description	    : Power Up
// Return type		: void CAN_PowerUp 
// Argument         : DWORD hDeviceContext

extern "C" void CAN_PowerUp(DWORD hDeviceContext)
{
#if 0
	RETAILMSG(1,( L"CAN: +CAN_PowerUp, Context=0x%08X\r\n", hDeviceContext));
	InPowerHandler = 1;
	CANBPowerOn();
	InPowerHandler = 0;
	RETAILMSG(1,(L"CAN: -CAN_PowerUp, Context=0x%08X\r\n", hDeviceContext));
#endif
}

// Function name	: CAN_PowerDown
// Description	    : Power Down
// Return type		: void CAN_PowerDown 
// Argument         : DWORD hDeviceContext

extern "C" void CAN_PowerDown (DWORD hDeviceContext)
{
#if 0
	CAN_INIT_CONTEXT *pCtx = (CAN_INIT_CONTEXT *)hDeviceContext;

	RETAILMSG(1, (L"CAN: +CAN_PowerDown, Context=0x%08X\r\n", hDeviceContext));
	InPowerHandler = 1;
	CANBPowerOff(pCtx, 0);
	InPowerHandler = 0;
	RETAILMSG(1, (L"CAN: -CAN_PowerDown, Context=0x%08X\r\n", hDeviceContext));
#endif
}

// Function name	: CAN_Seek
// Description	    : NOT SUPPORTED
// Return type		: DWORD CAN_Seek 
// Argument         : DWORD hOpenContext
// Argument         : long Amount
// Argument         : WORD Type

extern "C" DWORD CAN_Seek (DWORD hOpenContext, long Amount, WORD Type)
{
	return (DWORD)-1;
}

// Function name	: CAN_Read
// Description	    : Read data from CAN Bus
// Return type		: DWORD CAN_Read 
// Argument         : DWORD hOpenContext
// Argument         : LPVOID pBuffer
// Argument         : DWORD Count

extern "C" DWORD CAN_Read(DWORD hOpenContext, LPVOID pBuffer, DWORD Count)
{
	DWORD dwRetVal = -1;
	DWORD dwRet;

    if( hOpenContext == 0 )
	{
	 return dwRetVal;
	}

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +CAN_Read, Context=0x%08X\r\n", hOpenContext));

	if (hOpenContext == (DWORD)hQueryAccess)
	{
		return dwRetVal;
	}
	__try
	{
		EnterCriticalSection(&g_Access);
		if( g_Status != OPENED )
		{
			RETAILMSG(1, (L"CAN: CAN_Read is failed. Handle=0x%08X is not opened\r\n", hOpenContext));
			SetLastError(ERROR_INVALID_HANDLE);
			__leave;
		}
		if( !pBuffer || (sizeof(CANRECEIVEPACKET) != Count) )
		{
			RETAILMSG(1, (L"CAN: CAN_Read, Invalid Parameters : pBuffer=0x%04X, Count=0x%04X !!!\r\n", pBuffer, Count));
			SetLastError(ERROR_INVALID_PARAMETER);
			__leave;
		}
		dwRetVal = 0;
	}
	__finally
	{
		LeaveCriticalSection(&g_Access);
	}

	if( dwRetVal != 0 ) return dwRetVal;

	dwRet = CANBReceiveMsg((HANDLE)hOpenContext, (PCANRECEIVEPACKET)pBuffer);
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -CAN_Read, CANBReceiveMsg ret = 0x%08X\r\n", dwRet));
	return(dwRet);
}

// Function name	: CAN_Write
// Description	    : Write data to CAN Bus
// Return type		: DWORD CAN_Write 
// Argument         : DWORD hOpenContext
// Argument         : LPCVOID pBuffer
// Argument         : DWORD Count

extern "C" DWORD CAN_Write(DWORD hOpenContext, LPCVOID pBuffer, DWORD Count)
{
	DWORD dwRetVal = -1;
	DWORD dwRet;

    if( hOpenContext == 0 )
	{
     RETAILMSG(1, (L"CAN: CAN_Write is failed. Handle=0x%08X\r\n", hOpenContext));
	 return dwRetVal;
	}

	RETAILMSG(CAN_DRV_DBG, (L"CAN: +CAN_Write, Context=0x%08X\r\n", hOpenContext));

	if (hOpenContext == (DWORD)hQueryAccess)
	{
		RETAILMSG(1, (L"CAN: CAN_Write is in QueryAccess. Handle=0x%08X\r\n", hOpenContext));
		return dwRetVal;
	}
	__try
	{
		EnterCriticalSection(&g_Access);
		if( g_Status != OPENED )
		{
			RETAILMSG(1, (L"CAN: CAN_Write is failed. Handle=0x%08X is not opened\r\n", hOpenContext));
			SetLastError(ERROR_INVALID_HANDLE);
			__leave;
		}
		if( !pBuffer || (sizeof(CANSENDPACKET) != Count) )
		{
			RETAILMSG(1, (L"CAN: CAN_Write, Invalid Parameters : pBuffer=0x%04X, Count=0x%04X !!!\r\n", pBuffer, Count));
			SetLastError(ERROR_INVALID_PARAMETER);
			__leave;
		}
		dwRetVal = 0;
	}
	__finally
	{
		LeaveCriticalSection(&g_Access);
	}

	//if( dwRetVal < 0 ) return dwRetVal;
    if( dwRetVal == (DWORD)-1 ) return dwRetVal;

	dwRet = CANBTransmitMsg((HANDLE)hOpenContext, (PCANSENDPACKET)pBuffer);
    RETAILMSG(CAN_DRV_DBG, (L"CAN: -CAN_Write, CANBTransmitMsg ret = 0x%08X\r\n", dwRet));
	return(dwRet);
}

// Function name	: CAN_IOControl
// Description	    : IOCtrl Implementation for Stream Interface Device Driver
// Return type		: BOOL CAN_IOControl 
// Argument         : DWORD  hOpenContext
// Argument         : DWORD  dwCode
// Argument         : PBYTE  pBufIn
// Argument         : DWORD  dwLenIn
// Argument         : PBYTE  pBufOut
// Argument         : DWORD  dwLenOut
// Argument         : PDWORD pdwActualOut

extern "C" BOOL CAN_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	BOOL bRetVal = FALSE;

	DEBUGMSG(ZONE_STEPS, (L"CAN: +CAN_IOControl, Handle=0x%08X, dwCode=0x%08X\r\n", hOpenContext, dwCode));

    if( hOpenContext == 0 )
	{
	 return bRetVal;
	}

	if (hOpenContext == (DWORD)hQueryAccess)
	{
		static CEDEVICE_POWER_STATE LastDx = D0;
		switch(dwCode)
		{
		case IOCTL_POWER_CAPABILITIES:
			if (pBufOut  &&  dwLenOut >= sizeof(POWER_CAPABILITIES)  &&  pdwActualOut)
			{
				PPOWER_CAPABILITIES ppc = (PPOWER_CAPABILITIES)pBufOut;
				memset(ppc, 0, sizeof(POWER_CAPABILITIES));
				ppc->DeviceDx   = DX_MASK(D0) | DX_MASK(D1) | DX_MASK(D2) | DX_MASK(D4);
#ifdef  WAKEUPONTHISDEVICE
				ppc->DeviceDx  |= DX_MASK(D3);
#endif//WAKEUPONTHISDEVICE
				ppc->WakeFromDx = DX_MASK(D3);
				*pdwActualOut = sizeof(POWER_CAPABILITIES);
				RETAILMSG(1, (L"CAN: IOCTL_POWER_CAPABILITIES: DeviceDx=0x%02X, WakeFromDx=0x%02X\r\n", ppc->DeviceDx, ppc->WakeFromDx));
				bRetVal = TRUE;
			}
			break;
		case IOCTL_POWER_QUERY:
			if (pBufOut  &&  dwLenOut >= sizeof(CEDEVICE_POWER_STATE)  &&  pdwActualOut)
			{
				CEDEVICE_POWER_STATE reqDx = *(PCEDEVICE_POWER_STATE)pBufOut;
				RETAILMSG(CAN_DRV_DBG, (L"CAN: IOCTL_POWER_QUERY: D%d\r\n", reqDx));
				//*(PCEDEVICE_POWER_STATE)pBufOut = PwrDeviceUnspecified; // way to veto the query
				*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
				bRetVal = TRUE;
			}
			break;
		case IOCTL_POWER_GET:
			if (pBufOut  &&  dwLenOut >= sizeof(CEDEVICE_POWER_STATE)  &&  pdwActualOut)
			{
				*(PCEDEVICE_POWER_STATE)pBufOut = LastDx;
				RETAILMSG(CAN_DRV_DBG, (L"CAN: IOCTL_POWER_GET: D%d\r\n", LastDx));
				*pdwActualOut = sizeof(CEDEVICE_POWER_STATE);
				bRetVal = TRUE;
			}
			break;
		case IOCTL_POWER_SET:
			if (pBufOut  &&  dwLenOut >= sizeof(CEDEVICE_POWER_STATE)  &&  pdwActualOut)
			{
				CEDEVICE_POWER_STATE newDx = *(PCEDEVICE_POWER_STATE)pBufOut;
				RETAILMSG(CAN_DRV_DBG, (L"CAN: IOCTL_POWER_SET: requested D%d\r\n", newDx));
				switch (newDx)
				{
				case D0:
					CANBPowerOn(g_pInitCtx, LastDx);
					LastDx = D0;
					//CANBPowerOn();
#ifdef  WAKEUPONTHISDEVICE
					//KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &dwSysIntr, sizeof(dwSysIntr), NULL, 0, NULL);
   	 			    GPIOWakeDisable(g_pInitCtx->hGpio, nCAN_IRQ);

#endif//WAKEUPONTHISDEVICE
					bRetVal = TRUE;
					break;
				case D3:
#ifdef  WAKEUPONTHISDEVICE
					AbortAllPendingIO();
					LastDx = newDx;
					CANBPowerOff(g_pInitCtx, newDx, 1);
					//KernelIoControl(IOCTL_HAL_ENABLE_WAKE,  &dwSysIntr, sizeof(dwSysIntr), NULL, 0, NULL);
					GPIOWakeEnable(g_pInitCtx->hGpio, nCAN_IRQ);
					bRetVal = TRUE;
					break;
#endif//WAKEUPONTHISDEVICE
				case D1:
				case D2:
				case D4:
					LastDx = newDx;
					if( (newDx == D1 )||(newDx == D2) )
					  CANBPowerOff(g_pInitCtx, newDx, 1);
					else
                      CANBPowerOff(g_pInitCtx, newDx, 0);
#ifdef  WAKEUPONTHISDEVICE
					//KernelIoControl(IOCTL_HAL_DISABLE_WAKE, &dwSysIntr, sizeof(dwSysIntr), NULL, 0, NULL);
					GPIOWakeDisable(g_pInitCtx->hGpio, nCAN_IRQ);
#endif//WAKEUPONTHISDEVICE
					bRetVal = TRUE;
					break;
				default:
					break;
				}
				// did we set the device power?
				if (bRetVal == TRUE)
				{
					*(PCEDEVICE_POWER_STATE)pBufOut = LastDx;
					*pdwActualOut = sizeof(LastDx);
					RETAILMSG(CAN_DRV_DBG, (L"CAN: IOCTL_POWER_SET: set D%d\r\n", LastDx));
				}
			}
			break;
		}
		if (bRetVal == FALSE)
		{
			SetLastError(ERROR_INVALID_PARAMETER);
			RETAILMSG(1, (L"CAN: CAN_IOControl: Device Query is FAILED.\r\n"));
		}
		return bRetVal;
	}

	EnterCriticalSection(&g_Access);
	if( g_Status != OPENED )
	{
		RETAILMSG(1, (L"CAN: CAN_IOControl is failed. Handle=0x%08X is not opened\r\n", hOpenContext));
		SetLastError(ERROR_INVALID_HANDLE);
		goto END;
	}
	SetLastError(ERROR_INVALID_PARAMETER);

	switch(dwCode)
	{
		case IOCTL_CAN_SET_BITTIMING:
		{

			if(pBufIn && (dwLenIn >= sizeof(CANCONFIG)))
				bRetVal = CANBSetCNF((PCANCONFIG)pBufIn);

			break;
		}
		case IOCTL_CAN_GET_BITTIMING:
		{
			if(pBufOut && (dwLenOut >= sizeof(CANCONFIG)))
			{
				bRetVal = CANBGetCNF((PCANCONFIG)pBufOut);

				if(bRetVal)
					*pdwActualOut = sizeof(CANCONFIG);
			}

			break;
		}
		case IOCTL_CAN_SET_MASK:
		{
			if(pBufIn && (dwLenIn >= sizeof(CANMASK)))
				bRetVal = CANBSetMask((PCANMASK)pBufIn);

			break;
		}
		case IOCTL_CAN_GET_MASK:
		{
			if(pBufOut && (dwLenOut >= sizeof(CANMASK)))
			{
				bRetVal = CANBGetMask((PCANMASK )pBufOut);

				if(bRetVal)
					*pdwActualOut = sizeof(CANMASK);
			}

			break;
		}
		case IOCTL_CAN_SET_FILTERS:
		{
			if(pBufIn &&(dwLenIn >= sizeof(CANFILTER)))
				bRetVal = CANBSetFilter((PCANFILTER)pBufIn);

			break;
		}
		case IOCTL_CAN_GET_FILTERS:
		{
			if(pBufOut && (dwLenOut >= sizeof(CANFILTER)))
			{
				bRetVal = CANBGetFilter((PCANFILTER)pBufOut);

				if(bRetVal )
					*pdwActualOut = sizeof(CANFILTER);
			}

			break;
		}
		case IOCTL_CAN_SET_RWTIMEOUT:
		{
			if(pBufIn && (dwLenIn >= sizeof(CANRWTIMEOUT)))
				bRetVal = CANBSetTimeOuts((PCANRWTIMEOUT)pBufIn);

			break;
		}
		case IOCTL_CAN_GET_RWTIMEOUT:
		{
			if(pBufOut && (dwLenOut >= sizeof(CANRWTIMEOUT)))
			{
				bRetVal = CANBGetTimeOuts((PCANRWTIMEOUT)pBufOut);

				if(bRetVal)
					*pdwActualOut = sizeof(CANRWTIMEOUT);
			}

			break;
		}
		case IOCTL_CAN_SET_RXBOPMODE:
		{
			if(pBufIn && (dwLenIn >= sizeof(CANRXBOPMODE)))
				bRetVal = CANBSetRXM((PCANRXBOPMODE)pBufIn);

			break;
		}
		case IOCTL_CAN_GET_RXBOPMODE:
		{
			if(pBufOut && (dwLenOut >= sizeof(CANRXBOPMODE)))
			{
				bRetVal = CANBGetRXM((PCANRXBOPMODE)pBufOut);

				if(bRetVal)
					*pdwActualOut = sizeof(CANRXBOPMODE);
			}

			break;
		}
		case IOCTL_CAN_MCP2515_RESET:
		{
			bRetVal = CANBResetMCP2515();

			break;
		}
		case IOCTL_CAN_GET_ERRORS:
		{
			if(pBufOut && (dwLenOut >= sizeof(DWORD)))
			{
				bRetVal = CANBGetErrors((DWORD *)pBufOut);

				if(bRetVal)
					*pdwActualOut = sizeof(DWORD);
			}

			break;
		}

		case IOCTL_CAN_GET_OPMODE:
		{
			if(pBufOut && (dwLenOut >= sizeof(DWORD)))
			{
                *((DWORD *)pBufOut) = CANBGetOpMode();
	
				*pdwActualOut = sizeof(DWORD);
			}

			break;
		}

		case IOCTL_CAN_SET_OPMODE:
		{
			if(pBufIn && (dwLenIn >= sizeof(CAN_OPMODE)))
				bRetVal = CANBSetOpMode(*((CAN_OPMODE *)pBufIn));

			break;
		}

		case IOCTL_CAN_SET_ECHO:
		{
			if(pBufIn && (dwLenIn >= sizeof(BOOL)))
			{
				if( *(BOOL *)pBufIn == TRUE ) 
				  g_EchoHandle = (HANDLE)hOpenContext;
				else
                  g_EchoHandle = NULL;

				bRetVal = TRUE;
			}

			break;
		}

		default:
			DEBUGMSG(ZONE_ERROR, (L"CAN: CAN_IOControl, Not Supported IOCTRL : dwCode=0x%04X !!!\r\n", dwCode));
			break;
	}

	if(bRetVal == TRUE)
		SetLastError(ERROR_SUCCESS);
END:
	LeaveCriticalSection(&g_Access);
	DEBUGMSG(ZONE_STEPS, (L"CAN: -CAN_IOControl, Return Value=0x%08X\r\n", bRetVal));
	return bRetVal;
}

// Function name	: CAN_DllEntry
// Description	    : Dll Entry
// Return type		: BOOL CAN_DllEntry 
// Argument         : HANDLE hInstDll
// Argument         : DWORD dwReason
// Argument         : LPVOID lpvReserved
//extern "C" BOOL CAN_DllEntry(HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved)
BOOL WINAPI DllEntry ( HANDLE hInstDll, DWORD dwReason, LPVOID lpvReserved )
{
   DEBUGMSG(ZONE_INIT|ZONE_DEINIT, (L"CAN: +DllEntry, hInstDll=0x%08X, dwReason=0x%08X, lpvReserved=0x%08X\r\n", hInstDll, dwReason, lpvReserved));
   RETAILMSG(CAN_DRV_DBG, (L"CAN: +DllEntry, hInstDll=0x%08X, dwReason=0x%08X, lpvReserved=0x%08X\r\n", hInstDll, dwReason, lpvReserved));

    switch ( dwReason ) 
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HINSTANCE)hInstDll);

			DEBUGREGISTER((HINSTANCE)hInstDll);
			DEBUGMSG(ZONE_INIT, (L"CAN: DLL_PROCESS_ATTACH\r\n"));
			RETAILMSG(CAN_DRV_DBG, (L"CAN: DLL_PROCESS_ATTACH\r\n"));

			break;
		case DLL_PROCESS_DETACH:
			DEBUGMSG(ZONE_DEINIT, (L"CAN: DLL_PROCESS_DETACH\r\n"));
			RETAILMSG(CAN_DRV_DBG, (L"CAN: DLL_PROCESS_DETACH\r\n"));
			break;
	}

	DEBUGMSG(ZONE_INIT|ZONE_DEINIT, (L"CAN: -DllEntry\r\n"));
	RETAILMSG(CAN_DRV_DBG, (L"CAN: -DllEntry\r\n"));

	return 1;
}
