/***************************************************************************
**+----------------------------------------------------------------------+**
**|                                ****                                  |**
**|                                ****                                  |**
**|                                ******o***                            |**
**|                          ********_///_****                           |**
**|                           ***** /_//_/ ****                          |**
**|                            ** ** (__/ ****                           |**
**|                                *********                             |**
**|                                 ****                                 |**
**|                                  ***                                 |**
**|                                                                      |**
**|     Copyright (c) 1998 - 2009 Texas Instruments Incorporated         |**
**|                        ALL RIGHTS RESERVED                           |**
**|                                                                      |**
**| Permission is hereby granted to licensees of Texas Instruments       |**
**| Incorporated (TI) products to use this computer program for the sole |**
**| purpose of implementing a licensee product based on TI products.     |**
**| No other rights to reproduce, use, or disseminate this computer      |**
**| program, whether in part or in whole, are granted.                   |**
**|                                                                      |**
**| TI makes no representation or warranties with respect to the         |**
**| performance of this computer program, and specifically disclaims     |**
**| any responsibility for any damages, special or consequential,        |**
**| connected with the use of this program.                              |**
**|                                                                      |**
**+----------------------------------------------------------------------+**
***************************************************************************/

/** \file  osapi.c 
 *  \brief api to some basic function that are os dependent
 *
 *  \see   
 */

#include "tidef.h"

#include <Winsock2.h>
#include <windows.h>
#include <ceddk.h>
#include <ceddkex.h>
#include <initguid.h>
#if defined(BSP_EVM_DM_AM_37X)
#include <sdk_gpio.h>
#else
#include <gpio.h>
#endif
#include <ndis.h>

#include "WlanDrvIf.h"
#include "osEntry.h"
#include "osapi.h"
#include "bmtrace_api.h"
#ifdef GEM_SUPPORTED
#include "gem_api.h"
#endif

#define DRIVER_NAME L"TIWLAN"

typedef struct _OS_PROTECT {
	CRITICAL_SECTION        tCriticalSection;
} OS_PROTECT, *POS_PROTECT;

typedef struct _TIMER_DATA {
	TWlanDrvIfObjPtr		pAdapter;
	NDIS_MINIPORT_TIMER		NdisTimer;
	fTimerFunction			pTimerFunc;
	TI_HANDLE				TimerCtx;
} TIMER_DATA, *PTIMER_DATA;

NDIS_STATUS recvNewPacket(TWlanDrvIfObjPtr pAdapter, RxIfDescriptor_t* pRxDesc, void *pWlanBuffer, USHORT Length);

static NDIS_PHYSICAL_ADDRESS High = NDIS_PHYSICAL_ADDRESS_CONST(-1, -1);
static void send_frag(char* msg, int message_len, int level, int module);

TI_BOOL bRedirectOutputToTicon = FALSE; /* when set to TRUE output will be send to TICON to print to the serial console */
TI_BOOL bRedirectOutputToLogger = FALSE;
SOCKET g_redirectSocketId = 0;

#define FRAG_SIZE	200
#define OS_CCX_CCKM_START_SIZE	14

#define TI_DRIVER_PRINT_REDIRECTED_PORT 3421

TI_BOOL use_debug_module = TI_FALSE;

extern void SendLoggerData(TI_UINT8* data, TI_UINT16 len);


/****************************************************************************************
 *                        																*
 *						OS Report API													*       
 *																						*
 ****************************************************************************************/

/** 
 * \fn     os_setDebugMode
 * \brief  Set the Debug Mode
 * 
 * \param  enable - TI_BOOL
 * \return void 
 */ 
void os_setDebugMode(TI_BOOL enable)
{
    use_debug_module = enable;
}


/** 
 * \fn     os_initPrintRedirectSocket
 * \brief  Initialize redirection of output 
 * 
 *  direct the output to the right console
 * 
 * \return SOCKET 
 */ 
