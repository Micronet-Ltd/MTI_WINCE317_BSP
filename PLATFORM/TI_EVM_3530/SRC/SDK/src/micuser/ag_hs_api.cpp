/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           CPP Audio Gateway Headset API
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   18-Jan-2011
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */


/********************** INCLUDES **********************************************/
#include <windows.h>
#include <winreg.h>
#include <string.h>
#include <MicUserSdk.h>
#include <btagpub.h>
#include <initguid.h>
#include <bt_sdp.h>
#include <service.h>
#include "ag_hs_api.h"
#include <oal.h>
#include <oalex.h>
#include <bsp.h>
#include <args.h>
#include "hsdrv.h"

/********************** LOCAL CONSTANTS ***************************************/
#define AG_HS_DBG_PRN                          0

#define MAX_OPEN_HANDLERS                      32 

/*********************** LOCAL MACROS *****************************************/
typedef BT_DEVICE_CONTEXT_AG_HS  *(WINAPI *BTHAGOPENDEVICE)( void );
typedef INT32                     (WINAPI *BTHAGCLOSEDEVICE)( HANDLE );
typedef BOOL                      (WINAPI *BTHAGSETREGEVENT)(HANDLE, DWORD);
typedef BOOL                      (WINAPI *BTHAGGETREGEVENT)(HANDLE, DWORD *);
typedef BOOL                      (WINAPI *BTHAGWRITESHAREDMEM)( void *, DWORD, void *, DWORD);
typedef BOOL                      (WINAPI *BTHAGREADSHAREDMEM)( void *, DWORD, void *, DWORD);

/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
typedef struct 
{
    BT_ADDR                   b;
	BT_DEVICE_CONTEXT_AG_HS  *pDevContext;
	HMODULE                   BtAgSvcModule;
	BTHAGOPENDEVICE           pfnBthAGOpenDevice; 
	BTHAGCLOSEDEVICE          pfnBthAGCloseDevice;
	BTHAGSETREGEVENT          pfnBthAGSetRegEvent;
	BTHAGGETREGEVENT          pfnBthAGGetRegEvent;
	BTHAGWRITESHAREDMEM       pfnBthAGWriteSharedMem; 
    BTHAGREADSHAREDMEM        pfnBthAGReadSharedMem;  
	//DWORD               devContextSize;
	HANDLE                    hsQueue; 
    BOOL                      hsQuitStatus;
	
	//HANDLE              hClosingEvent;
	CRITICAL_SECTION          cs;

} MIC_DEVICE_CONTEXT_AG_HS;




/****************** STATIC FUNCTION PROTOTYPES *********************************/
static LONG  BthDelAgHSDevices(void);
static BOOL BthSetAgHSDevice(BT_ADDR b);
static BOOL IsWirelessModulePresent();
static BOOL IsWirelessModuleEnabled(VOID);
static INT32  AGHSOpenFail(HANDLE Handle, LPCTSTR pszFuncName, INT32 errNum );

/********************* GLOBAL VARIABLES ***************************************/

TCHAR agHsSemName[] = TEXT("AG_HS_SEM");

const TCHAR *agHsErros[AG_HS_MAX_ERRORS] =
{
    TEXT("AG_HS_OK"),
	TEXT("AG_HS_DEV_SET_ERROR"),
	TEXT("AG_HS_DEV_OPEN_ERROR"),
	TEXT("AG_HS_SERVICE_OPEN_ERROR"),
	TEXT("AG_HS_SERVICE_CTRL_ERROR"),
	TEXT("AG_HS_SERVICE_INVALID_CMD"),
	TEXT("AG_HS_SERVICE_INVALID_PARAM"),
	TEXT("AG_HS_SERVICE_INVALID_STATE"),
	TEXT("AG_HS_MODULE_ABSENT"),
	TEXT("AG_HS_MODULE_OFF"),
	TEXT("AG_HS_INVALID_HANDLE_VALUE"),
	TEXT("AG_HS_OPEN_OVERFLOW"),
	TEXT("AG_HS_INVALID_BT_ADDR"),
	TEXT("AG_HS_MSG_CREATE_ERROR"),
	TEXT("AG_HS_LIB_LOAD_ERROR"),
	TEXT("AG_HS_PROC_ADDR_ERROR"),
	TEXT("AG_HS_REG_EVENT_ERROR"),
	TEXT("AG_HS_CREATE_EVENT_ERROR"),
	TEXT("AG_HS_MSG_READ_ERROR"),
	TEXT("AG_HS_DEV_CLOSED"),
};
	

