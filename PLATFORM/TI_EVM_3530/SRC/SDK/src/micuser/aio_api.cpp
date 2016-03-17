//-----------------------------------------------------------------------------
// Copyright 2009 Micronet Ltd. All Rights Reserved.
//
//------------------------------------------------------------------------------
//
//  File:  aio_api.c
//
//  This file contains Automotive Input/Output component API code.
//
//  Created by Anna Rayer    2009
//-----------------------------------------------------------------------------

#include <windows.h>
#include <MicUserSdk.h>
#include <aio.h>

//------------------------------------------------------------------------------
//          local funcrions
//------------------------------------------------------------------------------
static BOOL  RegisterForStateChange  (HANDLE handle, PAIO_NOTIFY_CONTEXT pNotifyContext,PAIO_INPUT_CONTEXT pInputContext);
static BOOL  DeregisterForStateChange(HANDLE handle, PAIO_NOTIFY_CONTEXT pNotifyContext,PAIO_INPUT_CONTEXT pInputContext);
static DWORD BitNo( DWORD mask);


//------------------------------------------------------------------------------
// Function name	: MIC_AIOOpen
// Description	    : This function returns handle for safe access to automotive pins. 
//                    This handle must be used for all automotive pins’ operations. 
// Return type		: Pointer to valid handle if succeeded, INVALID_HANDLE_VALUE if failed. 
// Argument         : pOpenContext – This parameter is NULL (currently) but may be 
//                    needed for future use (We don't want to change API function 
//                     prototype in the case of requirements changes).
//------------------------------------------------------------------------------
HANDLE  MIC_AIOOpen(LPVOID pContext)
{
    HANDLE hDevice;
	PAIO_NOTIFY_CONTEXT pNotifyContext = (PAIO_NOTIFY_CONTEXT)pContext;
    HANDLE              rc = INVALID_HANDLE_VALUE;

	if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH)
	{
		RETAILMSG (1,(TEXT("MIC_AIOOpen: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		return INVALID_HANDLE_VALUE;
	}


	hDevice = CreateFile(AIO_IDENTIFIER, GENERIC_READ | GENERIC_WRITE, 
		                           0, 0, OPEN_EXISTING,
							       FILE_ATTRIBUTE_NORMAL,
							       NULL);
   
	if(hDevice)
	{
		AIO_GET_NAME_CONTEXT    nameContext;
		DWORD                   ret = FALSE;
		MSGQUEUEOPTIONS         msgOptions;
		AUT_PINS                pinNo;
		DWORD                   inputPinsNo;

		
		memset(pNotifyContext->pinInfo,0,(sizeof(pNotifyContext->pinInfo)));

		ret = DeviceIoControl (hDevice, IOCTL_INPUT_GET_PINS_NO,NULL, NULL,(LPVOID)&inputPinsNo, sizeof(DWORD),0,NULL ); 


		if (!ret)
		{
			RETAILMSG (1,(TEXT("MIC_AIOOpen: Getting input pins no is failed\r\n")));
			MIC_AIOClose(hDevice,pNotifyContext);
			return INVALID_HANDLE_VALUE;
		} 
		RETAILMSG(0, (L"API: AIOOpen:  inputPinsNo - %d \r\n", inputPinsNo));

		for(DWORD i = 0;i< inputPinsNo;i++)
		{

			memset (&nameContext,0,sizeof(nameContext));

			pinNo.mask = (0x01<<i);

			ret = DeviceIoControl (hDevice, IOCTL_ATSR_GETQUEUENAME,(LPVOID)&pinNo, sizeof (AUT_PINS),
						    (LPVOID)&nameContext, sizeof(nameContext),0,NULL ); 


			if (!ret || nameContext.opErrorCode!=AIO_OK)
			{
				RETAILMSG (1,(TEXT("MIC_AIOOpen: Getting queue name is failed or queue already exists \r\n")));
				MIC_AIOClose(hDevice,pNotifyContext);
				return INVALID_HANDLE_VALUE;
			}

			RETAILMSG(0, (L"API: AIOOpen:  da: %s\r\n", nameContext.daQueueName));
	
			memset(&msgOptions, 0, sizeof(msgOptions));
			msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
			msgOptions.dwFlags = 0;
			msgOptions.cbMaxMessage = sizeof(AIO_INPUT_CONTEXT);

			// create read queue
			msgOptions.bReadAccess = 1;

			pNotifyContext->pinInfo[i].hInQueue = CreateMsgQueue(nameContext.daQueueName, &msgOptions);
			if(pNotifyContext->pinInfo[i].hInQueue == NULL)
			{
				RETAILMSG (1,(TEXT("MIC_AIOOpen: Couldn't create read msg queue \r\n")));
				MIC_AIOClose(hDevice,pNotifyContext);
				return INVALID_HANDLE_VALUE;
			}

#if 0
			// create write queue
			msgOptions.bReadAccess = 0;

			pNotifyContext->pinInfo[i].hOutQueue = CreateMsgQueue(nameContext.adQueueName, &msgOptions);
			if(pNotifyContext->pinInfo[i].hOutQueue == NULL)
			{
				RETAILMSG (1,(TEXT("MIC_AIOOpen: Couldn't create write msg queue \r\n")));
				MIC_AIOClose(hDevice,pNotifyContext);
				return INVALID_HANDLE_VALUE;
			}
#endif
			pNotifyContext->pinInfo[i].hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			if(pNotifyContext->pinInfo[i].hEvent == NULL)
			{
				RETAILMSG (1,(TEXT("MIC_AIOOpen: Couldn't create event \r\n")));
				MIC_AIOClose(hDevice,pNotifyContext);
				return INVALID_HANDLE_VALUE;
			}

			InitializeCriticalSection(&(pNotifyContext->pinInfo[i].cs));

			
			pNotifyContext->pinInfo[i].state = OPENED_STATE;
				
		}
	}

	rc = hDevice;

    return rc;
}



//------------------------------------------------------------------------------
// Function name	: MIC_AIOClose
// Description	    : This function destroys the given handle.
// Return type		: 0 if succeeded, error number if failed 
// Argument         : handle – valid handle from XXX_AIOOpen;
//                    pOpenContext  is a context getting from MIC_AIOOpen

//------------------------------------------------------------------------------
DWORD  MIC_AIOClose(HANDLE handle,LPVOID pContext)
{
	PAIO_NOTIFY_CONTEXT pNotifyContext = (PAIO_NOTIFY_CONTEXT)pContext;
	BOOL                ret = FALSE;
	AUT_PINS            pinNo;
	DWORD               error;
	DWORD               inputPinsNo;

	if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH)
	{
		RETAILMSG (1,(TEXT("MIC_AIOClose: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		return AIO_ERROR_INVALID_PARAMETER;
	}

	ret = DeviceIoControl (handle, IOCTL_INPUT_GET_PINS_NO,NULL, NULL,(LPVOID)&inputPinsNo, sizeof(DWORD),0,NULL ); 


	if (!ret)
	{
		RETAILMSG (1,(TEXT("MIC_AIOClose: Getting input pins no is failed\r\n")));
		return AIO_INTERNAL_ERROR;
	} 

	RETAILMSG(0, (L"API: AIOClose:  inputPinsNo - %d \r\n", inputPinsNo));

	
	for(int i = 0;i< inputPinsNo;i++)
	{
		if(pNotifyContext->pinInfo[i].hInQueue && pNotifyContext->pinInfo[i].hEvent)
		{

			//
			// we need to signal WaitForStateChange function
			if(pNotifyContext->pinInfo[i].state & WAITING_FOR_EVENT)
			{
				SetEvent(pNotifyContext->pinInfo[i].hEvent);
				// to give possibility WaitForStateChange to be signalled.
				Sleep(0);
			}

			pinNo.mask = (0x01 << i);
				// if handle is in Notify state, then send Stop Notify cmd
		
			if(pNotifyContext->pinInfo[i].state & START_NOTIFY_STATE)
			{
				ret = DeviceIoControl ( handle, IOCTL_INPUT_STOP_CHANGE_EVENT,(LPVOID)&pinNo, sizeof (AUT_PINS),
						    (LPVOID)&error, sizeof(DWORD),0,NULL ); 
				if(ret)
				{
					EnterCriticalSection(&(pNotifyContext->pinInfo[i].cs));
					pNotifyContext->pinInfo[i].state &= ~START_NOTIFY_STATE;
					LeaveCriticalSection(&(pNotifyContext->pinInfo[i].cs));
				}
			}

				// if handle is in REGISTERED state, then send DEREGISTER cmd
			if(pNotifyContext->pinInfo[i].state & REGISTERED_STATE)
			{
				ret = DeviceIoControl ( handle, IOCTL_INPUT_DEREGISTER_STATE_CHANGE,(LPVOID)&pinNo, sizeof (AUT_PINS),
						    (LPVOID)&error, sizeof(DWORD),0,NULL ); 
				if(ret)
				{
					EnterCriticalSection(&(pNotifyContext->pinInfo[i].cs));
					pNotifyContext->pinInfo[i].state &= ~REGISTERED_STATE;
					LeaveCriticalSection(&(pNotifyContext->pinInfo[i].cs));
				}
			}

			
			if(pNotifyContext->pinInfo[i].hInQueue)
			{
				CloseMsgQueue(pNotifyContext->pinInfo[i].hInQueue);
				pNotifyContext->pinInfo[i].hInQueue=NULL;
			}
#if 0
			if(pNotifyContext->pinInfo[i].hOutQueue)
			{
				CloseMsgQueue(pNotifyContext->pinInfo[i].hOutQueue);
				pNotifyContext->pinInfo[i].hOutQueue=NULL;
			}
#endif
			if(pNotifyContext->pinInfo[i].hEvent)
			{
				CloseHandle(pNotifyContext->pinInfo[i].hEvent);
				pNotifyContext->pinInfo[i].hEvent = NULL;
			}

			DeleteCriticalSection(&(pNotifyContext->pinInfo[i].cs));
		}
		
		pNotifyContext->pinInfo[i].state &= ~OPENED_STATE;	
			
	}

	memset(pNotifyContext->pinInfo,0,sizeof(pNotifyContext->pinInfo));


	ret = CloseHandle(handle);
	
	return (ret)? AIO_OK : AIO_INTERNAL_ERROR;
}



#if 0
//------------------------------------------------------------------------------
// Function name	: MIC_AIOGetInputState
// Description	    : Returns the current state of the given input pin. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : pInputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: currentState and opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOGetInputState(HANDLE handle, LPVOID context)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext = (PAIO_INPUT_CONTEXT) context;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOGetInputState: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD pOutBuff[2];

		ret = DeviceIoControl ( handle, IOCTL_INPUT_GET_STATE,(LPVOID)&pInputContext->pinNo, sizeof (AUT_PINS),
							    pOutBuff, sizeof(pOutBuff),0,NULL ); 

		pInputContext->opErrorCode = pOutBuff[0];

		if(pInputContext->opErrorCode == AIO_OK)
			pInputContext->currentState = pOutBuff[1];

		
	}

	return ret;
}

#endif


//------------------------------------------------------------------------------
// Function name	: MIC_AIOGetInputInfo
// Description	    : Returns the current state and mode of the given input pin.   
//
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
//
// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : context  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size,pinNo. 
//                   : OUT: state,mode and opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOGetInputInfo (HANDLE handle, LPVOID context)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext = (PAIO_INPUT_CONTEXT) context;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOGetInputInfo: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD pOutBuff[3];

		ret = DeviceIoControl ( handle, IOCTL_INPUT_GET_INFO,(LPVOID)&pInputContext->pinNo, sizeof (AUT_PINS),
							    pOutBuff, sizeof(pOutBuff),0,NULL ); 

		pInputContext->opErrorCode = pOutBuff[0];

		if(pInputContext->opErrorCode == AIO_OK)
		{
			pInputContext->currentState = (PIN_STATE)pOutBuff[1];
			pInputContext->mode         = (PIN_MODE) pOutBuff[2];
		}

		
	}

	return ret;
}


