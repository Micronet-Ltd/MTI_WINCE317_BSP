//-----------------------------------------------------------------------------
// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  owr_api.c
//
//  This file contains ONE-WIRE component API code.
//
//  Created by Michael Streshinsky    2009
//-----------------------------------------------------------------------------

#include <windows.h>
#include <MicUserSdk.h>
#include <owr.h>

#define   OWR_API_DBG   0


//------------------------------------------------------------------------------
//
//  Type:  DEVICE_CONTEXT_AIN
//
//  This structure is used to store AIN device context
//

typedef struct 
{
    HANDLE              hDevice;
	OWR_DEVICE_TYPE     devType;
	DWORD               devContextSize;
	HANDLE              hrQueue;
	HANDLE              hEvent;
	CRITICAL_SECTION    cs;

} MIC_DEVICE_CONTEXT_OWR;

//------------------------------------------------------------------------------
//          local funcrions
//------------------------------------------------------------------------------
static BOOL checkHandle(HANDLE handle);
static BOOL checkInputContext(POWR_INPUT_CONTEXT pInputContext);

//------------------------------------------------------------------------------
// Function name	: MIC_OWROpen
// Description	    : This function returns handle for safe access to One-Wire. This handle 
//                    must be used for all One-Wire operations.  This API supports multiple  
//                    accesses. Each time when this function is called, it returns valid handle.
// Return type		: Pointer to valid handle if succeeded, INVALID_HANDLE_VALUE if failed.  
//------------------------------------------------------------------------------

HANDLE  MIC_OWROpen(VOID)
{
    HANDLE                  hDevice;
	OWR_GET_DEVICE_CONTEXT  deviceContext;
	MIC_DEVICE_CONTEXT_OWR  *pContext = NULL; 
	BOOL                    ret;
    MSGQUEUEOPTIONS         msgOptions;

    RETAILMSG(OWR_API_DBG, (L"+MIC_OWROpen\r\n"));

	hDevice = CreateFile(OWR_IDENTIFIER, GENERIC_READ | GENERIC_WRITE, 
		                           0, 0, OPEN_EXISTING,
							       FILE_ATTRIBUTE_NORMAL,
							       NULL);
   
	if( ( hDevice != NULL )&&( hDevice != INVALID_HANDLE_VALUE ) )
	{
		memset (&deviceContext,0,sizeof(deviceContext));

		ret = DeviceIoControl (hDevice, IOCTL_OWR_GET_DEVICE_CONTEXT, NULL, 0,
					    (LPVOID)&deviceContext, sizeof(deviceContext),0,NULL ); 

		if (!ret || deviceContext.opErrorCode != OWR_OK)
		{
			RETAILMSG(1,(TEXT("MIC_OWROpen: Getting queue name is failed or queue already exists \r\n")));
			MIC_OWRClose(hDevice);
			return INVALID_HANDLE_VALUE;
		}

		// Allocate memory for our handler...
		pContext = (MIC_DEVICE_CONTEXT_OWR *)LocalAlloc(LPTR, sizeof(MIC_DEVICE_CONTEXT_OWR));
		if (pContext == NULL)
		 {
          RETAILMSG(1,(TEXT("MIC_OWROpen: Couldn't allocate memory\r\n")));
		  CloseHandle(hDevice);
		  return(INVALID_HANDLE_VALUE);
		 }
			
		// Save device handle
		pContext->hDevice = hDevice;
		pContext->devContextSize = sizeof(MIC_DEVICE_CONTEXT_OWR);
		pContext->devType = deviceContext.owrDeviceType;

		memset(&msgOptions, 0, sizeof(msgOptions));
		msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
		msgOptions.dwFlags = 0;
		msgOptions.cbMaxMessage = sizeof(OWR_INPUT_CONTEXT);

		// create read queue
		msgOptions.bReadAccess = 1;

		pContext->hrQueue = CreateMsgQueue(deviceContext.owrQueueName, &msgOptions);
		if(pContext->hrQueue == NULL)
		{
			RETAILMSG(1,(TEXT("MIC_OWROpen: Couldn't create read msg queue \r\n")));
			MIC_OWRClose(hDevice);
			return INVALID_HANDLE_VALUE;
		}

		RETAILMSG(OWR_API_DBG, (L"MIC_OWROpen:msg queue created: %s [0x%08X]\r\n", deviceContext.owrQueueName, pContext->hrQueue));
       
		pContext->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if(pContext->hEvent == NULL)
		{
			RETAILMSG(1,(TEXT("MIC_OWROpen: Couldn't create event \r\n")));
			MIC_OWRClose(hDevice);
			return INVALID_HANDLE_VALUE;
		}

        InitializeCriticalSection(&pContext->cs);

	 RETAILMSG(OWR_API_DBG, (L"MIC_OWROpen: OK\r\n"));
	}
	else
	{
     RETAILMSG(1, (L"MIC_OWROpen: Fail, err = %d\r\n", GetLastError() ));
	 return INVALID_HANDLE_VALUE;
	}

	RETAILMSG(OWR_API_DBG, (L"-MIC_OWROpen\r\n"));

	return pContext;
}



