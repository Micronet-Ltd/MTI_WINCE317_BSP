/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:          C module template
 *  Author(s):       Michael Streshinsky
 *  Creation Date:  6-Nov-2007
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

/********************** INCLUDES **********************************************/
#include "stdafx.h"
#include <pm.h>
#include "muxmdm.h"
#include <winioctl.h>
#include "MicUserSdk.h"
#include "Mixer.h"

/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
DWORD WINAPI MsgTh(PVOID param);


/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
HANDLE hMuxQueue, hBreakEvent, hRssiExitEvent;
BOOL  g_muxOwner = FALSE;

/********************* STATIC VARIABLES ***************************************/


/******************** FUNCTION DEFINITIONS ************************************/

/** 
 * @fn BOOL burnData(const BYTE *data, int len)
 *  
 * @param const BYTE *data
 * @param int len
 *
 * @return status of operation
 *
 * @retval TRUE success
 * @retval FALSE error
 *
 * @brief burn incoming data to Flash ( S-record format )
 *    
 */  
//BOOL TMP_bExample(const uint8 u8Parameter)
//{
//    return FALSE;
//}
#if 1
BOOL  InitMux(VMODEM_CONTEXT_s  *pVModemContext)
{
  MSGQUEUEOPTIONS msgOptions	= {0};

	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= MSGQUEUE_ALLOW_BROKEN;
	msgOptions.cbMaxMessage = sizeof(GSM710MESSAGE);
	msgOptions.bReadAccess	= 1;

	hMuxQueue = CreateMsgQueue(GSM710_OUTQUEUE_NAME, &msgOptions);
	if(hMuxQueue == INVALID_HANDLE_VALUE)
	  return(FALSE);

	hBreakEvent = CreateEvent(0, 0, 0, 0);
	//hRssiExitEvent = CreateEvent(0, 0, 0, 0);
	CloseHandle(CreateThread(NULL, 0, MsgTh, (LPVOID)pVModemContext, 0, NULL));

	return(TRUE);
}

void  DeInitMux(void)
{
	SetEvent(hBreakEvent);
	//SetEvent(hRssiExitEvent);
	CloseHandle(hBreakEvent);
	//CloseHandle(hRssiExitEvent);
	CloseMsgQueue(hMuxQueue);
}
#endif

#if 0
BOOL  MuxTurnOn(MUX_CMD_e muxCmd)
{
  INT32 res;

  switch(muxCmd)
  {
    case  MUX_ACTIVATE:
	 {
      res = MIC_GSM710Activate(1, 0);
      RETAILMSG(1, (L"MUX:%S, Activation result = %d\r\n",__FUNCTION__, res));
	 } break;

    case  MUX_DEACTIVATE:
	 {
      res = MIC_GSM710Activate(0, 0);
      RETAILMSG(1, (L"MUX:%S, De-Activation result = %d\r\n",__FUNCTION__, res));
	 } break;

	case  MODEM_TRURN_ON:
	 {
       Mic_GSM710ModemOn(1);
	 } break;

	case  MODEM_TRURN_OFF:
	 {
       res = MIC_GSMPower(0);
	   RETAILMSG(1, (L"MUX:%S, MODEM_TRURN_OF result = %d\r\n",__FUNCTION__, res));
	 } break;

	case  MUX_MODEM_ON_MESSAGE:
	 {
       SendModemOnToMux();
	 } break;

	default: return(FALSE);
  }

  return(TRUE);
}
#endif


MUX_ACTIVATION_ST_e   ActivateMux(void)
{
  INT32   res;

  res = MIC_GSM710Activate(1, 0);
  switch(res)
  {
    case GSM710ERROR_SUCCESS: 
	 {
      // This Application is owner of MUX
      g_muxOwner = TRUE;  
	  return(MUX_START_ACTIVATING);
	 }
    // case for future fix of 710 MUX driver:  return(MUX_ACTIVATION_IN_PROGRESS);
	case GSM710ERROR_ALREADY_ACTIVATED: return(MUX_ACTIVATED);
	default:
	 {
      RETAILMSG(1, (L"MUX:%S, MIC_GSM710Activate invalid return = %d\r\n",__FUNCTION__, res)); 
	  return(MUX_ACTIVATION_ERR);
	 }
  }

}