//------------------------------------------------------------------------------
// Function name	: MIC_AIORegisterForStateChange
// Description	    : Registers/Deregisters to get notifications on the state change event 
//                    on the given pin. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : IN: handle – valid handle from MIC_AIOOpen;
//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
//                   : IN/OUT: inputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: opErrorCode.
//                   : IN: needToRegister  if TRUE - need to register, otherwise - to deregister; 
//------------------------------------------------------------------------------
BOOL MIC_AIORegisterForStateChange(HANDLE handle, LPVOID notifyContext,LPVOID inputContext,BOOL needToRegister)
{
	 BOOL              ret = FALSE;

	 PAIO_INPUT_CONTEXT  pInputContext =  (PAIO_INPUT_CONTEXT) inputContext;
	 PAIO_NOTIFY_CONTEXT pNotifyContext = (PAIO_NOTIFY_CONTEXT) notifyContext;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIORegisterForStateChange: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		if(needToRegister)
		{
			ret =RegisterForStateChange(handle,pNotifyContext,pInputContext);
		}
		else
		{
			ret = DeregisterForStateChange(handle,pNotifyContext,pInputContext);
		}
	
		//	
	}

	return ret;
}


//------------------------------------------------------------------------------
// Function name	: MIC_AIOStartStateChangeNotify
// Description	    : Starts sending notifications on the pin status change.
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOStartStateChangeNotify(HANDLE handle, LPVOID notifyContext,LPVOID inputContext)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext   = (PAIO_INPUT_CONTEXT) inputContext;
	PAIO_NOTIFY_CONTEXT pNotifyContext = (PAIO_NOTIFY_CONTEXT) notifyContext;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOStartStateChangeNotify: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		ret = FALSE;
	}
	else
	{
		 DWORD   bitNo = BitNo(pInputContext->pinNo.mask);

		if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH || ((pNotifyContext->pinInfo[bitNo].state & OPENED_STATE)!=1))
		{
			RETAILMSG (1,(TEXT("MIC_AIOStartStateChangeNotify: Invalid Parameter \r\n")));
			pInputContext->opErrorCode = AIO_ERROR_INVALID_PARAMETER;
			return FALSE;
		}

		/*
		if(pNotifyContext->pinInfo[bitNo].state == OPENED_STATE)
			{
			RETAILMSG (1,(TEXT("MIC_AIORegisterForStateChange: Invalid Parameter \r\n")));
			pInputContext->opErrorCode = AIO_ERROR_IS_NOT_REGISTERED;
			return FALSE;
		}
		*/
 
		

		ret = DeviceIoControl ( handle, IOCTL_INPUT_START_CHANGE_EVENT,(LPVOID)&pInputContext->pinNo, sizeof (AUT_PINS),
							    (LPVOID)&pInputContext->opErrorCode, sizeof(DWORD),0,NULL ); 

		if(ret)
		{
			EnterCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
			pNotifyContext->pinInfo[bitNo].state |= START_NOTIFY_STATE;
			LeaveCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
		}

		
	}

	return ret;
}