//------------------------------------------------------------------------------
// Function name	: MIC_OWRClose
// Description	    : This function destroys the given handle.
// Return type		: TRUE if succeeded, FALSE if failed
// Argument         : handle – valid handle from XXX_OWROpen;
//------------------------------------------------------------------------------

BOOL  MIC_OWRClose(HANDLE handle)
{
	MIC_DEVICE_CONTEXT_OWR  *pContext;
	BOOL                     ret = TRUE;

	RETAILMSG(OWR_API_DBG, (L"+MIC_OWRClose\r\n"));

    if( !checkHandle(handle) )
	 {
	  RETAILMSG(1,(TEXT("MIC_OWRClose: handle\r\n")));
	  //SetLastError(ERROR_INVALID_PARAMETER);
	  return(FALSE);
	 }

	pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 
	
    // we need to signal MIC_OneWireWaitForEvent function
	SetEvent(pContext->hEvent);
	// to give possibility WaitForStateChange to be signalled.
	Sleep(0);

	if(pContext->hrQueue)
	{
		if( !CloseMsgQueue(pContext->hrQueue) )
		{
         ret = FALSE;
	     RETAILMSG(1, (L"MIC_OWRClose: CloseMsgQueu err = %d\r\n", GetLastError() ));
		}

		pContext->hrQueue = NULL;
	}

	if(pContext->hEvent)
	{
		if( !CloseHandle(pContext->hEvent) )
		{
         ret = FALSE;
	     RETAILMSG(1, (L"MIC_OWRClose: CloseHandle for Event err = %d\r\n", GetLastError() ));
		}

		pContext->hEvent = NULL;
	}

	DeleteCriticalSection(&pContext->cs);

	if( !CloseHandle(pContext->hDevice) )
	{
     ret = FALSE;
	 RETAILMSG(1, (L"MIC_OWRClose: CloseHandle for Device err = %d\r\n", GetLastError() ));
	}

	if( LocalFree(pContext) != NULL )
	{
     ret = FALSE;
     RETAILMSG(1, (L"MIC_OWRClose: LocalFree err = %d\r\n", GetLastError() )); 
	}

	RETAILMSG(OWR_API_DBG, (L"-MIC_OWRClose\r\n"));

	return(ret);
}




//------------------------------------------------------------------------------
// Function name	: MIC_OneWireRead
// Description	    : Read 1-Wire data. 
//                   In case of iButton, it reads its ROM (8-byte) value

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_OWROpen;
//                   : context  is of POWR_INPUT_CONTEXT type 
//                   : IN: oneWireDatalength ( sizeof(ONEWIRE_ROM_ID_T) )
//                   : OUT: opErrorCode. Possible return codes for this function:
//
//                      OWR_OK 
//                      OWR_ERROR_INVALID_PARAMETER
//                      OWR_ERROR_INVALID_DEVICE
//                      OWR_ERROR_INVALID_IO_PARAMETER
//                      OWR_ERROR_READ_FAULT
//                      OWR_ERROR_ACCESS_DENIED
//------------------------------------------------------------------------------