SOCKET os_initPrintRedirectSocket(void)
{
    SOCKET socket_id = 0;
    int result;
    WORD wVersionRequested;
    WSADATA wsaData;
    
    wVersionRequested = MAKEWORD( 2, 2 );

    result = WSAStartup( wVersionRequested, &wsaData );
    if ( result != 0 ) {
        /* 
		 * Tell the user that we could not find a usable 
         * WinSock DLL.                                 
		 */
        os_printf("Could not initializate WinSocks.\n" );
        return(0);
    }

    /* Confirm that the WinSock DLL supports 2.2.
     * Note that if the DLL supports versions later    
     * than 2.2 in addition to 2.2, it will still return 
     * 2.2 in wVersion since that is the version we      
     * requested.                                        
	 */

    if ( LOBYTE( wsaData.wVersion ) != 2 ||
            HIBYTE( wsaData.wVersion ) != 2 ) {
        /* 
		 * Tell the user that we could not find a usable 
         * WinSock DLL.                                 
		 */
        WSACleanup( );
        os_printf("Could not find a usable WinSock DLL.\n" );
        return(0);
    }

    /* The WinSock DLL is acceptable. Proceed. */
	
    /* Open a socket */
    socket_id = socket(PF_INET, SOCK_DGRAM, 0);

    if (!socket_id) {
        /* Error opening socket */
        os_printf("%s: error opening socket socket.\n", __FUNCTION__);
        return(0);
    }

    return(socket_id);
}

/** 
 * \fn     os_setDebugOutputToTicon
 * \brief  Redirect output of Ticon
 * 
 * \param  value - TI_BOOL
 * \return void 
 */ 
VOID os_setDebugOutputToTicon(TI_BOOL value)
{
    if(value == TRUE){
        g_redirectSocketId = os_initPrintRedirectSocket();
        if(g_redirectSocketId == 0){
            RETAILMSG(1, (TEXT("Error init socket for printout redirection\n")));
            return;
        }
    }
    else {
        closesocket(g_redirectSocketId);
    }
    bRedirectOutputToTicon = value;
    RETAILMSG(1, (TEXT("\nos_setDebugOutputToTicon set flag to %d\n"), value));
}


VOID os_setDebugOutputToLogger(TI_BOOL value)
{
    bRedirectOutputToLogger = value;
    RETAILMSG(1, (TEXT("\nos_setDebugOutputToLogger set flag to %d\n"), value));
}


/** 
 * \fn     os_printf
 * \brief  Print formatted output
 * 
 * \param  format - const char* ,...
 * \return void 
 */ 
void os_printf(const char *format ,...)
{
    static int from_new_line = 1;       	/* Used to save the last message EOL */
    static TI_UINT8 module = 0;            	/* Used to save the last message module */
    static TI_UINT8 level = 0;             	/* Used to save the last message level */
		va_list ap;
    char msg[500];    						/* TI added static at 4.03.9 MAY BE REENTRANCY PROBLEM.. */
    char *p_msg = &msg[1];                  /* Pointer to the message */
    UINT16 message_len;                 
    UINT32 sec = 0;
    UINT32 uSec = 0;
    int result;
    struct sockaddr_in server_addr;

    /* Format the message and keep the message length */
		va_start(ap,format);
    message_len = vsprintf(p_msg, format, ap);

	#ifdef TI_DBG
    if (use_debug_module)
    {
        /* Use debug module */
        if (msg[1] == '$')
        {
            /************************************
                  Message format:		"$XX" 
                                         |||
                      message prefix ----|||
                      module index -------||
                      severity index ------|
            ************************************/

            level = (msg[2] - 'A');
            module = (msg[3] - 'A');
        }
        else
        {
            send_frag(msg, message_len, level, module);
        }
    }
    else
	/* ifdef TI_DBG */
	#endif 
    {
        /***********************/
        /* Use regular printf */
        /*********************/


        /* Convert to wide char */
        WCHAR wdMsg[512];

        if(bRedirectOutputToTicon == TRUE)
        {
        
        if (msg[1] == '$')
         {
                p_msg += 3;
                if(message_len == 3){
                    goto _check_new_line;
                }
         }
            server_addr.sin_family = AF_INET;
			 /* local host address*/
            server_addr.sin_addr.s_addr = htonl (0x7F000001);
            server_addr.sin_port = htons(TI_DRIVER_PRINT_REDIRECTED_PORT);
            result = sendto(g_redirectSocketId, p_msg, strlen(p_msg) + 1, 0, 
                            (const struct sockaddr *)&server_addr, sizeof(server_addr));
            if(result == SOCKET_ERROR) {
                closesocket(g_redirectSocketId);
                g_redirectSocketId = 0;
                RETAILMSG(1, (TEXT("Error sendto socket. Printout redirection canceled\r\n")));
            }

        
        }
		
			if ( from_new_line )
			{
				if (msg[1] == '$')
				{
					p_msg += 3;
				}
				if (MultiByteToWideChar(CP_ACP, 0, p_msg, -1, wdMsg, 512) > 0)
				{
					sec = os_timeStampUs(NULL);
					uSec = sec % 1000000;
					sec /= 1000000;
					
					RETAILMSG(1, (TEXT("%s: %d.%06d: %s"), DRIVER_NAME, sec, uSec, wdMsg));
				}
			}
			else
			{
				if (MultiByteToWideChar(CP_ACP, 0, p_msg, -1, wdMsg, 512) > 0)
				{
					RETAILMSG(1, (TEXT("%s\r"), wdMsg));
				}
			}
       
_check_new_line:
        from_new_line = ( msg[message_len] == '\n' );
    }
	va_end(ap);
}