//------------------------------------------------------------------------------
// Function name	: MIC_AIOStopStateChangeNotify
// Description	    : Stops sending notifications on the pin status change.
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOStopStateChangeNotify(HANDLE handle, LPVOID notifyContext,LPVOID inputContext)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext   = (PAIO_INPUT_CONTEXT) inputContext;
	PAIO_NOTIFY_CONTEXT pNotifyContext = (PAIO_NOTIFY_CONTEXT) notifyContext;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOStopStateChangeNotify: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		ret = FALSE;
	}
	else
	{
		 DWORD   bitNo = BitNo(pInputContext->pinNo.mask);

		if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH || ((pNotifyContext->pinInfo[bitNo].state & OPENED_STATE)!=1))
		{
			RETAILMSG (1,(TEXT("MIC_AIORegisterForStateChange: Invalid Parameter \r\n")));
			pInputContext->opErrorCode = AIO_ERROR_INVALID_PARAMETER;
			return FALSE;
		}

		/*
		if(pNotifyContext->pinInfo[bitNo].state == OPENED_STATE)
			{
			RETAILMSG (1,(TEXT("MIC_AIORegisterForStateChange: Invalid Parameter \r\n")));
			pInputContext->opErrorCode = AIO_ERROR_IS_NOT_REGISTERED;
			return FALSE;
		}
		*/
 
		

		ret = DeviceIoControl ( handle, IOCTL_INPUT_STOP_CHANGE_EVENT,(LPVOID)&pInputContext->pinNo, sizeof (AUT_PINS),
							    (LPVOID)&pInputContext->opErrorCode, sizeof(DWORD),0,NULL ); 

		if(ret)
		{
			EnterCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
			pNotifyContext->pinInfo[bitNo].state &= ~START_NOTIFY_STATE;
			LeaveCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
		}

		
	}

	return ret;
}