BOOL  MIC_OneWireRead(HANDLE handle, POWR_INPUT_CONTEXT pInputContext, LPVOID pOneWireData )
{
	MIC_DEVICE_CONTEXT_OWR  *pContext;
	BOOL                     ret = FALSE;

	RETAILMSG(OWR_API_DBG, (L"+MIC_OneWireRead\r\n"));

    if( !checkHandle(handle) )
	 {
		RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid handle\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	 }

	if( !checkInputContext(pInputContext) )
     {
		RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid InputContext\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
     }

	 pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 

	  // Check pOneWireData, it depends on 1-Wire device type
	  switch(pContext->devType)
	  {
	    case i_BUTTON:
		{
          OWR_IBUTTON_CONTEXT  iButtonContext;
          ONEWIRE_ROM_ID_T     *piButtonROM;

		  // Check pOneWireData
		  if( IsBadReadPtr(pOneWireData, sizeof(LPVOID) ) )
		  {
            pInputContext->opErrorCode = OWR_ERROR_INVALID_PARAMETER;
			RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid 3rd Parameter as pointer!\r\n")));
			//SetLastError(ERROR_INVALID_PARAMETER);
			return(FALSE);
		  }

		  if( pInputContext->oneWireDatalength != sizeof(ONEWIRE_ROM_ID_T) )
		  {
            pInputContext->opErrorCode = OWR_ERROR_INVALID_PARAMETER;
			RETAILMSG(1,(TEXT("MIC_OneWireRead: oneWireDatalength size error\r\n")));
			//SetLastError(ERROR_INVALID_PARAMETER);
			return(FALSE);
		  }

		  // Read iButton ROM
			ret = DeviceIoControl( pContext->hDevice, IOCTL_OUTPUT_IBUTTON_READ, NULL, 0,
							       (LPVOID)&iButtonContext, sizeof(OWR_IBUTTON_CONTEXT),0,NULL ); 

			if (!ret || iButtonContext.opErrorCode != OWR_OK)
			{
				RETAILMSG(1,(TEXT("MIC_OneWireRead: iButton read failure, err = %d\r\n"), iButtonContext.opErrorCode ));
				pInputContext->opErrorCode = iButtonContext.opErrorCode;
				return(FALSE);
			}

			piButtonROM = (ONEWIRE_ROM_ID_T  *)pOneWireData;
			piButtonROM->familyCode = iButtonContext.romValue[0];
			memcpy(piButtonROM->SerialNumber, &iButtonContext.romValue[1], 6);
			piButtonROM->CRC = iButtonContext.romValue[7];

			pInputContext->opErrorCode = OWR_OK;

			ret = TRUE;

		} break; 

	    case TEMPR_SENS1:
		{
          pInputContext->opErrorCode = OWR_ERROR_INVALID_DEVICE;
		  RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid Device # = %d\r\n"), pContext->devType ));
		} break; 

		default:
		{
          RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid Device # = %d\r\n"), pContext->devType ));
          pInputContext->opErrorCode = OWR_ERROR_INVALID_DEVICE;
		} break; 

	  }

   RETAILMSG(OWR_API_DBG, (L"-MIC_OneWireRead\r\n"));

   return(ret);
}



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireReadASCII
// Description	    : Read 1-Wire data. 
//                   In case of iButton, it reads its ROM (8-byte) value
//                   This function read iButton number as strings, see structure ' ONEWIRE_ROM_ID_ASCII_T'

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_OWROpen;
//                   : context  is of POWR_INPUT_CONTEXT type 
//                   : IN: oneWireDatalength ( sizeof(ONEWIRE_ROM_ID_ASCII_T) )
//                   : OUT: opErrorCode. Possible return codes for this function:
//
//                      OWR_OK 
//                      OWR_ERROR_INVALID_PARAMETER
//                      OWR_ERROR_INVALID_DEVICE
//                      OWR_ERROR_INVALID_IO_PARAMETER
//                      OWR_ERROR_READ_FAULT
//                      OWR_ERROR_ACCESS_DENIED
//------------------------------------------------------------------------------