BOOL  TurnOffMuxModem(void)
{
 INT32 res;

 if( g_muxOwner == TRUE )  // This Application is owner of the Mux
 {
   // MUX de-activate
   res = MIC_GSM710Activate(0, 0);
   RETAILMSG(1, (L"MUX:%S, De-Activation result = %d\r\n",__FUNCTION__, res));
  
   res = MIC_GSMPower(0);
   RETAILMSG(1, (L"MUX:%S, MODEM_TRURN_OFF result = %d\r\n",__FUNCTION__, res));

   g_muxOwner = FALSE;
 }
 else
 {
  // Other Application is owner of the MUX
  RETAILMSG(1, (L"MUX:%S, Other Application is owner of the MUX\r\n",__FUNCTION__));
 }

 return(TRUE);
}




BOOL  testVoicePort(LPCTSTR pszVoicePortName, DWORD timeout)
{
  HANDLE hCom;
  BOOL   bRes = FALSE;
  DWORD  ticks;
  DWORD  dwTime;
  int    i;

    dwTime = 0;
	i = 0;
  
    do 
	{
		//Sleep(1000);
        ticks = GetTickCount();
		hCom = CreateFile(pszVoicePortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(INVALID_HANDLE_VALUE != hCom)
		{
		 CloseHandle(hCom);
		 bRes = TRUE;
         break;
		}
		i++;
		RETAILMSG(1, (L"MUX:%S hCom = 0x%X, try # %d, try time = %d ms\r\n",__FUNCTION__, hCom, i, (GetTickCount() - ticks) ));

		Sleep(1000);
		dwTime += 1000;

	} while( dwTime < timeout);

	RETAILMSG(1, (L"MUX:%S - %s  result = %d \r\n",__FUNCTION__, pszVoicePortName, bRes));
	return(bRes);
}

BOOL AT(HANDLE h, char *command, char *response)
{
	char mem[255] = {0};
	char *rbuf;
	DWORD wlen, rlen;

	if (response != NULL)
		rbuf = response;
	else
		rbuf = mem;

	BOOL wf = WriteFile(h, command, strlen(command), &wlen, NULL);
	Sleep(500);
	BOOL rf = ReadFile(h, rbuf, 255, &rlen, NULL);
	return wf;
}


BOOL Mic_GSM710ModemOn(BOOL bNeedRegistration)
{
	TCHAR b[100] = {0};
	INT32 res = MIC_GSMPower(1);
	if(res != 0)
	{
		RETAILMSG(1, (L"MUX:%S, MIC_GSMPower(ON) error = %d\r\n",__FUNCTION__, res));
		return(FALSE);
	}

	if(!bNeedRegistration)
	  return(TRUE);

	HANDLE hCom = CreateFile(L"COM0:", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hCom == INVALID_HANDLE_VALUE)
	{
		RETAILMSG(1, (L"MUX:%S, Can't open COM0 port\r\n",__FUNCTION__));
		return(FALSE);
	}

	DCB dcb = {0};
	dcb.DCBlength = sizeof(DCB);

    if (!GetCommState(hCom, &dcb))
    {
		RETAILMSG(1, (L"MUX:%S, GetCommState failed, error = %d\r\n",__FUNCTION__, GetLastError()));
		CloseHandle(hCom);
		return(FALSE);
    }

    // Set com port
    dcb.fBinary = TRUE;
    dcb.fParity = 0;
    dcb.fOutxCtsFlow = FALSE; //TRUE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fTXContinueOnXoff = TRUE;
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE; // RTS_CONTROL_HANDSHAKE;
    dcb.fAbortOnError = FALSE;
    dcb.ByteSize	= 8;
    dcb.Parity		= NOPARITY;
    dcb.StopBits	= ONESTOPBIT;

    dcb.BaudRate = CBR_115200;

    if (!SetCommState(hCom, &dcb))
    {
		RETAILMSG(1, (L"MUX:%S, SetCommState failed, error = %d\r\n",__FUNCTION__, GetLastError()));
		CloseHandle(hCom);
		return(FALSE);
    }

    COMMTIMEOUTS cto;
    memset(&cto, 0, sizeof(COMMTIMEOUTS));
    cto.ReadIntervalTimeout = MAXDWORD;
    cto.WriteTotalTimeoutConstant = 1000;
    if (!SetCommTimeouts(hCom, &cto))
	{
		RETAILMSG(1, (L"MUX:%S, SetCommState failed, error = %d\r\n",__FUNCTION__, GetLastError()));
		CloseHandle(hCom);
		return(FALSE);
	}

	RETAILMSG(1, (L"MUX:%S, Waiting for modem registration...\r\n",__FUNCTION__));

	char response[255] = {};
	DWORD ticks = GetTickCount() + 30000;
	BOOL bRegistered = FALSE;
	
	while (ticks > GetTickCount())
	{
		AT(hCom, "at+creg?\r", response);
		if (strstr(response, "+CREG: 0,1"))
		{
			bRegistered = TRUE;
			break;
		}
	}

	CloseHandle(hCom);
	if (!bRegistered)
	{
		RETAILMSG(1, (L"MUX:%S, Registration timeout error.\r\n",__FUNCTION__));
		return(FALSE);
	}

	RETAILMSG(1, (L"MUX:%S, Modem successfully registered\r\n",__FUNCTION__));
	return(TRUE);
}


void SendModemOnToMux()
{
	MSGQUEUEOPTIONS msgOptions	= {0};
	msgOptions.dwSize		= sizeof(MSGQUEUEOPTIONS);
	msgOptions.dwFlags		= 0;
	msgOptions.cbMaxMessage = sizeof(GSM710MESSAGE);
	msgOptions.bReadAccess	= 0;

	HANDLE h = CreateMsgQueue(GSM710_INQUEUE_NAME, &msgOptions);
	if (h != INVALID_HANDLE_VALUE)
	{
		GSM710MESSAGE msg;
		msg.Type = GSM710MESSAGETYPE_MODEMSTATE;
		msg.Value = 0;
		
		if (!WriteMsgQueue(h, &msg, sizeof(GSM710MESSAGE), 0, 0))
		{
			DWORD error = GetLastError();
			if (ERROR_PIPE_NOT_CONNECTED == error)
                RETAILMSG(1, (L"MUX:%S, Error sending message. MUX not active?\r\n",__FUNCTION__));
			else
				RETAILMSG(1, (L"MUX:%S, Error sending message\r\n",__FUNCTION__));
		}
		else
			RETAILMSG(1, (L"MUX:%S, Message sent\r\n",__FUNCTION__));

		CloseMsgQueue(h);
	}
	else
		RETAILMSG(1, (L"MUX:%S, Message queue error\r\n",__FUNCTION__));
}


#if 1
DWORD WINAPI MsgTh(LPVOID lpParam)
{
	GSM710MESSAGE lsdata;
	DWORD dwSize, dwFlags;
	HANDLE ev[2];
	ev[0] = hBreakEvent;
	ev[1] = hMuxQueue;
    VMODEM_CONTEXT_s  *pContext;

    pContext = (VMODEM_CONTEXT_s *)lpParam;

	while ((WAIT_OBJECT_0 + 1) == WaitForMultipleObjects(sizeof(ev)/sizeof(ev[0]), ev, 0, INFINITE))
	{
		if (ReadMsgQueue(hMuxQueue, &lsdata, sizeof(GSM710MESSAGE), &dwSize, 0, &dwFlags)) 
		{
		  //RETAILMSG(1, (L"MUX:%S, type = %d, value = %d\r\n",__FUNCTION__, lsdata.Type, lsdata.Value));
		  SendMessage(pContext->hDlg, WM_USER, lsdata.Type , lsdata.Value);

			if (lsdata.Type == GSM710MESSAGETYPE_CONNECTIONSTATE)
			 {
               //SendMessage(pContext->hDlg, WM_USER, GSM710MESSAGETYPE_CONNECTIONSTATE , lsdata.Value);  
               if( lsdata.Value == 1 )
			   {
                 //modemOnMux(pContext);
                 //RETAILMSG(1, (L"MUX:%S, GSM710MUX driver activated.\r\n",__FUNCTION__));
			   }
			   else
			   {
                 //RETAILMSG(1, (L"MUX:%S, GSM710MUX driver deactivated.\r\n",__FUNCTION__));
			   }
			 }
			else if (lsdata.Type == GSM710MESSAGETYPE_MODEMSTATE)
			 {
                //SendMessage(pContext->hDlg, WM_USER, GSM710MESSAGETYPE_MODEMSTATE , 0);  
				//RETAILMSG(1, (L"MUX:%S, Please turn on the GPRS modem.\r\n",__FUNCTION__));
				//Mic_GSM710ModemOn(TRUE);
			 }
			else if (lsdata.Type == GSM710MESSAGETYPE_POWERSTATE)
			{
				CEDEVICE_POWER_STATE power_state = (CEDEVICE_POWER_STATE)lsdata.Value;
				//RETAILMSG(1, (L"MUX:%S, Power state was changed to %d\r\n",__FUNCTION__, power_state));
			}
			else if (lsdata.Type == GSM710MESSAGETYPE_CONNECTIONERROR)
			{
				//RETAILMSG(1, (L"MUX:%S, Connection error = %\r\n",__FUNCTION__, lsdata.Value));
			}

		}
	}

	return 0;
}
#endif