//------------------------------------------------------------------------------
// Function name	: MIC_AIOWaitForStateChange
// Description	    : It synchronously waits for given pin status change 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : IN  notifyContext is a valid context which has been get in MIC_AIOOpen(notifyContext);
//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: currentState,previousState and opErrorCode.
//------------------------------------------------------------------------------
 BOOL  MIC_AIOWaitForStateChange(HANDLE handle, LPVOID notifyContext,LPVOID inputContext)
 {
	 BOOL               ret = FALSE;
	 DWORD              bitNo;
//	 DWORD              ack;

	PAIO_INPUT_CONTEXT  pInputContext   = (PAIO_INPUT_CONTEXT) inputContext;
	PAIO_NOTIFY_CONTEXT pNotifyContext = (PAIO_NOTIFY_CONTEXT)notifyContext;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOWaitForStateChange: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	bitNo = BitNo(pInputContext->pinNo.mask);

	if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH || pNotifyContext->pinInfo[bitNo].hInQueue == NULL 
		|| ((pNotifyContext->pinInfo[bitNo].state & OPENED_STATE)!=1))
	{
		RETAILMSG (1,(TEXT("MIC_AIOWaitForStateChange: Invalid Parameter \r\n")));
		pInputContext->currentState = INVALID_STATE;
        pInputContext->prevState    = INVALID_STATE;
		pInputContext->opErrorCode  = AIO_ERROR_INVALID_PARAMETER ;
		return FALSE;
	}

	if( (pNotifyContext->pinInfo[bitNo].state & REGISTERED_STATE) == 0  || (pNotifyContext->pinInfo[bitNo].state & WAITING_FOR_EVENT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOWaitForStateChange: wrong notification state notifyContext = 0x%x\r\n"),notifyContext));
		pInputContext->currentState = INVALID_STATE;
        pInputContext->prevState    = INVALID_STATE;
		pInputContext->opErrorCode  = (pNotifyContext->pinInfo[bitNo].state & WAITING_FOR_EVENT) ? 
                                       AIO_ERROR_ALREADY_WAITING_FOR_EVENT:AIO_ERROR_IS_NOT_REGISTERED ;
		return FALSE;
	}


	//RETAILMSG (1,(TEXT("MIC_AIOWaitForStateChange: bitNo= 0x%x, mask = 0x%x\r\n"),bitNo,pInputContext->pinNo.mask));



	AIO_INPUT_CONTEXT msgContext;
	DWORD             dwNumberOfBytesRead,dwFlags;
	HANDLE            hEvents[2] = {pNotifyContext->pinInfo[bitNo].hEvent,pNotifyContext->pinInfo[bitNo].hInQueue};

	
	EnterCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
	pNotifyContext->pinInfo[bitNo].state |= WAITING_FOR_EVENT;
	LeaveCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));