#ifdef TI_DBG
/** 
 * \fn     send_frag
 * \brief  Print formatted output
 * 
 * \param  msg - string to be sent
 * \param  level - message error level
 * \parma  module - the module that sent the message
 * \return void 
 */ 
static void send_frag(char* msg, int message_len, int level, int module)
{
    int return_value=0;
    int offset = 1;
    int TmpLen;
    char* FragMsg;

    do
    {
        TmpLen = min(message_len - offset, FRAG_SIZE);
        FragMsg = msg + offset - 1;
        FragMsg[0] = module;
        if (return_value)
        {
            /* Message overrun */

            /* Send the overrun indication to the debug module */
            os_memoryCopy(NULL, &msg[1], "*** Message Overrun ***", strlen("*** Message Overrun ***"));
            msg[0] = 0;
        }
        offset += TmpLen;
    }while (offset < message_len);
}
/* ifdef TI_DBG */
#endif 



/****************************************************************************************
 *                        																*
 *							OS TIMER API												*
 *																						*
 ****************************************************************************************/

/*-----------------------------------------------------------------------------

Routine Name:

	GeneralTimerRoutine

Routine Description:

	
Arguments:

	
Return Value:

	None

-----------------------------------------------------------------------------*/
static void
GeneralTimerRoutine(
	IN void* SystemSpecific1,
	IN void* pContext,
	IN void* SystemSpecific2,
	IN void* SystemSpecific3
	)
{
	PTIMER_DATA pTimerData = (PTIMER_DATA) pContext;
	TWlanDrvIfObjPtr pAdapter = pTimerData->pAdapter;

    if (pTimerData->pTimerFunc != NULL)
    {  /* TRS:AS check for timer callback before call it */
		pTimerData->pTimerFunc(pTimerData->TimerCtx);
    }
}


/** 
 * \fn     os_timerCreate
 * \brief  creates and initializes an OS timer object
 * 
 *  This function creates and initializes an OS timer object associated with a
 *	caller's pRoutine function.
 *
 * \note   1) The user's callback is called directly from OS timer context when expired.
 * \note   2) In some OSs, it may be needed to use an intermediate callback in the 
 * \note      osapi layer (use os_timerHandlr for that).
 * \param  OsContext   - The OS handle
 * \param  pRoutine    - The user callback function
 * \param  hFuncHandle - The user callback handle
 * \return TI_HANDLE 
 */ 