MIC_DEVICE_CONTEXT_AG_HS  *gpContext[MAX_OPEN_HANDLERS] = {NULL};
volatile LONG              gContextCount = 0;
CRITICAL_SECTION           gContextCS;
BT_ADDR                    gBtAddr = 0;

/********************* STATIC VARIABLES ***************************************/


/******************** FUNCTION DEFINITIONS ************************************/

/** 
 * @fn INT32  MIC_AGHSOpen(HANDLE *pHandle, BT_ADDR b)
 *  
 * @param    HANDLE *pHandle  - return HANDLE to device
 * @param    BT_ADDR b  - Bluetoot device address
 *
 * @return status of operation:
 *         0 - if Ok
 *         AG_HS_DEV_SET_ERROR - Failure
 *
 * @brief  Open Bluetooth Headset device (only one, mean only one BT address) 
 *         with given Bluetooth address, but can be multiple times opened
 *    
 */  

INT32  MIC_AGHSOpen(HANDLE *pHandle, BT_ADDR b)
{
  INT32                      iRet = AG_HS_OK;
  MIC_DEVICE_CONTEXT_AG_HS   *pContext = NULL;
  DWORD                      AgServiceStatus; 
  MSGQUEUEOPTIONS            msgOptions;
  BT_DEVICE_CONTEXT_AG_HS    *pBtDevice;
  
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S, process Id = 0x%X\r\n",__FUNCTION__, GetCurrentProcessId()));


  //AG Service should be started before
  // Temporary disable
  iRet = MIC_AGHSControl(AG_SERVICE_STATUS, &AgServiceStatus);
  if( iRet != AG_HS_OK )
  {
	RETAILMSG(1, (L"AG_HS_API:%S, Ag Service Control Failure, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
    return(iRet);
  }

  if( AgServiceStatus != SERVICE_STATE_ON )
  {
    iRet = AG_HS_SERVICE_INVALID_STATE;
	RETAILMSG(1, (L"AG_HS_API:%S, Ag Service Not Active, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
    return(iRet);
  }


  // Allocate memory for handler
  pContext = (MIC_DEVICE_CONTEXT_AG_HS *)LocalAlloc(LPTR, sizeof(MIC_DEVICE_CONTEXT_AG_HS));
  if(pContext == NULL)
   {
     iRet = AG_HS_INVALID_HANDLE_VALUE;
     RETAILMSG(1, (L"AG_HS_API:-%S, ret = %d\r\n",__FUNCTION__, iRet));
     return(iRet);
   }

  InitializeCriticalSection(&pContext->cs);

  pContext->hsQuitStatus = FALSE;

  // open Device
  pContext->BtAgSvcModule  = LoadLibrary( L"btagsvc.dll" );
  if( pContext->BtAgSvcModule == NULL )
  {
	 iRet = AG_HS_LIB_LOAD_ERROR;
	 LocalFree(pContext);
     RETAILMSG(1, (L"AG_HS_API:-%S, ret = %d\r\n",__FUNCTION__, iRet));
     return(iRet);
  }

  pContext->pfnBthAGOpenDevice  = (BTHAGOPENDEVICE)GetProcAddress( pContext->BtAgSvcModule, L"BthAGOpenDevice" );
  if( pContext->pfnBthAGOpenDevice == NULL )
  {
     return( AGHSOpenFail(pContext, L"BthAGOpenDevice", AG_HS_PROC_ADDR_ERROR) );
  }

	pContext->pfnBthAGWriteSharedMem = (BTHAGWRITESHAREDMEM)GetProcAddress( pContext->BtAgSvcModule, L"BthAGWriteSharedMem" );
    if( pContext->pfnBthAGWriteSharedMem == NULL )
    {
     return( AGHSOpenFail(pContext, L"BthAGWriteSharedMem", AG_HS_PROC_ADDR_ERROR) );
   }

	pContext->pfnBthAGReadSharedMem = (BTHAGREADSHAREDMEM)GetProcAddress( pContext->BtAgSvcModule, L"BthAGReadSharedMem" );
    if( pContext->pfnBthAGReadSharedMem == NULL )
    {
     return( AGHSOpenFail(pContext, L"BthAGReadSharedMem", AG_HS_PROC_ADDR_ERROR) );
    }

	pContext->pfnBthAGCloseDevice = (BTHAGCLOSEDEVICE)GetProcAddress( pContext->BtAgSvcModule, L"BthAGCloseDevice" );
	if( pContext->pfnBthAGCloseDevice == NULL )
    {
     return( AGHSOpenFail(pContext, L"BthAGCloseDevice", AG_HS_PROC_ADDR_ERROR) );
    }

   	//BTHAGSETREGEVENT          pfnBthAGSetRegEvent;
	pContext->pfnBthAGSetRegEvent = (BTHAGSETREGEVENT)GetProcAddress( pContext->BtAgSvcModule, L"BthAGSetRegEvent" );
	if( pContext->pfnBthAGCloseDevice == NULL )
    {
     return( AGHSOpenFail(pContext, L"BthAGSetRegEvent", AG_HS_PROC_ADDR_ERROR) );
    }

	//BTHAGGETREGEVENT          pfnBthAGGetRegEvent;
	pContext->pfnBthAGGetRegEvent = (BTHAGGETREGEVENT)GetProcAddress( pContext->BtAgSvcModule, L"BthAGGetRegEvent" );
	if( pContext->pfnBthAGCloseDevice == NULL )
    {
     return( AGHSOpenFail(pContext, L"BthAGGetRegEvent", AG_HS_PROC_ADDR_ERROR) );
    }


  //btDevice.contextNum = contextNum;
  pBtDevice = pContext->pfnBthAGOpenDevice();
  if( (pBtDevice == NULL) || (pBtDevice->bRes == FALSE) )
  {
     return( AGHSOpenFail(pContext, L"pfnBthAGOpenDevice", AG_HS_PROC_ADDR_ERROR) );
  }
  else
  {
   pContext->pDevContext = pBtDevice;
   RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, pDevContext = 0x%X\r\n",__FUNCTION__, pContext->pDevContext));
  }

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, shared mem addr = 0x%X\r\n",__FUNCTION__, pContext->pDevContext->pSharedMem));
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, msg name = %s\r\n",__FUNCTION__, pContext->pDevContext->hsQueueName));


  if( pBtDevice->contextNum == 0 )
  {
    RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Saving btAddr %04x%08x\r\n",__FUNCTION__, GET_NAP(b), GET_SAP(b))); 

    if( !pContext->pfnBthAGWriteSharedMem(pBtDevice->pSharedMem, 0, &b, sizeof(BT_ADDR)) )
	{
     RETAILMSG(1, (L"AG_HS_API:%S, BthAGWriteSharedMem  Failrue\r\n",__FUNCTION__));
	}
  }
  else
  {
   BT_ADDR  bStored;

   RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, comparing btAddr %04x%08x\r\n",__FUNCTION__, GET_NAP(b), GET_SAP(b))); 

   if( !pContext->pfnBthAGReadSharedMem(pBtDevice->pSharedMem, 0, &bStored, sizeof(BT_ADDR)) )
	{
     RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S,BthAGReadSharedMem  Failrue\r\n",__FUNCTION__));
	}

   if( bStored != b )
	{
     RETAILMSG(1, (L"AG_HS_API:%S, Different BT address  %04x%08x\r\n",__FUNCTION__, GET_NAP(bStored), GET_SAP(bStored)));
	}

  }


  if( pBtDevice->contextNum == 0 )
  {
	  BthDelAgHSDevices();

	  if( !BthSetAgHSDevice(b) )
	  {
		RETAILMSG(1, (L"AG_HS_API:-%S, ret = %d\r\n",__FUNCTION__, iRet));
		return( AGHSOpenFail(pContext, L"error", AG_HS_DEV_SET_ERROR) );
	  }

	  if( AgServiceStatus != SERVICE_STATE_ON )
	  {
	   // Try to start AG Service
	   iRet = MIC_AGHSControl(AG_SERVICE_START, NULL);
	   if( iRet != AG_HS_OK )
		{
  		 RETAILMSG(1, (L"AG_HS_API:%S, Ag Service Start Failure, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
		 return( AGHSOpenFail(pContext, L"error", iRet) );
		}
	  }
  }


    pContext->hsQuitStatus = FALSE;

	memset(&msgOptions, 0, sizeof(msgOptions));
	msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags = 0;
	msgOptions.cbMaxMessage = sizeof(AGHS_MSG);

	// create read queue
	msgOptions.bReadAccess = 1;

	pContext->hsQueue = CreateMsgQueue(pBtDevice->hsQueueName, &msgOptions);
	if(pContext->hsQueue == NULL)
	{
	 return( AGHSOpenFail(pContext, L"error", AG_HS_MSG_CREATE_ERROR) );
	}

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, contextNum = %d\r\n",__FUNCTION__, pBtDevice->contextNum));
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, msg name = %s\r\n",__FUNCTION__, pBtDevice->hsQueueName));

  *pHandle = pContext;

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, ret = %d, Handle = 0x%X\r\n",__FUNCTION__, iRet, *pHandle));

  return(iRet);
}


static INT32  AGHSOpenFail(HANDLE Handle, LPCTSTR pszFuncName, INT32 errNum )
{
  MIC_DEVICE_CONTEXT_AG_HS   *pContext = (MIC_DEVICE_CONTEXT_AG_HS *)Handle;

  FreeLibrary(pContext->BtAgSvcModule);
  LocalFree(pContext);
  RETAILMSG(1, (L"AG_HS_API:%S, Fname = %s, ret = %d\r\n",__FUNCTION__, pszFuncName, errNum));
  return(errNum);
}

/** 
 * @fn INT32  MIC_AGHSClose(HANDLE Handle)
 *  
 * @param    HANDLE Handle  - device HANDLE to close
 *
 * @return status of operation:
 *         AG_HS_OK - if Ok
 *         AG_HS_DEV_SET_ERROR - Failure
 *
 * @brief Close Bluetooth Headset device
 *    
 */  

INT32  MIC_AGHSClose(HANDLE Handle)
{
  INT32                      iRet = AG_HS_OK;
  MIC_DEVICE_CONTEXT_AG_HS   *pContext;
  DWORD                      AgServiceStatus;

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S, pContext = 0x%X\r\n",__FUNCTION__, Handle));

  if( Handle == NULL )
  {
   RETAILMSG(1, (L"AG_HS_API:%S, Handle == NULL\r\n",__FUNCTION__));
   return(AG_HS_SERVICE_INVALID_PARAM);
  }

  pContext = (MIC_DEVICE_CONTEXT_AG_HS *)Handle;

  EnterCriticalSection(&pContext->cs);

   // Close Read msg queue
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Close Read msg queue\r\n",__FUNCTION__));

  if( pContext->hsQueue != NULL )
    CloseMsgQueue(pContext->hsQueue);

  CloseHandle(pContext->hsQueue);

  
  // Close BT device
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Close BT device\r\n",__FUNCTION__));
  iRet = pContext->pfnBthAGCloseDevice(pContext->pDevContext);
  if( iRet < 0 )
  {
   RETAILMSG(1, (L"AG_HS_API:%S, Close BT device failure, ret = %d\r\n",__FUNCTION__, iRet ));
  }

  // Close DLL Library
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Close DLL Library\r\n",__FUNCTION__));
  if( !FreeLibrary(pContext->BtAgSvcModule) )
  {
   RETAILMSG(1, (L"AG_HS_API:%S, FreeLibrary failure, BtAgSvcModule = 0x%X\r\n",__FUNCTION__, pContext->BtAgSvcModule ));
  }

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Free Context\r\n",__FUNCTION__));
  if( LocalFree(pContext) )
  {
   RETAILMSG(1, (L"AG_HS_API:%S, LocalFree failure, pContext = 0x%X\r\n",__FUNCTION__, pContext ));
  }

  LeaveCriticalSection(&pContext->cs);

#if 0
  // Stop AG Service
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Stop AG Service\r\n",__FUNCTION__));
  if( iRet == 0 )
  {
   iRet = MIC_AGHSControl(AG_SERVICE_STOP, &AgServiceStatus);
  }
#endif

  DeleteCriticalSection(&pContext->cs);

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S\r\n",__FUNCTION__));

  return(iRet);
}



/** 
 * @fn INT32 MIC_AGHSControl(AGHSCmd_e cmd, VOID *pParam)
 *  
 * @param   AGHSCmd_e cmd - Headset AudioGateway Control command
 * @param   VOID *pParam  - Headset AudioGateway command's parameter
 *
 *  Command list
 *  cmd                * param In/Out/Type/ N/A * description
 *  AG_CONNECT         *    N/A                 * Establish ALC (command) connecttion
 *  AG_AUDIO_ON        *    N/A                 * Establish SCO (Audio) connecttion
 *  AG_AUDIO_OFF       *    N/A                 * disconnect SCO (Audio)
 *  AG_DISCONNECT      *    N/A                 * disconnect ALC (command)
 *	AG_SERVICE_START   *    N/A                 *
 *	AG_SERVICE_STOP    *    N/A                 *
 *  AG_SERVICE_STATUS  *    In/DWORD            * see service.h
 *
 *  AG_GET_MIC_VOL     *    Out/USHORT          * retrieves the microphone volume of the headset or the hands-free device;
 *  AG_GET_SPEAKER_VOL *    Out/USHORT          * retrieves the speaker volume of the headset or the hands-free device;
 *
 *  AG_SET_MIC_VOL     *    In/USHORT           * sets the microphone volume of the headset or the hands-free device;
 *  AG_SET_SPEAKER_VOL *    In/USHORT           * sets the speaker volume
 *
 *  AG_GET_POWER_MODE  *    Out/BOOL            * get headset power-save mode  
 *  AG_SET_POWER_MODE  *    In/BOOL             * set headset power-save mode  
 *
 * @return status of operation:
 *         0 - if Ok
 *         AG_HS_SERVICE_INVALID_PARAM
 *         AG_HS_SERVICE_INVALID_CMD
 *         AG_HS_SERVICE_OPEN_ERROR
 *         AG_HS_SERVICE_CTRL_ERROR 
 *        
 * @brief Control Bluetooth Headset device
 *    
 */  

INT32 MIC_AGHSControl(AGHSCMD_e cmd, VOID *pParam)
{
 int      ret = AG_HS_OK;
 DWORD    ioctl, dwOutParam, dwInParam, nInBufferSize, nOutBufferSize, lpBytesReturned;
 DWORD    svcStatus;
 LPVOID   lpInBuffer, lpOutBuffer;

 RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S\r\n",__FUNCTION__)); 


 if( pParam == NULL )
 {
  ret = AG_HS_SERVICE_INVALID_PARAM;
  RETAILMSG(1, (L"AG_HS_API:-%S, ret = %d [%s]\r\n",__FUNCTION__, ret, agHsErros[ret] )); 
  return(ret);
 }

 lpInBuffer      = NULL; 
 lpOutBuffer     = NULL;
 dwOutParam      = 0;
 dwInParam       = 0;
 nInBufferSize   = 0;
 nOutBufferSize  = 0;
 lpBytesReturned = 0;

 // If AG Service was not yet started, it remains not fully initializaed.
 // It may cause problem, for example, if try stop it in this situation.
    HANDLE h = CreateFile(L"BAG0:",0,0,NULL,OPEN_EXISTING,0,NULL);

	 if(INVALID_HANDLE_VALUE == h) 
	  {
		ret = AG_HS_SERVICE_OPEN_ERROR;
		RETAILMSG(1, (L"AG_HS_API:%S, ret = %d [%s]\r\n",__FUNCTION__, ret, agHsErros[ret] )); 
		return(ret);
	  }
	 else
	  {
       RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Get SERVICE STATUS\r\n",__FUNCTION__));

	   ioctl = IOCTL_SERVICE_STATUS;
 	   lpOutBuffer =  (DWORD *)&svcStatus; // &dwParam;
	   nOutBufferSize = sizeof(DWORD);

		BOOL fStatus = DeviceIoControl(h,ioctl,lpInBuffer,nInBufferSize,lpOutBuffer,nOutBufferSize, &lpBytesReturned, NULL);
		 if (FALSE == fStatus) 
		  {
			//wprintf(L"Operation failed: %d.\n", GetLastError());
 		   ret = AG_HS_SERVICE_CTRL_ERROR;
		   RETAILMSG(1, (L"AG_HS_API:%S, ret = %d [%s]\r\n",__FUNCTION__, ret, agHsErros[ret] )); 
		   return(ret);
		  }
	  }

	 RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, SERVICE STATUS = %d\r\n",__FUNCTION__, *(DWORD *)lpOutBuffer));

	 if( svcStatus == SERVICE_STATE_UNINITIALIZED )
	 {
	  lpOutBuffer = NULL;
	  nOutBufferSize  = 0;

      RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, SERVICE_STATE_UNINITIALIZED\r\n",__FUNCTION__ ));        
      RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, START SERVICE\r\n",__FUNCTION__ ));

	  ioctl = IOCTL_SERVICE_START; 
	  BOOL fStatus = DeviceIoControl(h,ioctl,lpInBuffer,nInBufferSize,lpOutBuffer,nOutBufferSize, &lpBytesReturned, NULL);

	  // Let them to wake-up, it takes some time , anyway ti will be stucking!!!
	  Sleep(3);

	  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, STOP SERVICE\r\n",__FUNCTION__ ));
	  ioctl = IOCTL_SERVICE_STOP;
	  fStatus = DeviceIoControl(h,ioctl,lpInBuffer,nInBufferSize,lpOutBuffer,nOutBufferSize, &lpBytesReturned, NULL);
	  if( fStatus != FALSE )
       RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, SERVICE Was stop successfully\r\n",__FUNCTION__ ));
	  else
       RETAILMSG(1, (L"AG_HS_API:%S, SERVICE stop failed\r\n",__FUNCTION__ ));
	 }

	 lpOutBuffer = NULL;
	 nOutBufferSize  = 0;

     CloseHandle(h);


	     RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, cmd = %d\r\n",__FUNCTION__, cmd ));

         switch(cmd)
		 {
		  case AG_CONNECT:          ioctl = IOCTL_AG_OPEN_CONTROL; break;
		  case AG_AUDIO_ON:         ioctl = IOCTL_AG_OPEN_AUDIO; break;
		  case AG_AUDIO_OFF:        ioctl = IOCTL_AG_CLOSE_AUDIO; break;
		  case AG_DISCONNECT:       ioctl = IOCTL_AG_CLOSE_CONTROL; break;
		  case AG_SERVICE_START:
			  { 
			     if (!IsBluetoothStackActive()) 
                     ret = AG_HS_MODULE_OFF;

				  ioctl = IOCTL_SERVICE_START; 

			  } break;

		  case AG_SERVICE_STOP:     ioctl = IOCTL_SERVICE_STOP; break;
		  case AG_SERVICE_STATUS:   
			  {
				  ioctl = IOCTL_SERVICE_STATUS;
				  lpOutBuffer =  (DWORD *)pParam; // &dwParam;
				  nOutBufferSize = sizeof(DWORD);
			  } break;  // DWORD 

		  case AG_GET_MIC_VOL: 
			    {
				   ioctl = IOCTL_AG_GET_MIC_VOL; 
				   lpOutBuffer =  (USHORT *)pParam;
				   nOutBufferSize = sizeof(USHORT *);
				} break;  // USHORT

		  case AG_GET_SPEAKER_VOL:
				{
                    ioctl = IOCTL_AG_GET_SPEAKER_VOL;
					lpOutBuffer =  (USHORT *)pParam;
					nOutBufferSize = sizeof(USHORT *);
				} break; // USHORT

		  case AG_SET_MIC_VOL:      
			    { 
					ioctl = IOCTL_AG_SET_MIC_VOL;
					lpInBuffer =  (USHORT *)pParam;
					nInBufferSize   = sizeof(USHORT *);
				} break;     // USHORT

		  case AG_SET_SPEAKER_VOL:  
			    {
					ioctl = IOCTL_AG_SET_SPEAKER_VOL; 
					lpInBuffer =  (USHORT *)pParam;
					nInBufferSize   = sizeof(USHORT *);
				} break; // USHORT

		  case AG_GET_POWER_MODE:   
			  {
				   ioctl = IOCTL_AG_GET_POWER_MODE; 
				   lpOutBuffer =  (BOOL *)pParam;
				   nOutBufferSize = sizeof(BOOL *);
			  } break;  // BOOL  

		  case AG_SET_POWER_MODE:   
			  {
				  ioctl = IOCTL_AG_SET_POWER_MODE;
				  lpInBuffer =  (BOOL *)pParam;
				  nInBufferSize   = sizeof(BOOL *);
			  } break;  // BOOL

		  default: 
			  {
                ret = AG_HS_SERVICE_INVALID_CMD;
				//RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S , ret = %d\r\n",__FUNCTION__, ret));
				//return(ret);
			  }	
		 }

		 if( ret == AG_HS_OK )
		 {
			 HANDLE h = CreateFile(L"BAG0:",0,0,NULL,OPEN_EXISTING,0,NULL);
			 if(INVALID_HANDLE_VALUE == h) 
			  {
				ret = AG_HS_SERVICE_OPEN_ERROR;
				return(ret);
			  }
			 else
			  {
				BOOL fStatus = DeviceIoControl(h,ioctl,lpInBuffer,nInBufferSize,lpOutBuffer,nOutBufferSize, &lpBytesReturned, NULL);
				CloseHandle(h);
				 if (FALSE == fStatus) 
				  {
					  ret = AG_HS_SERVICE_CTRL_ERROR;
				  }
				 else 
				  {
					  ret = AG_HS_OK;
				  }
			  }
		 }

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, ret = %d [%s]\r\n",__FUNCTION__, ret, agHsErros[ret] )); 
  return(ret);
}


INT32 MIC_AGHSEventRegistering(HANDLE Handle, DWORD eventMask)
{
 int                        iRet = AG_HS_OK;
 MIC_DEVICE_CONTEXT_AG_HS   *pContext;


 RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S\r\n",__FUNCTION__)); 

 // Verify Handle
	 if( Handle == NULL )
	 {
	  iRet = AG_HS_SERVICE_INVALID_PARAM;
	  return(iRet);
	  RETAILMSG(1, (L"AG_HS_API:-%S, Invalid Handle ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet] )); 
	 }

 pContext = (MIC_DEVICE_CONTEXT_AG_HS *)Handle;

     // Verify invalid events, empty event is valid, means event de-registering
     if( ( eventMask != 0 ) && ( eventMask & (~EV_AG_ALL_MASK) ) )
	 {
      iRet = AG_HS_SERVICE_INVALID_PARAM;
	  RETAILMSG(1, (L"AG_HS_API:-%S, Invalid Event mask, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet] )); 
	  return(iRet);
	 }


  EnterCriticalSection(&pContext->cs);


  if( pContext->hsQueue == NULL )
  {
   iRet = AG_HS_MSG_CREATE_ERROR;
   LeaveCriticalSection(&pContext->cs);
   RETAILMSG(1, (L"AG_HS_API:%S, no message queue, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
   return(iRet);
  }


  //BOOL BthAGSetRegEvent(HANDLE pDevContext, DWORD eventMask)
  if( !pContext->pfnBthAGSetRegEvent(pContext->pDevContext, eventMask) )
   iRet = AG_HS_REG_EVENT_ERROR;

  LeaveCriticalSection(&pContext->cs);

  RETAILMSG(1, (L"AG_HS_API:-%S, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet] )); 
  return(iRet);
}



INT32  MIC_AGHSGetEvent(HANDLE Handle, DWORD *pEventMask)
{
 int                        iRet = AG_HS_OK;
 MIC_DEVICE_CONTEXT_AG_HS   *pContext;

// Verify Handle
	 if( Handle == NULL )
	 {
	  iRet = AG_HS_SERVICE_INVALID_PARAM;
  	  RETAILMSG(1, (L"AG_HS_API:-%S, Invalid Handle ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet] )); 
	  return(iRet);
	 }

  pContext = (MIC_DEVICE_CONTEXT_AG_HS *)Handle;

  EnterCriticalSection(&pContext->cs);

  if( pContext->hsQueue == NULL )
  {
   iRet = AG_HS_MSG_CREATE_ERROR;
   LeaveCriticalSection(&pContext->cs);
   RETAILMSG(1, (L"AG_HS_API:%S, no message queue, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
   return(iRet);
  }

  if( !pContext->pfnBthAGGetRegEvent(pContext->pDevContext, pEventMask) )
   iRet = AG_HS_REG_EVENT_ERROR;

  LeaveCriticalSection(&pContext->cs);

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, ret = %d [%s], event = 0x%X\r\n",__FUNCTION__, iRet, agHsErros[iRet], *pEventMask )); 
  return(iRet);

}



/** 
 * @fn       INT32  MIC_AGHSWaitForEvent(HANDLE Handle, DWORD *pEvent)
 *  
 * @param    HANDLE Handle  - device HANDLE to close
 *
 * @param    DWORD *pEvent - event returned
 *
 * @return status of operation:
 *         AG_HS_OK - if Ok
 *         AG_HS_DEV_SET_ERROR - Failure
 *
 * @brief Close Bluetooth Headset device
 *    
 */  

INT32  MIC_AGHSWaitForEvent(HANDLE Handle, AGHS_MSG *pAgEvent)
{
 int                        iRet = AG_HS_OK;
 MIC_DEVICE_CONTEXT_AG_HS   *pContext;
 DWORD                      dwRetVal, dwFlags;
 AGHS_MSG                   msgContext;
 DWORD                      dwNumberOfBytesRead;

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S\r\n",__FUNCTION__)); 

 // Verify Handle
	 if( ( Handle == NULL ) || (pAgEvent == NULL) )
	 {
	  iRet = AG_HS_SERVICE_INVALID_PARAM;
	  RETAILMSG(1, (L"AG_HS_API:-%S, Invalid Handle ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet] )); 
	  return(iRet);
	 }

  pContext = (MIC_DEVICE_CONTEXT_AG_HS *)Handle;

  EnterCriticalSection(&pContext->cs);

  if( pContext->hsQueue == NULL )
  {
   iRet = AG_HS_MSG_CREATE_ERROR;
   RETAILMSG(1, (L"AG_HS_API:%S, no message queue, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
   LeaveCriticalSection(&pContext->cs);
   return(iRet);
  }

  LeaveCriticalSection(&pContext->cs);
  

		 if(!ReadMsgQueue(pContext->hsQueue,
							&msgContext,
							sizeof(msgContext),
							&dwNumberOfBytesRead,
							INFINITE,
							&dwFlags)|| ( sizeof(msgContext) != dwNumberOfBytesRead ))
		 {
            dwRetVal = GetLastError();
			RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, ReadMsgQueue failed, ret = %d\r\n",__FUNCTION__, dwRetVal));

            if( dwRetVal == ERROR_INVALID_HANDLE )
			{ // Exiting process
			 iRet = AG_HS_DEV_CLOSED; // Legal return on Closing
			 RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Legal return on Closing, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
			}
			else
			{
		     iRet = AG_HS_MSG_READ_ERROR;
			 RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, read message failure, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet]));
             pAgEvent->hsEvent = 0;
		     pAgEvent->dwParam = 0; 
			}
		 }
		 else
		 {
          pAgEvent->hsEvent = msgContext.hsEvent;
		  pAgEvent->dwParam = msgContext.dwParam; 

		  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, read message Ok, event = 0x%X, param = 0x%X\r\n",__FUNCTION__, pAgEvent->hsEvent, pAgEvent->dwParam));
		 }

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, ret = %d [%s]\r\n",__FUNCTION__, iRet, agHsErros[iRet] )); 
  return(iRet);
}



////////////////////////////////////////////////////
static LONG  BthDelAgHSDevices(void)
{
  int     iNumDevices = 1;
  WCHAR   szAgSubKey[128];
  HKEY    hKey;
  LONG    ret = ERROR_SUCCESS;

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S\r\n",__FUNCTION__));

  if( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, 0, &hKey) )
  {
    RegDeleteValue(hKey, TEXT("NoHandsFree"));
	RegCloseKey(hKey); 
  }

  while( ret == ERROR_SUCCESS )
  {
    wsprintf(szAgSubKey, L"%s\\%d", RK_AUDIO_GATEWAY_DEVICES, iNumDevices);
	RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, del subkey: %s\r\n",__FUNCTION__, szAgSubKey));
	ret = RegDeleteKey(HKEY_LOCAL_MACHINE, szAgSubKey);
	if( ret == ERROR_SUCCESS )
	  iNumDevices++;
  }
  
  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, ret = %d\r\n",__FUNCTION__, iNumDevices-1));
  return(iNumDevices-1);
}


static BOOL BthSetAgHSDevice(BT_ADDR b)
{
  int     iNumDevices = 1;
  HKEY    hkAgSubKey;
  WCHAR   szAgSubKey[128];
  DWORD   dwDis;
  DWORD   dwData;
  GUID    g;
  BOOL    bRet = TRUE;

  RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:+%S\r\n",__FUNCTION__));

  // Need to Inform AG Service that we support HeadSet profile only
  // Set registry subkey 'NoHandsFree' in 'RK_AUDIO_GATEWAY' to
  // 1 - Disables the Hands-Free Profile
      if( ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, RK_AUDIO_GATEWAY, 0, NULL, 0, NULL, NULL, &hkAgSubKey, &dwDis) )
	  {
       dwData = 1;
       if( ERROR_SUCCESS != RegSetValueEx(hkAgSubKey, L"NoHandsFree", 0, REG_DWORD, (PBYTE)&dwData, sizeof(dwData)) )
	   {
        RegCloseKey(hkAgSubKey); 
		RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, RegSetValueEx Failure\r\n",__FUNCTION__));
	   }
	  }
	  else
	  {
       RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, RegCreateKeyEx Failure\r\n",__FUNCTION__));
	   return(FALSE);
	  }

	  RegCloseKey(hkAgSubKey); 

      //wsprintf(szAgSubKey, L"SOFTWARE\\Microsoft\\Bluetooth\\AudioGateway\\Devices\\%d", iNumDevices);
	  wsprintf(szAgSubKey, L"%s\\%d", RK_AUDIO_GATEWAY_DEVICES, iNumDevices);

	   if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, szAgSubKey, 0, NULL, 0, NULL, NULL, &hkAgSubKey, &dwDis)) 
	   {
		//BD_ADDR b;
		//b.NAP = GET_NAP(pbtDevice->b);
		//b.SAP = GET_SAP(pbtDevice->b);
		RegSetValueEx(hkAgSubKey, L"Address", 0, REG_BINARY, (PBYTE)&b, sizeof(b));
        g = HeadsetServiceClass_UUID;

		RegSetValueEx(hkAgSubKey, L"Service", 0, REG_BINARY, (PBYTE)&g, sizeof(GUID));
		RegCloseKey (hkAgSubKey);

		RegFlushKey (HKEY_LOCAL_MACHINE);

		RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:%S, Saving to registry btAddr %04x%08x\r\n",__FUNCTION__, GET_NAP(b), GET_SAP(b)));
		for( int i = 0; i < sizeof(GUID); i++)
		  RETAILMSG(1, (L"[%02X]", *((unsigned char *)&g +i) ));

		RETAILMSG(AG_HS_DBG_PRN, (L"\r\n")); 

	   }
	   else
        bRet = FALSE;

   RETAILMSG(AG_HS_DBG_PRN, (L"AG_HS_API:-%S, ret = %d\r\n",__FUNCTION__, bRet));
   return(TRUE);
}