//	RETAILMSG (1,(TEXT("MIC_AIOWaitForStateChange: before waiting ,notifyContext = 0x%x \r\n"),notifyContext));
	DWORD dwRetVal = WaitForMultipleObjects(sizeof(hEvents)/sizeof(hEvents[0]), hEvents, FALSE, INFINITE);
 //   RETAILMSG (1,(TEXT("MIC_AIOWaitForStateChange: dwRetVal= 0x%x,notifyContext = 0x%x \r\n"),dwRetVal,notifyContext));

	if(1 != (dwRetVal - WAIT_OBJECT_0))
	{
		pInputContext->currentState = INVALID_STATE;
        pInputContext->prevState    = INVALID_STATE;
		pInputContext->opErrorCode  = AIO_ERROR_WRONG_NOTIFICATION_STATE;
		ret = FALSE;
	}
	else
	{

		//	Okay, we have notification..  
		if(!ReadMsgQueue(pNotifyContext->pinInfo[bitNo].hInQueue,
							&msgContext,
							sizeof(msgContext),
							&dwNumberOfBytesRead,
							1,
							&dwFlags)|| ( sizeof(msgContext) != dwNumberOfBytesRead ))
		{

		
			RETAILMSG(1, (L"MIC_AIOWaitForStateChange: ReadMsgQueue failed %d\r\n",GetLastError()));
			ret = FALSE;
			pInputContext->currentState =INVALID_STATE;
			pInputContext->prevState    =INVALID_STATE;
			pInputContext->opErrorCode  =AIO_ERROR_INVALID_PARAMETER ;
		}
		else
		{
		//	RETAILMSG(1, (L"MIC_AIOWaitForStateChange: ReadMsgQueue succeeded NotifyContext = 0x%x\r\n",notifyContext));
#if 0
			ack = 1;

			if(!WriteMsgQueue(pNotifyContext->pinInfo[bitNo].hOutQueue, &ack, sizeof(ack),0, 0)) 
			{
				RETAILMSG(1, (_T("AIO API: WriteMsgQueue failed 0x%x\r\n"),GetLastError()));
			}
#endif

			pInputContext->currentState =msgContext.currentState;
			pInputContext->prevState    =msgContext.prevState;
			pInputContext->opErrorCode  =msgContext.opErrorCode; 

			if(pInputContext->opErrorCode == AIO_OK)
				ret = TRUE;


		}
	}

	EnterCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
	pNotifyContext->pinInfo[bitNo].state &= ~WAITING_FOR_EVENT;
	LeaveCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
	


	
	return ret;
 }


 //------------------------------------------------------------------------------