TI_HANDLE os_timerCreate (TI_HANDLE OsContext, fTimerFunction pRoutine, TI_HANDLE hFuncHandle)
{

    TWlanDrvIfObjPtr pAdapter;
	PTIMER_DATA pTimerData;

    pAdapter = (TWlanDrvIfObjPtr) OsContext;

	pTimerData = (PTIMER_DATA) os_memoryAlloc(OsContext, sizeof(TIMER_DATA));

    if (!pTimerData)
    {
		return NULL;
	}

    pTimerData->pAdapter = pAdapter;
	pTimerData->pTimerFunc = pRoutine;
	pTimerData->TimerCtx = hFuncHandle;

	NdisMInitializeTimer(&pTimerData->NdisTimer, pAdapter->MiniportHandle,
		GeneralTimerRoutine, (PVOID) pTimerData);

    return(TI_HANDLE) pTimerData;
}


/** 
 * \fn     os_timerDestroy
 * \brief  This function destroys the OS timer object.
 * 
 * \param  OsContext   - The OS handle
 * \param  TimerHandle - The Timer handle
 * \return void 
 */ 
void os_timerDestroy (TI_HANDLE OsContext, TI_HANDLE TimerHandle)
{
    os_timerStop(OsContext, TimerHandle);
	os_memoryFree(OsContext, (PVOID) TimerHandle, sizeof(TIMER_DATA));
}


/** 
 * \fn     os_timerStart
 * \brief  This function start the timer object.
 * 
 * \param  OsContext   - The OS handle
 * \param  TimerHandle - The Timer handle
 * \param  DelayMs - The time in MS till the timer is awaken
 * \return void 
 * \sa     
 */ 
void os_timerStart (TI_HANDLE OsContext, TI_HANDLE TimerHandle, TI_UINT32 DelayMs)
{
	PTIMER_DATA pTimerData = (PTIMER_DATA) TimerHandle;

	NdisMSetTimer(&pTimerData->NdisTimer, DelayMs);
}


/** 
 * \fn     os_stopTimer
 * \brief  This function stops the timer object.
 * 
 * \param  OsContext   - The OS handle
 * \param  TimerHandle - The Timer handle
 * \return void 
 */ 
void os_timerStop (TI_HANDLE OsContext, TI_HANDLE TimerHandle)
{
	PTIMER_DATA pTimerData = (PTIMER_DATA) TimerHandle;
	TI_BOOLEAN Canceled;

	NdisMCancelTimer(&pTimerData->NdisTimer, &Canceled);
}


/** 
 * \fn     os_periodicIntrTimerStart
 * \brief  This function starts the periodic interrupt mechanism.
 * 
 *  This mode is used when interrupts that usually received from the Fw is now masked,
 *  and we are checking for any need of Fw handling in time periods.
 *
 * \param  OsContext   - The OS handle
 * \return void 
 */ 
#ifdef PRIODIC_INTERRUPT
void os_periodicIntrTimerStart (TI_HANDLE OsContext)
{
	/* to use TIWLAN_IRQ_POLL_INTERVAL_MS */
}
#endif


/** 
 * \fn     os_timeStampMs
 * \brief  returns the number of milliseconds that have elapsed since the system was booted.
 * 
 * \param  OsContext   - The OS handle
 * \return void 
 */ 
TI_UINT32 os_timeStampMs (TI_HANDLE OsContext)
{
	ULONG RetValue;

	NdisGetSystemUpTime(&RetValue);

	return RetValue;
}


/** 
 * \fn     os_timeStampUs
 * \brief  returns the number of microseconds that have elapsed since the system was booted.
 * 
 * \note  sometimes this function will be called with NULL(!!!) as argument!
 * \param  OsContext   - The OS handle
 * \return void 
 */ 
TI_UINT32 os_timeStampUs (TI_HANDLE OsContext)
{
    return os_timeStampMs(OsContext);
}


/****************************************************************************************
 *                        																*
 *							Protection services	API										*
 *																						*
 ****************************************************************************************
 

/** 
 * \fn     os_protectCreate
 * \brief  alocates a spinlock object.
 * 
 * \param  OsContext   - The OS handle
 * \return A handle of the created mutex/spinlock.
 * \return NULL if there is insufficient memory available or problems initializing the mutex 
 */ 