BOOL  MIC_OneWireReadASCII(HANDLE handle, POWR_INPUT_CONTEXT pInputContext, LPVOID pOneWireData )
{
	MIC_DEVICE_CONTEXT_OWR  *pContext;
	BOOL                     ret = FALSE;

	RETAILMSG(OWR_API_DBG, (L"+MIC_OneWireRead\r\n"));

    if( !checkHandle(handle) )
	 {
		RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid handle\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	 }

	if( !checkInputContext(pInputContext) )
     {
		RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid InputContext\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
     }

	 pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 

	  // Check pOneWireData, it depends on 1-Wire device type
	  switch(pContext->devType)
	  {
	    case i_BUTTON:
		{
          OWR_IBUTTON_CONTEXT      iButtonContext;
          ONEWIRE_ROM_ID_ASCII_T   *piButtonROM;

		  // Check pOneWireData
		  if( IsBadReadPtr(pOneWireData, sizeof(LPVOID) ) )
		  {
            pInputContext->opErrorCode = OWR_ERROR_INVALID_PARAMETER;
			RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid 3rd Parameter as pointer!\r\n")));
			//SetLastError(ERROR_INVALID_PARAMETER);
			return(FALSE);
		  }

		  if( pInputContext->oneWireDatalength != sizeof(ONEWIRE_ROM_ID_ASCII_T) )
		  {
            pInputContext->opErrorCode = OWR_ERROR_INVALID_PARAMETER;
			RETAILMSG(1,(TEXT("MIC_OneWireRead: oneWireDatalength size error\r\n")));
			//SetLastError(ERROR_INVALID_PARAMETER);
			return(FALSE);
		  }

		  // Read iButton ROM
			ret = DeviceIoControl( pContext->hDevice, IOCTL_OUTPUT_IBUTTON_READ, NULL, 0,
							       (LPVOID)&iButtonContext, sizeof(OWR_IBUTTON_CONTEXT),0,NULL ); 

			if (!ret || iButtonContext.opErrorCode != OWR_OK)
			{
				RETAILMSG(1,(TEXT("MIC_OneWireRead: iButton read failure, err = %d\r\n"), iButtonContext.opErrorCode ));
				pInputContext->opErrorCode = iButtonContext.opErrorCode;
				return(FALSE);
			}

			piButtonROM = (ONEWIRE_ROM_ID_ASCII_T  *)pOneWireData;
			sprintf( (char *)piButtonROM->familyCode, "%02X", iButtonContext.romValue[0]); 
			//piButtonROM->familyCode = iButtonContext.romValue[0];
			//memcpy(piButtonROM->SerialNumber, &iButtonContext.romValue[1], 6);
			sprintf( (char *)piButtonROM->SerialNumber, "%02X%02X%02X%02X%02X%02X", 
				                                 iButtonContext.romValue[6],
												 iButtonContext.romValue[5],
												 iButtonContext.romValue[4],
												 iButtonContext.romValue[3],
												 iButtonContext.romValue[2],
												 iButtonContext.romValue[1]
			       ); 

			//piButtonROM->CRC = iButtonContext.romValue[7];
			sprintf( (char *)piButtonROM->CRC, "%02X", iButtonContext.romValue[7]); 

			pInputContext->opErrorCode = OWR_OK;

			ret = TRUE;

		} break; 

#if 0
	    case TEMPR_SENS1:
		{
          pInputContext->opErrorCode = OWR_ERROR_INVALID_DEVICE;
		  RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid Device # = %d\r\n"), pContext->devType ));
		} break; 
#endif
		default:
		{
          RETAILMSG(1,(TEXT("MIC_OneWireRead: Invalid Device # = %d\r\n"), pContext->devType ));
          pInputContext->opErrorCode = OWR_ERROR_INVALID_DEVICE;
		} break; 

	  }

   RETAILMSG(OWR_API_DBG, (L"-MIC_OneWireRead\r\n"));

   return(ret);

}



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireEventRegistering
// Description	    : This function permit event notification for process with given handle.. 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

//Parameters:         handle – valid handle from MIC_OneWireOpen;
//                    pInputContext – pointer to ONE_WIRE_INPUT_CONTEXT  structure, which filled the 'size' by user. Other fields are not relevant here.
//                    BOOL needToRegister – TRUE for Register, FALSE - unregister
//                    'opErrorCode' member filled by Driver with appropriate error code if error occur.
//                    Possible return codes for this function:

//                     OWR_OK
//                     OWR_ERROR_ALREDY_REGISTERED
//                     OWR_ERROR_INVALID_ACCESS
//                     OWR_ERROR_ACCESS_DENIED
//------------------------------------------------------------------------------

BOOL  MIC_OneWireEventRegistering(HANDLE handle, 
								  POWR_INPUT_CONTEXT pInputContext, 
                                  BOOL needToRegister)
{
	MIC_DEVICE_CONTEXT_OWR  *pContext;
	BOOL                     ret = FALSE;
	DWORD                    errorCode;

	RETAILMSG(OWR_API_DBG, (L"+MIC_OneWireEventRegistering\r\n"));

    if( !checkHandle(handle) )
	 {
		RETAILMSG(1,(TEXT("MIC_OneWireEventRegistering: Invalid handle\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	 }

	if( !checkInputContext(pInputContext) )
     {
		RETAILMSG(1,(TEXT("MIC_OneWireEventRegistering: Invalid InputContext\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
     }

	 pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 

		ret = DeviceIoControl( pContext->hDevice, IOCTL_INPUT_REGISTER_NOTIFY, (LPVOID)&needToRegister, sizeof(BOOL),
						       (LPVOID)&errorCode, sizeof(DWORD),0,NULL ); 

		if (!ret || errorCode != OWR_OK)
		{
			RETAILMSG(1,(TEXT("MIC_OneWireEventRegistering: failure, err = %d\r\n"), errorCode ));
			pInputContext->opErrorCode = errorCode;
			return(FALSE);
		}
		else
		{
          ret = TRUE;
		  pInputContext->opErrorCode = OWR_OK;
		}


    RETAILMSG(OWR_API_DBG, (L"-MIC_OneWireEventRegistering\r\n"));
    return(ret);

}


//------------------------------------------------------------------------------
// Function name	: MIC_OneWirePrivateAccess
// Description	    : This function grants/release exclusive access for process with such 
//                    handle. Once successfully granted, other processes cannot to access 
//                    One-Wire devices.

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

//Parameters:         handle        –      valid handle from MIC_OneWireOpen;
//                    pInputContext –      pointer to ONE_WIRE_INPUT_CONTEXT  tructure, 
//                                         which filled the 'size' by User,  'oneWireEventType' and  
//                    'oneWireDatalength'  members are not relevant here.
//                    BOOL privateAccess – TRUE for set, FALSE – for release
//                    'opErrorCode'        member filled by Driver with appropriate error code if error occur.
//                                         Possible return codes for this function:

//                                          OWR_OK
//                                          OWR_ERROR_ACCESS_DENIED
//                                          OWR_ERROR_INVALID_ACCESS
//------------------------------------------------------------------------------

BOOL  MIC_OneWirePrivateAccess( HANDLE handle, 
                                POWR_INPUT_CONTEXT pInputContext,
                                BOOL privateAccess)
{
	MIC_DEVICE_CONTEXT_OWR  *pContext;
	BOOL                     ret = FALSE;
	DWORD                    errorCode;

	RETAILMSG(OWR_API_DBG, (L"+MIC_OneWirePrivateAccess\r\n"));

    if( !checkHandle(handle) )
	 {
		RETAILMSG(1,(TEXT("MIC_OneWirePrivateAccess: Invalid handle\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	 }

	if( !checkInputContext(pInputContext) )
     {
		RETAILMSG(1,(TEXT("MIC_OneWirePrivateAccess: Invalid InputContext\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
     }

	pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 

	ret = DeviceIoControl( pContext->hDevice, IOCTL_INPUT_SET_ACCESS, (LPVOID)&privateAccess, sizeof(BOOL),
					       (LPVOID)&errorCode, sizeof(DWORD),0,NULL ); 

	if (!ret || errorCode != OWR_OK)
	{
		RETAILMSG(1,(TEXT("MIC_OneWirePrivateAccess: failure, err = %d\r\n"), errorCode ));
		pInputContext->opErrorCode = errorCode;
		return(FALSE);
	}
	else
	{
      ret = TRUE;
	  pInputContext->opErrorCode = OWR_OK;
	}

  RETAILMSG(OWR_API_DBG, (L"-MIC_OneWirePrivateAccess\r\n"));

  return(ret);
}



//------------------------------------------------------------------------------
// Function name	: MIC_OneWireWaitForEvent
// Description	    : This function grants/release exclusive access for process with such 
//                    handle. Once successfully granted, other processes cannot to access 
//                    One-Wire devices.

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

//Parameters:         handle        – valid handle from MIC_OneWireOpen;
//                    pInputContext – pointer to ONE_WIRE_INPUT_CONTEXT  tructure, 
//                                    which filled the 'size' by User,  'oneWireEventType' member 
//                                    returned by Driver. 'oneWireDatalength' is not relevant here.
//                    'opErrorCode'    member filled by Driver with appropriate error   
//                                         Possible return codes for this function:

//                                         OWR_OK
//                                         OWR_ERROR_CLOSED_HANDLE
//                                         OWR_ERROR_NOTIFICATION
//------------------------------------------------------------------------------

BOOL  MIC_OneWireWaitForEvent( HANDLE handle, POWR_INPUT_CONTEXT pInputContext)
{
	MIC_DEVICE_CONTEXT_OWR  *pContext;
	BOOL                     ret = FALSE;
	//DWORD                    errorCode;
	OWR_INPUT_CONTEXT        msgContext;
	DWORD                    dwNumberOfBytesRead,dwFlags;
	HANDLE                   hEvents[2]; //= {pNotifyContext->pinInfo[bitNo].hEvent,pNotifyContext->pinInfo[bitNo].hInQueue};

	RETAILMSG(OWR_API_DBG, (L"+MIC_OneWireWaitForEvent\r\n"));

    if( !checkHandle(handle) )
	 {
		RETAILMSG(1,(TEXT("MIC_OneWireWaitForEvent: Invalid handle\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	 }

	if( !checkInputContext(pInputContext) )
     {
		RETAILMSG(1,(TEXT("MIC_OneWireWaitForEvent: Invalid InputContext\r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
     }

	pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 
	hEvents[0] = pContext->hEvent;
	hEvents[1] = pContext->hrQueue;

	DWORD dwRetVal = WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]), hEvents, FALSE, INFINITE);

	switch(dwRetVal)
	{
	  case WAIT_OBJECT_0:
	   {
         // Notify by MIC_OWRClose
		   pInputContext->opErrorCode = OWR_ERROR_CLOSED_HANDLE;
		   //SetLastError(OWR_ERROR_CLOSED_HANDLE);
	   } break;

	  case WAIT_OBJECT_0 + 1:
	   {

		 //	Okay, we have notification..  
		 if(!ReadMsgQueue(pContext->hrQueue,
							&msgContext,
							sizeof(msgContext),
							&dwNumberOfBytesRead,
							1,
							&dwFlags)|| ( sizeof(msgContext) != dwNumberOfBytesRead ))
		 {

		
			RETAILMSG(1, (L"MIC_OneWireWaitForEvent: ReadMsgQueue failed %d\r\n",GetLastError()));
			pInputContext->opErrorCode = OWR_ERROR_NOTIFICATION ;
		 }
		 else
		 {
          pInputContext->oneWireEventType = msgContext.oneWireEventType;
		  pInputContext->opErrorCode = msgContext.opErrorCode;
		  ret = TRUE;
		  RETAILMSG(OWR_API_DBG, (L"MIC_OneWireWaitForEvent: ReadMsgQueue [0x%08X] OK\r\n", pContext->hrQueue));
		 }

	   } break;

	  default: pInputContext->opErrorCode = OWR_ERROR_NOTIFICATION ; break;
	}

  RETAILMSG(OWR_API_DBG, (L"-MIC_OneWireWaitForEvent\r\n"));

  return(ret);
}

//------------------------------------------------------------------------------
// Function name	: MIC_OWROut
// Description	    : Sets pin state according to the chosen mode. 
//                   In free mode (default) everyone is allowed to use this function. 
//                   In accumulated mode the output is a logical OR of all the handle states. 
//                   In lock mode, the only "lock" caller handler is allowed to use this function. 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_OWROpen;
//                   : context  is of POWR_OUTPUT_CONTEXT type 
//                   : IN: Size,pinNo and state. 
//                   : OUT: opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_OWROut(HANDLE handle)
{
	BOOL ret = FALSE;
    DWORD                     inBuf, outBuf, outSize;
	//POWR_OUTPUT_CONTEXT pOutputContext = (POWR_OUTPUT_CONTEXT) context;

	if(!handle)
	{
		RETAILMSG(1,(TEXT("MIC_OWRSetOutputState: Invalid Parameter \r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{

	    RETAILMSG(OWR_API_DBG,(TEXT("MIC_OWROut: OK\r\n")));
		ret = DeviceIoControl ( handle, IOCTL_OUTPUT_OW,(VOID*)&inBuf, sizeof(inBuf), 
		                        (VOID*)&outBuf, sizeof(outBuf), &outSize, NULL); 
		
	}

	return ret;
}


//------------------------------------------------------------------------------
// Function name	: MIC_OWRTest
// Description	    : 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_OWROpen;
//                   : context  is of POWR_OUTPUT_CONTEXT type 
//                   : IN: Size,pinNo and state. 
//                   : OUT: opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_OWRTest(HANDLE handle, UINT32 testNum, void *pOut)
{
	BOOL ret = FALSE;
    DWORD                     inBuf, outBuf, outSize;
	//POWR_OUTPUT_CONTEXT pOutputContext = (POWR_OUTPUT_CONTEXT) context;

	if(!handle)
	{
		RETAILMSG(1,(TEXT("MIC_OWRTes: Invalid Parameter \r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
        switch(testNum)
		{
		  case 1: 
			  {
		       inBuf = testNum;
			   RETAILMSG(OWR_API_DBG,(TEXT("MIC_OWRTest #1\r\n")));
		       ret = DeviceIoControl( handle, IOCTL_OUTPUT_OW_TEST,(VOID*)&inBuf, sizeof(inBuf), 
		                               NULL, 0, NULL, NULL); 

			  } break;

		  default: 
			  {
               RETAILMSG(1,(TEXT("MIC_OWRTest - Unknown test\r\n")));
			  }break;
		}

    /* ret = DeviceIoControl ( handle, IOCTL_OUTPUT_OW_TEST,(VOID*)&inBuf, sizeof(inBuf), 
	                        (VOID*)&outBuf, sizeof(outBuf), &outSize, NULL); 
							*/

	}

	return ret;
}



static BOOL checkHandle(HANDLE handle)
{
  MIC_DEVICE_CONTEXT_OWR  *pContext;

	  if( ( (DWORD)handle % (sizeof(HANDLE)) ) != 0 )
	  {
		RETAILMSG(1,(TEXT("checkHandle: Invalid alignment! handle = 0x%08X\r\n"), handle));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	  }

	  pContext = (MIC_DEVICE_CONTEXT_OWR *)handle; 

	  // Check Hanlde
	  if( IsBadReadPtr(pContext, sizeof(MIC_DEVICE_CONTEXT_OWR *) ) )
	  {
		RETAILMSG(1,(TEXT("checkHandle: bad pointer, handle = 0x%08X\r\n"), handle));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	  }

	  if( pContext->devContextSize != sizeof(MIC_DEVICE_CONTEXT_OWR) )
	  {
		RETAILMSG(1,(TEXT("checkHandle: bad szie = %d\r\n"), pContext->devContextSize));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	  }

  return(TRUE);
}

static BOOL checkInputContext(POWR_INPUT_CONTEXT pInputContext)
{
	  // Check pInputContext
	  if( IsBadReadPtr(pInputContext, sizeof(POWR_INPUT_CONTEXT) ) )
	  {
		RETAILMSG(1,(TEXT("checkInputContext: bad pointer is 0x%08X\r\n"), pInputContext));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	  }

	  if( pInputContext->size != sizeof(OWR_INPUT_CONTEXT) )
	  {
        RETAILMSG(1,(TEXT("checkInputContext: invalid size is %d\r\n"), pInputContext->size)); 
		//RETAILMSG(OWR_API_DBG,(TEXT("MIC_OneWireRead: Invalid 2nd Parameter \r\n")));
		//SetLastError(ERROR_INVALID_PARAMETER);
		return(FALSE);
	  }

   return(TRUE);
}