// Function name	: MIC_AIOSetSignalStabilityTime
// Description	    : Sets pin's stability' time. 
//                   In free mode (default) everyone is allowed to use this function.  
//                   In lock mode, the only "lock" caller handler is allowed to use this function. 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : inputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size,pinNo and 
//                   : OUT: opErrorCode.
//                   : IN:  stability time 
//------------------------------------------------------------------------------
BOOL MIC_AIOSetSignalStabilityTime(HANDLE handle, LPVOID inputContext,DWORD stabilityTime)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext = (PAIO_INPUT_CONTEXT) inputContext;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOSetSignalStabilityTime: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD buff[2];

		buff[0] = pInputContext->pinNo.mask;
		buff[1] = stabilityTime;

		ret = DeviceIoControl ( handle, IOCTL_INPUT_SET_STABLE_TIME,(LPVOID)&buff, sizeof (buff),
							    (LPVOID)&pInputContext->opErrorCode, sizeof(DWORD),0,NULL ); 
		
	}

	return ret;

}



 //------------------------------------------------------------------------------
// Function name	: MIC_AIOGetSignalStabilityTime
// Description	    : Gets pin's stability' time. 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : inputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo 
//                   : OUT: opErrorCode.
//                   : OUT:  stability time 
//------------------------------------------------------------------------------
BOOL MIC_AIOGetSignalStabilityTime(HANDLE handle, LPVOID inputContext,DWORD* stabilityTime)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext = (PAIO_INPUT_CONTEXT) inputContext;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOSetSignalStabilityTime: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD buff[2];

		ret = DeviceIoControl ( handle, IOCTL_INPUT_GET_STABLE_TIME,(LPVOID)&pInputContext->pinNo, sizeof (DWORD),
							    (LPVOID)&buff, sizeof(buff),0,NULL ); 

        pInputContext->opErrorCode = buff[0];

		if(ret && pInputContext->opErrorCode == AIO_OK)
			*stabilityTime = buff[1];
		
	}

	return ret;

}