TI_HANDLE os_protectCreate (TI_HANDLE OsContext)
{
	TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr) OsContext;
	POS_PROTECT pOsProtect;

	pOsProtect = (POS_PROTECT) os_memoryAlloc(OsContext, sizeof(OS_PROTECT));

    if (!pOsProtect)
    {
		return NULL;
	}

	InitializeCriticalSection(&pOsProtect->tCriticalSection);

    return(TI_HANDLE) pOsProtect;
}


/** 
 * \fn     os_protectDestroy
 * \brief  destroys s a spinlock object.
 * 
 *  free the spinlock and then free the objects memory
 * 
 * \param  OsContext   - The OS handle
 * \param  ProtectCtx  - handle to the spinLock
 * \return void
 */
void os_protectDestroy (TI_HANDLE OsContext, TI_HANDLE ProtectCtx)
{
	DeleteCriticalSection(&((POS_PROTECT)ProtectCtx)->tCriticalSection);
	os_memoryFree(OsContext, (void*) ProtectCtx, sizeof(OS_PROTECT));
}


/** 
 * \fn     os_protectLock
 * \brief  locks the spinlock.
 * 
 * \param  OsContext   - The OS handle
 * \param  ProtectCtx  - handle to the spinLock
 * \return void
 */
void os_protectLock (TI_HANDLE OsContext, TI_HANDLE ProtectContext)
{
    EnterCriticalSection(&((POS_PROTECT)ProtectContext)->tCriticalSection);
}


/** 
 * \fn     os_protectUnlock
 * \brief  unlocks the spinlock.
 * 
 * \param  OsContext   - The OS handle
 * \param  ProtectCtx  - handle to the spinLock
 * \return void
 */
void os_protectUnlock (TI_HANDLE OsContext, TI_HANDLE ProtectContext)
{
    LeaveCriticalSection(&((POS_PROTECT)ProtectContext)->tCriticalSection);
}


/** 
 * \fn     os_StalluSec
 * \brief  This function make delay in microseconds.
 * 
 * \param  OsContext   - The OS handle
 * \param  uSec - delay time in microseconds
 * \return void
 */
void os_StalluSec(TI_HANDLE OsContext, TI_UINT32 uSec)
{
    /* NdisStallExecution is used only on desktops, CE uses NdisMSleep */
    NdisMSleep(uSec);
}

/****************************************************************************************
 *                        																*
 *							Other services API  										*
 *																						*
 ****************************************************************************************


/** 
 * \fn     os_receivePacket
 * \brief  Transfers a packet from WLAN driver to NDIS.
 * 
 * \param  OsContext   - The OS handle
 * \param  pPacket - a pointer to the data packet
 * \param  Length - packet length
 * \return TI_BOOL
 */
TI_BOOL os_receivePacket(TI_HANDLE OsContext, void *pRxDesc ,void *pPacket, TI_UINT16 Length)
{
	NDIS_STATUS Status;
    CL_TRACE_START_L1();  

	Status = recvNewPacket((TWlanDrvIfObjPtr)OsContext, pRxDesc, pPacket, Length);

    /* Note: Don't change this trace (needed to exclude OS processing from Rx CPU utilization) */
    CL_TRACE_END_L1("tiwlan_drv.ko", "OS", "RX", "");  

    if (Status != NDIS_STATUS_SUCCESS)
    {
		return TI_FALSE;
	}

	return TI_TRUE;
}


/** 
 * \fn     os_IndicateEvent
 * \brief  Indicate the OS about different connection events.
 * 
 * \param  OsContext - The OS handle
 * \param  IPC_EV_DATA - event data
 * \return TI_INT32
 */
