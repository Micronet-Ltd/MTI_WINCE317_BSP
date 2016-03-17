//#include "UART.h"
#include "stdafx.h"
#include "comm.h"
#include <windows.h>
#include <commctrl.h>
#include <gsm_api.h>


#define   COMM_DBG_PRN               0

//bool    g_bUseXonXoff = true;
//HANDLE  m_hPort = NULL;

static DWORD calcTimeoutApp(DWORD prevTime);
static BOOL WaitIncomingApp(HANDLE hCom, DWORD *pEventMask);

//#include <devload.h>
extern "C" HANDLE WINAPI __OpenComPort(LPCTSTR pszPortName)
{
	  HANDLE hCom;
      DWORD  dwRet;
	  BOOL   bRet;
 
      //DebugPV(_T("Initializing com port [%s], Xon Flow control=%d"), pszPortName, (int)g_bUseXonXoff);
     
      hCom = CreateFile(pszPortName , GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
      if (hCom == INVALID_HANDLE_VALUE)
      {
           // DebugP(L"Cannot open relevat port");
            return hCom;
      }
      
      SetupComm(hCom, 4096, 4096);
	  dwRet = GetLastError();
 

      DCB dcb = {0};

      bRet = GetCommState(hCom, &dcb);
	  dwRet = GetLastError();

      dcb.DCBlength = sizeof(dcb);
      dcb.fBinary           = 1;
      dcb.fParity           = 0;
      dcb.BaudRate          = CBR_115200;
      dcb.ByteSize          = DATABITS_8; 
      dcb.Parity            = NOPARITY; 
      dcb.StopBits          = ONESTOPBIT;
      dcb.fOutxCtsFlow      = 0;
      dcb.fOutxDsrFlow      = 0;
      dcb.fDtrControl       = DTR_CONTROL_ENABLE;
      dcb.fRtsControl       = RTS_CONTROL_ENABLE;
      dcb.fDsrSensitivity   = 0;
      dcb.fTXContinueOnXoff = 0;
 
      dcb.fOutX             = 0; //g_bUseXonXoff?TRUE:FALSE;
      dcb.fInX              = 0; //g_bUseXonXoff?TRUE:FALSE;
      dcb.XoffLim               = 1024;
      dcb.XoffLim               = 512;
      dcb.XonChar               = XON_CHAR;
      dcb.XoffChar              = XOFF_CHAR;    
      dcb.fErrorChar        = 0;  
      dcb.ErrorChar         = 0x0D;  
      dcb.EvtChar               = 0x0D;   
      dcb.fNull             = 0;
      dcb.fAbortOnError     = 0;


      bRet = SetCommState(hCom, &dcb);
	  dwRet = GetLastError();

      COMMTIMEOUTS CommTimeOuts;

      CommTimeOuts.ReadIntervalTimeout = 1 ;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
      CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.WriteTotalTimeoutConstant = 0 ;

      bRet = SetCommTimeouts(hCom, &CommTimeOuts);
	  dwRet = GetLastError();

      bRet = EscapeCommFunction(hCom, SETDTR);
	  dwRet = GetLastError();

      bRet = EscapeCommFunction(hCom, SETRTS);
	  dwRet = GetLastError();

      COMSTAT cs;

      bRet = ClearCommError(hCom, &dwRet, &cs);
	  dwRet = GetLastError();

      bRet = PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	  dwRet =  GetLastError();

      return hCom;

}


extern "C" HANDLE WINAPI __OpenComPortTest(LPCTSTR pszPortName)
{
	  HANDLE hCom;
      DWORD  dwRet;
	  INT32    ret;
	  BOOL   bRet;
 
      //DebugPV(_T("Initializing com port [%s], Xon Flow control=%d"), pszPortName, (int)g_bUseXonXoff);
     
      hCom = CreateFile(pszPortName , GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
      if (hCom == INVALID_HANDLE_VALUE)
      {
           // DebugP(L"Cannot open relevat port");
            return hCom;
      }
 
	  ret = MIC_GSMPower(1);

      SetupComm(hCom, 4096, 4096);
	  dwRet = GetLastError();
 

      DCB dcb = {0};

      bRet = GetCommState(hCom, &dcb);
	  dwRet = GetLastError();

      dcb.DCBlength = sizeof(dcb);
      dcb.fBinary           = 1;
      dcb.fParity           = 0;
      dcb.BaudRate          = CBR_115200;
      dcb.ByteSize          = DATABITS_8; 
      dcb.Parity            = NOPARITY; 
      dcb.StopBits          = ONESTOPBIT;
      dcb.fOutxCtsFlow      = 0;
      dcb.fOutxDsrFlow      = 0;
      dcb.fDtrControl       = DTR_CONTROL_ENABLE;
      dcb.fRtsControl       = RTS_CONTROL_ENABLE;
      dcb.fDsrSensitivity   = 0;
      dcb.fTXContinueOnXoff = 0;
 
      dcb.fOutX             = 0; //g_bUseXonXoff?TRUE:FALSE;
      dcb.fInX              = 0; //g_bUseXonXoff?TRUE:FALSE;
      dcb.XoffLim               = 1024;
      dcb.XoffLim               = 512;
      dcb.XonChar               = XON_CHAR;
      dcb.XoffChar              = XOFF_CHAR;    
      dcb.fErrorChar        = 0;  
      dcb.ErrorChar         = 0x0D;  
      dcb.EvtChar               = 0x0D;   
      dcb.fNull             = 0;
      dcb.fAbortOnError     = 0;


      bRet = SetCommState(hCom, &dcb);
	  dwRet = GetLastError();

      COMMTIMEOUTS CommTimeOuts;

      CommTimeOuts.ReadIntervalTimeout = 1 ;
      CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
      CommTimeOuts.WriteTotalTimeoutMultiplier = 0 ;
      CommTimeOuts.WriteTotalTimeoutConstant = 0 ;

      bRet = SetCommTimeouts(hCom, &CommTimeOuts);
	  dwRet = GetLastError();

      bRet = EscapeCommFunction(hCom, SETDTR);
	  dwRet = GetLastError();

      bRet = EscapeCommFunction(hCom, SETRTS);
	  dwRet = GetLastError();

      COMSTAT cs;

      bRet = ClearCommError(hCom, &dwRet, &cs);
	  dwRet = GetLastError();

      bRet = PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	  dwRet =  GetLastError();

      return hCom;

}


static BOOL WaitIncomingApp(HANDLE hCom, DWORD *pEventMask)
{
  RETAILMSG(COMM_DBG_PRN, (L"DIGVOICE:+%S, hCom = 0x%X\r\n",__FUNCTION__, hCom));

      while(hCom != INVALID_HANDLE_VALUE)
      {
            DWORD dwCommModemStatus = 0;
            DWORD dwBytesRead = 0;

            DWORD dwRet = WaitCommEvent(hCom, &dwCommModemStatus, 0);
            if (!dwRet) 
            {
              RETAILMSG(1,(TEXT("DIGVOICE:%S WaitIncoming error %d\r\n"),__FUNCTION__, GetLastError() )); 
	          return(FALSE);
            }

			*pEventMask = dwCommModemStatus;

            if (dwCommModemStatus & EV_ERR)
            {
                  DebugBreak();
            }

			if( dwCommModemStatus == 0 )
			{
              RETAILMSG(1, (L"DIGVOICE:-%S force exit\r\n",__FUNCTION__));
              return TRUE;
			}

            if (!(dwCommModemStatus & (EV_RXCHAR)))
            {
                  //DebugP(_T("WaitIncoming -Receive () - NOT RX\n"));
                  continue;
            }


            COMSTAT cs= {0};

            DWORD dwErr =0;

            ClearCommError(hCom, &dwErr, &cs);
			
            if (cs.cbInQue == 0)
                  continue;

			RETAILMSG(COMM_DBG_PRN, (L"DIGVOICE:-%S Ok\r\n",__FUNCTION__));
            return TRUE;
      }
	  RETAILMSG(1, (L"DIGVOICE:-%S Fail\r\n",__FUNCTION__));
      return FALSE;
}



//char	s_Buffer[MAX_SIZE_TO_READ];



BOOL sendATCmdApp(HANDLE hCom, const char *cmdStr)
{
 DWORD  NumBytesWritten;
 char	s_Buffer[MAX_PHONE_NUMBER+1];

 RETAILMSG(COMM_DBG_PRN, (L"DIGVOICE:+%S, cmdStr = %S\r\n",__FUNCTION__, cmdStr));

 // Check the validity of cmdStr
 if( cmdStr == NULL )
 {
  RETAILMSG(1,(TEXT("sendATCmd Error: invalid pointer\r\n")));
  return(FALSE);
 }

 // Check the length of cmdStr
 if( strnlen(cmdStr, (MAX_PHONE_NUMBER+1) ) >= (MAX_PHONE_NUMBER+1) )
 {
  RETAILMSG(1,(TEXT("sendATCmd Error: invalid length\r\n")));
  return(FALSE);
 }

   strcpy(s_Buffer, cmdStr );
   strcat(s_Buffer, "\r\n");
 
	 if( WriteFile(hCom, s_Buffer, strlen(s_Buffer), &NumBytesWritten, NULL) )
	 {
       RETAILMSG(COMM_DBG_PRN, (L"DIGVOICE:%S, cmdStr sent %d bytes\r\n",__FUNCTION__, NumBytesWritten));
	   return(TRUE);
	 }
	 else
	 {
      RETAILMSG(1,(TEXT("DIGVOICE:%S WriteFile error  %d\r\n"),__FUNCTION__, GetLastError() )); 
	  return(FALSE);
	 }

}


RESP_COMM_ERROR respATCmdApp(HANDLE hCom, char *respStr)
{
 char   *pbuffer;
 DWORD	NumOfReadBytes = 0;
 DWORD  dwEventMask, charCnt, timeout, curTime, Retval;
 RESP_COMM_ERROR   ret	= RESP_COMM_FAILURE;

 RETAILMSG(COMM_DBG_PRN,(TEXT("DIGVOICE:+%S\r\n"),__FUNCTION__)); 

	 if( !SetCommMask(hCom, EV_RXCHAR | EV_ERR) )
	 {
      RETAILMSG(1,(TEXT("DIGVOICE:%S SetCommMask error  %d\r\n"),__FUNCTION__, GetLastError() )); 
	  return(RESP_COMM_FAILURE);
	 }

      if( !WaitIncomingApp(hCom, &dwEventMask) )
	  {
        return(RESP_COMM_FAILURE);
	  }

	  if( dwEventMask == 0 )
        return(RESP_COMM_EMPTY_EVENT);

    //memset(respStr, 0, MAX_PHONE_NUMBER);
    pbuffer = respStr;
	charCnt = 0;
	timeout = 0;

     //This applies especially to applications that “sense” the OK text and therefore may send
     //the next command before the complete code <CR><LF>OK<CR><LF> is sent by the module.
	do
	{
		curTime = GetTickCount();
		Sleep(20);
		Retval = ReadFile(hCom, pbuffer, MAX_PHONE_NUMBER-1, &NumOfReadBytes, NULL);
		if( ( Retval != 0  )&& (NumOfReadBytes !=0) )
		{
          charCnt += NumOfReadBytes;
		  if( charCnt >= MAX_PHONE_NUMBER-1 )
		  {
           charCnt = MAX_PHONE_NUMBER-1;
		   pbuffer[MAX_PHONE_NUMBER-1] = '\0';
           break;
		  }

          pbuffer += NumOfReadBytes;
          *pbuffer = '\0';

		  if( strstr(respStr, "\r\n") )
		  {
            ret	= RESP_COMM_OK;
            break;
		  }

		}

		if( NumOfReadBytes == 0 )
         break;

		timeout += calcTimeoutApp(curTime);
 	    //timeout++;

	}while(timeout < 1000*60);


    //It is advisable anyway to wait for at least 20ms between the end of the reception of the
    //response and the issue of the next AT command.
   Sleep(30);

  RETAILMSG(COMM_DBG_PRN, (L"DIGVOICE:%S, ret = %d, timeout = %d ms, char cnt = %d\r\n",__FUNCTION__, ret, (timeout), charCnt));
#if 1
  DWORD i;

  RETAILMSG(1/*COMM_DBG_PRN*/, (L"DIGVOICE:%S, resp:\r\n",__FUNCTION__));
  for( i = 0; i < charCnt; i++ )
  {
    if( respStr[i] == '\r' )
     RETAILMSG(1/*COMM_DBG_PRN*/, (L"[\\r]"));
    else if( respStr[i] == '\n' )
     RETAILMSG(1/*COMM_DBG_PRN*/, (L"[\\n]"));
	else
      RETAILMSG(1/*COMM_DBG_PRN*/, (L"%c",respStr[i])); 
  }
  RETAILMSG(1/*COMM_DBG_PRN*/, (L"\r\n\r\n"));
#endif

  RETAILMSG(COMM_DBG_PRN, (L"DIGVOICE:-%S, ret = %d\r\n",__FUNCTION__, ret));
  return(ret);

}


static DWORD calcTimeoutApp(DWORD prevTime)
{
	DWORD curTime;

    curTime = GetTickCount();

	if( curTime > prevTime )
     return(curTime - prevTime);
	else if( curTime < prevTime ) // carry occured
	 {
       return( (UINT_MAX - prevTime) + curTime + 1 );
	 }
	else
     return(0);
}