//------------------------------------------------------------------------------
// Function name	: MIC_AIOSetInputMode
// Description	    : Sets input pin mode (FREE,LOCKED)  
//
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
//
// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : context  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size,pinNo and mode. 
//                   : OUT: opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOSetInputMode(HANDLE handle, LPVOID context)
{
	BOOL ret = FALSE;

	PAIO_INPUT_CONTEXT pInputContext = (PAIO_INPUT_CONTEXT) context;

	if(!pInputContext ||pInputContext->size != sizeof(AIO_INPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOSetInputMode: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD pInBuff[2];
        pInBuff[0]=(DWORD)pInputContext->pinNo.mask;
        pInBuff[1]=pInputContext->mode;

		ret = DeviceIoControl ( handle, IOCTL_INPUT_SET_MODE,(LPVOID)pInBuff, sizeof (pInBuff),
							    (LPVOID)&pInputContext->opErrorCode, sizeof(DWORD),0,NULL ); 
		
	}

	return ret;
}



//------------------------------------------------------------------------------
// Function name	: MIC_AIOSetOutputState
// Description	    : Sets pin state according to the chosen mode. 
//                   In free mode (default) everyone is allowed to use this function. 
//                   In accumulated mode the output is a logical OR of all the handle states. 
//                   In lock mode, the only "lock" caller handler is allowed to use this function. 

// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : context  is of PAIO_OUTPUT_CONTEXT type 
//                   : IN: Size,pinNo and state. 
//                   : OUT: opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOSetOutputState(HANDLE handle, LPVOID context)
{
	BOOL ret = FALSE;

	PAIO_OUTPUT_CONTEXT pOutputContext = (PAIO_OUTPUT_CONTEXT) context;

	if(!pOutputContext ||pOutputContext->size != sizeof(AIO_OUTPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOSetOutputState: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{

		ret = DeviceIoControl ( handle, IOCTL_OUTPUT_SET_STATE,(LPVOID)&pOutputContext->pinNo, 2*sizeof (DWORD),
							    (LPVOID)&pOutputContext->opErrorCode, sizeof(DWORD),0,NULL ); 
		
	}

	return ret;
}



//------------------------------------------------------------------------------
	// Function name	: MIC_AIOSetOutputMode
	// Description	    : Sets pin mode  
	//                  : Possible operations:
	//                  : FREE->LOCKED/ACCUMULATED - OK
	//                  : LOCKED -> FREE only "lock" caller handler ia allowed
	//                  : LOCKED -> LOCKED/ACCUMULATED - error
	//                  : ACCUMULATED-> FREE only "accumulated" caller handler ia allowed
	//                  : ACCUMULATED -> LOCKED - everyone is allowed
	//                  : ACCUMULATED ->ACCUMULATED - error
	// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
	//
	// Arguments         : handle – valid handle from MIC_AIOOpen;
	//                   : context  is of PAIO_OUTPUT_CONTEXT type 
	//                   : IN: Size,pinNo and mode. 
	//                   : OUT: opErrorCode.
	//------------------------------------------------------------------------------
BOOL  MIC_AIOSetOutputMode(HANDLE handle, LPVOID context)
{
	BOOL ret = FALSE;

	PAIO_OUTPUT_CONTEXT pOutputContext = (PAIO_OUTPUT_CONTEXT) context;

	if(!pOutputContext ||pOutputContext->size != sizeof(AIO_OUTPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOOutputSetMode: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD pInBuff[2];
        pInBuff[0]=(DWORD)pOutputContext->pinNo.mask;
        pInBuff[1]=pOutputContext->mode;

		ret = DeviceIoControl ( handle, IOCTL_OUTPUT_SET_MODE,(LPVOID)pInBuff, sizeof (pInBuff),
							    (LPVOID)&pOutputContext->opErrorCode, sizeof(DWORD),0,NULL ); 
		
	}

	return ret;
}



//------------------------------------------------------------------------------
// Function name	: MIC_AIOGetOutputInfo
// Description	    : Returns the current state and mode of the given output pin.   
//
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 
//
// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : context  is of PAIO_OUTPUT_CONTEXT type 
//                   : IN: Size,pinNo. 
//                   : OUT: state,mode and opErrorCode.
//------------------------------------------------------------------------------
BOOL  MIC_AIOGetOutputInfo (HANDLE handle, LPVOID context)
{
	BOOL ret = FALSE;

	PAIO_OUTPUT_CONTEXT pOutputContext = (PAIO_OUTPUT_CONTEXT) context;

	if(!pOutputContext ||pOutputContext->size != sizeof(AIO_OUTPUT_CONTEXT))
	{
		RETAILMSG (1,(TEXT("MIC_AIOGetOutputInfo: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
	}
	else
	{
		DWORD pOutBuff[3];

		ret = DeviceIoControl ( handle, IOCTL_OUTPUT_GET_INFO,(LPVOID)&pOutputContext->pinNo, sizeof (AUT_PINS),
							    pOutBuff, sizeof(pOutBuff),0,NULL ); 

		pOutputContext->opErrorCode = pOutBuff[0];

		if(pOutputContext->opErrorCode == AIO_OK)
		{
			pOutputContext->state = (PIN_STATE)pOutBuff[1];
			pOutputContext->mode  = (PIN_MODE) pOutBuff[2];
		}

		
	}

	return ret;
}

//------------------------------------------------------------------------------
DWORD BitNo( DWORD mask)
{
	DWORD ret = -1;

	UINT32 i;

	for (i = 0;i<32;i++)
	{
		if((mask >>i ) & 0x01)
		{
			ret = i;
			break;
		}
	}

	return ret;
}


//------------------------------------------------------------------------------
// Function name	: RegisterForStateChange
// Description	    : Registers/Deregisters to get notifications on the state change event 
//                    on the given pin. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: opErrorCode.
//                   : IN: needToRegister  if TRUE - need to register, otherwise - to deregister; 
//------------------------------------------------------------------------------
BOOL RegisterForStateChange(HANDLE handle, PAIO_NOTIFY_CONTEXT pNotifyContext,PAIO_INPUT_CONTEXT pInputContext)
{
	 BOOL                   ret   = FALSE;
	 DWORD                  bitNo = BitNo(pInputContext->pinNo.mask);;

	RETAILMSG (1,(TEXT("RegisterForStateChange: bitNo= 0x%x, mask = 0x%x\r\n"),bitNo,pInputContext->pinNo.mask));

	if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH || ((pNotifyContext->pinInfo[bitNo].state & OPENED_STATE)!=1))
	{
		RETAILMSG (1,(TEXT("MIC_AIORegisterForStateChange: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		pInputContext->opErrorCode = AIO_ERROR_INVALID_PARAMETER;
		return FALSE;
	}
 
	// send register io control

	ret = DeviceIoControl ( handle, IOCTL_INPUT_REGISTER_STATE_CHANGE,(LPVOID)&pInputContext->pinNo, sizeof (AUT_PINS),
						    (LPVOID)&pInputContext->opErrorCode, sizeof(DWORD),0,NULL ); 

	
	if(ret)
	{
		EnterCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
		pNotifyContext->pinInfo[bitNo].state |= REGISTERED_STATE;
		LeaveCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
	}


	return ret;
}


//------------------------------------------------------------------------------
// Function name	: DeregisterForStateChange
// Description	    : Registers/Deregisters to get notifications on the state change event 
//                    on the given pin. 
// Return type		: If the function succeeds, TRUE is returned; otherwise, FALSE is returned 

// Arguments         : handle – valid handle from MIC_AIOOpen;
//                   : IN/OUT: pInputContext  is of PAIO_INPUT_CONTEXT type 
//                   : IN: Size and pinNo. 
//                   : OUT: opErrorCode.
//                   : IN: needToRegister  if TRUE - need to register, otherwise - to deregister; 
//------------------------------------------------------------------------------
BOOL DeregisterForStateChange(HANDLE handle, PAIO_NOTIFY_CONTEXT pNotifyContext,PAIO_INPUT_CONTEXT pInputContext)
{
	 BOOL                   ret   = FALSE;
	 DWORD                  bitNo = BitNo(pInputContext->pinNo.mask);

	 RETAILMSG (1,(TEXT("DeregisterForStateChange: bitNo= 0x%x, mask = 0x%x\r\n"),bitNo,pInputContext->pinNo.mask));

	if(!pNotifyContext || pNotifyContext->size != OPEN_BUFFER_LENGTH || ((pNotifyContext->pinInfo[bitNo].state & OPENED_STATE)!=1))
	{
		RETAILMSG (1,(TEXT("DeregisterForStateChange: Invalid Parameter \r\n")));
		SetLastError(ERROR_INVALID_PARAMETER);
		pInputContext->opErrorCode =AIO_ERROR_INVALID_PARAMETER;
		return FALSE;
	}
   

	// send deregister io control

	ret = DeviceIoControl ( handle, IOCTL_INPUT_DEREGISTER_STATE_CHANGE,(LPVOID)&pInputContext->pinNo, sizeof (AUT_PINS),
						    (LPVOID)&pInputContext->opErrorCode, sizeof(DWORD),0,NULL ); 

	
	if(ret)
	{

		// we need to signal WaitForStateChange function
		if(pNotifyContext->pinInfo[bitNo].state & WAITING_FOR_EVENT)
		{
			SetEvent(pNotifyContext->pinInfo[bitNo].hEvent);
		}
		EnterCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));
	
		pNotifyContext->pinInfo[bitNo].state &= ~REGISTERED_STATE;

		LeaveCriticalSection(&(pNotifyContext->pinInfo[bitNo].cs));

	}


	return ret;
}