TI_INT32 os_IndicateEvent (TI_HANDLE OsContext, IPC_EV_DATA* pData)
{
    TWlanDrvIfObjPtr pAdapter;
	TI_BOOL Indicate = FALSE;
	NDIS_STATUS ConnStatus;
    IPC_EV_DATA*         pEvent;
    
    pEvent = (IPC_EV_DATA*)pData;

    pAdapter = (TWlanDrvIfObjPtr)OsContext;

    PRINTF(DBG_RECV_LOUD, ("TIWL: Connection Status: Event=%d\n", pEvent->EvParams.uEventType));

    switch (pEvent->EvParams.uEventType)
    {
    
    case IPC_EVENT_ASSOCIATED:

			pAdapter->LinkStatus = NdisMediaStateConnected;
				ConnStatus = NDIS_STATUS_MEDIA_CONNECT;
			Indicate = TRUE;

#ifdef GEM_SUPPORTED
		if ( GEM_StatusHandler_Connect( pAdapter->MiniportHandle, ConnStatus, (void*)0, 0 ) != NDIS_STATUS_SUCCESS )
					{
							Indicate = FALSE;
					}
        PRINT(DBG_RECV_LOUD, "TIWL: ASSOCIATED <<<<< \n");
#endif
        if (Indicate)
        {
            NdisMIndicateStatus(pAdapter->MiniportHandle, ConnStatus, (void*) 0, 0);

            NdisMIndicateStatusComplete(pAdapter->MiniportHandle);
        }
        break;

    case IPC_EVENT_DISASSOCIATED:

			pAdapter->LinkStatus = NdisMediaStateDisconnected;
				ConnStatus = NDIS_STATUS_MEDIA_DISCONNECT;
			Indicate = TRUE;
#ifdef GEM_SUPPORTED
			if ( GEM_StatusHandler_Disconnect( pAdapter->MiniportHandle, ConnStatus, (void*)0, 0 ) != 
				 NDIS_STATUS_SUCCESS )
					{
							Indicate = FALSE;
					}
#endif
        PRINT(DBG_RECV_LOUD, "TIWL: DISASSOCIATED <<<<< \n");

        if (Indicate)
        {
            NdisMIndicateStatus(pAdapter->MiniportHandle, ConnStatus, (void*) 0, 0);

            NdisMIndicateStatusComplete(pAdapter->MiniportHandle);
        }
        break;

    case IPC_EVENT_LINK_SPEED:
        pAdapter->LinkSpeed = (*(PULONG)pEvent->uBuffer * 10000) / 2;
        break;

        /* generated by TI code, even if they do ignore it */
    case IPC_EVENT_MEDIA_SPECIFIC:
        
        NdisMIndicateStatus(pAdapter->MiniportHandle,NDIS_STATUS_MEDIA_SPECIFIC_INDICATION,
                            pEvent->uBuffer, pEvent->uBufferSize);
        NdisMIndicateStatusComplete(pAdapter->MiniportHandle);
        
        break;

    case    IPC_EVENT_SCAN_COMPLETE:
        break;
    case    IPC_EVENT_TIMEOUT:
        break;

        case IPC_EVENT_CCKM_START:

			#ifdef CCX_MODULE_INCLUDED
				NdisMIndicateStatus(pAdapter->MiniportHandle,
									NDIS_STATUS_FSW_CCX_CCKM_START, pEvent->uBuffer, OS_CCX_CCKM_START_SIZE);
				NdisMIndicateStatusComplete(pAdapter->MiniportHandle);
			#endif
        break;
	default:
        PRINT(DBG_IOCTL_ERROR, "TIWL: Not supported Connection Event\n");
		break;

	}

    return TI_OK;
}

/*--------------------------------------------------------------------------------------*/

void *os_SignalObjectCreate (TI_HANDLE OsContext)
{
    return (void *) CreateEvent(NULL, FALSE, FALSE, NULL);

} /* os_SignalObjectCreate() */

/*--------------------------------------------------------------------------------------*/

int os_SignalObjectWait (TI_HANDLE OsContext, void *ptr)
{
    if (WaitForSingleObject((HANDLE)ptr, INFINITE) != WAIT_OBJECT_0) 
    {
        return TI_NOK;
    }
    return TI_OK;

} /* os_SignalObjectWait() */

/*--------------------------------------------------------------------------------------*/

int os_SignalObjectSet (TI_HANDLE OsContext, void *ptr)
{
    if (!SetEvent((HANDLE)ptr))
    {
        return TI_NOK;
    }
    return TI_OK;

} /* os_SignalObjectWait() */

/*--------------------------------------------------------------------------------------*/

int os_SignalObjectFree (TI_HANDLE OsContext, void *ptr)
{
    if (!CloseHandle((HANDLE)ptr))
    {
        return TI_NOK;
    }
    return TI_OK;

} /* os_SignalObjectFree() */

/*--------------------------------------------------------------------------------------*/

int os_RequestSchedule (TI_HANDLE OsContext)
{
    int rc;
    TWlanDrvIfObjPtr pTWlanDrvIf = (TWlanDrvIfObjPtr)OsContext;

    /* Note: The performance trace below doesn't inclose the schedule itself because the rescheduling  
     *         can occur immediately and call os_RequestSchedule again which will confuse the trace tools
     */
    CL_TRACE_START_L3();
    CL_TRACE_END_L3("tiwlan_drv.ko", "OS", "TASK", "");

    rc =  os_SignalObjectSet(OsContext, pTWlanDrvIf->drvEvent);

    return rc;
} /* os_RequestSchedule() */

/*--------------------------------------------------------------------------------------*/

/** 
 * \fn     os_InterruptServiced
 * \brief  Called when IRQ line is not asserted any more 
 *			(i.e. we can enable IRQ in Level sensitive platform)
 * 
 * \param  OsContext - The OS handle
 * \return 
 */
void os_InterruptServiced (TI_HANDLE OsContext)
{
	/* To be implemented with Level IRQ */
}

/*-----------------------------------------------------------------------------
Routine Name:  os_WakeLockTimeout

Routine Description: Prevents system suspend for 1 sec if previously enabled 
                       by call to os_WakeLockTimeoutEnable.

Arguments:     OsContext - handle to OS context

Return Value:  1 if lock was enabled, 0 if not
-----------------------------------------------------------------------------*/
int os_WakeLockTimeout (TI_HANDLE OsContext)
{
	return 1;
}

/*-----------------------------------------------------------------------------
Routine Name:  os_WakeLockTimeoutEnable

Routine Description: Enables prevention of system suspend for 1 sec in next call to os_WakeLockTimeout

Arguments:     OsContext - handle to OS context

Return Value:  1 if lock was enabled, 0 if not
-----------------------------------------------------------------------------*/
int os_WakeLockTimeoutEnable (TI_HANDLE OsContext)
{
	return 1;
}

/*-----------------------------------------------------------------------------
Routine Name:  os_WakeLock

Routine Description: Called to prevent system from suspend

Arguments:     OsContext - handle to OS context

Return Value:  wake_lock counter
-----------------------------------------------------------------------------*/
int os_WakeLock (TI_HANDLE OsContext)
{
	return 0;
}

/*-----------------------------------------------------------------------------
Routine Name:  os_WakeUnlock

Routine Description: Called to allow system to suspend

Arguments:     OsContext - handle to OS context

Return Value:  wake_lock counter
-----------------------------------------------------------------------------*/
int os_WakeUnlock (TI_HANDLE OsContext)
{
	return 0;
}

int os_getWakeLockCounter(TI_HANDLE OsContext)
{
    return 0;
}

/*--------------------------------------------------------------------------------------*/

/** 
 * \fn     os_Trace
 * \brief  Prepare and send trace message to the logger.
 * 
 * \param  OsContext    - The OS handle
 * \param  uLevel   	- Severity level of the trace message
 * \param  uFileId  	- Source file ID of the trace message
 * \param  uLineNum 	- Line number of the trace message
 * \param  uParamsNum   - Number of parameters in the trace message
 * \param  ...          - The trace message parameters
 * 
 * \return void
 */
void os_Trace (TI_HANDLE OsContext, TI_UINT32 uLevel, TI_UINT32 uFileId, TI_UINT32 uLineNum, TI_UINT32 uParamsNum, ...)
{
	TI_UINT32	index;
	TI_UINT32	uParam;
	TI_UINT32	uMaxParamValue = 0;
	TI_UINT32	uMsgLen	= TRACE_MSG_MIN_LENGTH;
	TI_UINT8    aMsg[TRACE_MSG_MAX_LENGTH] = {0};
    TTraceMsg   *pMsgHdr  = (TTraceMsg *)&aMsg[0];
	TI_UINT8    *pMsgData = &aMsg[0] + sizeof(TTraceMsg);
	va_list	    list;

    if (!bRedirectOutputToLogger)
    {
        return;
    }

	if (uParamsNum > TRACE_MSG_MAX_PARAMS)
	{
		uParamsNum = TRACE_MSG_MAX_PARAMS;
	}

    /* sync on the parameters */
	va_start(list, uParamsNum);

	/* find the longest parameter */
	for (index = 0; index < uParamsNum; index++)
	{
		/* get parameter from the stack */
		uParam = va_arg (list, TI_UINT32);

		/* save the longest parameter at variable 'uMaxParamValue' */
		if (uParam > uMaxParamValue)
        {
			uMaxParamValue = uParam;
        }

		/* 32 bit parameter is the longest possible - get out of the loop */
		if (uMaxParamValue > UINT16_MAX_VAL)
        {
			break;
        }
	}

    /* Set msg length and format according to the biggest parameter value (8/16/32 bits) */
	if (uMaxParamValue > UINT16_MAX_VAL)		
	{
		pMsgHdr->uFormat = TRACE_FORMAT_32_BITS_PARAMS;
		uMsgLen += uParamsNum * sizeof(TI_UINT32);
	}
	else if (uMaxParamValue > UINT8_MAX_VAL)	
	{
		pMsgHdr->uFormat = TRACE_FORMAT_16_BITS_PARAMS;
		uMsgLen += uParamsNum * sizeof(TI_UINT16);
	}
	else							
	{
		pMsgHdr->uFormat = TRACE_FORMAT_8_BITS_PARAMS;
		uMsgLen += uParamsNum;
	}

    /* Fill all other header information */
    pMsgHdr->uLevel     = uLevel;
    pMsgHdr->uParamsNum = uParamsNum;
    pMsgHdr->uFileId    = uFileId;
    pMsgHdr->uLineNum   = uLineNum;

	/* re-sync on the parameters */
	va_start(list, uParamsNum);

	/* add the parameters */
	for (index = 0; index < uParamsNum; index++)
	{
		/* get parameter from the stack */
		uParam = va_arg(list, TI_UINT32);

		/* insert the parameter and increment msg pointer */
		switch(pMsgHdr->uFormat)
		{
		case TRACE_FORMAT_8_BITS_PARAMS:
			INSERT_BYTE(pMsgData, uParam);
			break;

		case TRACE_FORMAT_16_BITS_PARAMS:
			INSERT_2_BYTES(pMsgData, uParam);
			break;

		case TRACE_FORMAT_32_BITS_PARAMS:
			INSERT_4_BYTES(pMsgData, uParam);
			break;

		default:
			va_end(list);
            return;
		}     
	}

    /* Send the trace message to the logger */
	SendLoggerData(aMsg, uMsgLen);
	va_end(list);
}

/*--------------------------------------------------------------------------------------*/

/** 
 * \fn     os_SetDrvThreadPriority
 * \brief  Called upon init to set WLAN driver thread priority.
 * 
 * \param  OsContext              - The OS handle
 * \param  uWlanDrvThreadPriority - The WLAN driver thread priority
 * \return 
 */
void os_SetDrvThreadPriority (TI_HANDLE OsContext, TI_UINT32 uWlanDrvThreadPriority)
{
    TWlanDrvIfObjPtr pAdapter = (TWlanDrvIfObjPtr) OsContext;

    CeSetThreadPriority (pAdapter->drvThread, uWlanDrvThreadPriority);
}

#if !defined(BSP_EVM_DM_AM_37X)
#include <oal.h>
#include <oalex.h>
TI_INT32 os_getMAC(TI_UINT16 *mac, TI_UINT32 len)
{
	TI_UINT32 Ret;

	if(KernelIoControl(IOCTL_HAL_GET_WIFI_MACADDR, 0, 0x00, (LPVOID)mac, len, &Ret))
		Ret = 1;
	else
		Ret = 0;
	RETAILMSG(1, (L"TIWL: os_getMAC: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\r\n",
				  mac[0]&0xFF, mac[0]>>8, mac[1]&0xFF, mac[1]>>8, mac[2]&0xFF, mac[2]>>8));

	return Ret;
}
#endif


