/** =============================================================================
 *
 *  Copyright (c) 2011 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           C module template
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   7-Apr-2011
 *
 *  Revision Histiory:
 *
 * Date         Author                     Description	
 * ---------------------------------------------------------------------------
 *
 * ============================================================================= 
 */

/********************** INCLUDES **********************************************/
//#include "stdafx.h"
//#include <windows.h>
#include "modem.h"
#include <commctrl.h>
#include <mmsystem.h>
#include <MicUserSdk.h>
//#include <gsm_api.h>
#include "comm.h"
#include "Line.h"
#include "MuxMdm.h"


/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/
BOOL            gModemOn = FALSE;
int             modemAudioProfile = -1;
HANDLE          g_hCom = INVALID_HANDLE_VALUE;
TCHAR           g_ComPort[8] = L"COM0:";
HANDLE          g_hLineThr = NULL;


BOOL isModemExists(void)
{
  UINT32  power;
  INT32   iRet;

  iRet = MIC_GSMGetPowerStatus(&power);

  if( iRet != GSM_OK )
   return(FALSE);
  else
   return(TRUE);

}

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
/*
BOOL TMP_bExample(const uint8 u8Parameter)
{
    return FALSE;
}
*/

BOOL modemOn(VMODEM_CONTEXT_s  *pVModemContext)
{
  INT32    retPower, ret;
  BOOL     bRes;

  if( gModemOn == TRUE )
   return(TRUE);
   
  if( modemAudioProfile == -1 )
  {
    gModemOn = FALSE;
    return(FALSE);
  }

    //INT32  MIC_GSMPower(INT32 power);
    retPower = MIC_GSMPower(1);

	// MIC_GSMVoice  workaround
	// The problem is that QSS command returns 
	// "SIM NOT INSERTED" status , but
	// If let its some delay, it returns "SIM INSERTED"
	Sleep(350);

	  if( g_hCom == INVALID_HANDLE_VALUE )
	  {
	   g_hCom = __OpenComPort(g_ComPort);

		   if( g_hCom == INVALID_HANDLE_VALUE )
		   {
			MIC_GSMPower(0);
			return(FALSE);
		   }
	  }

	 //Sleep(350);
	 ret = MIC_GSMVoice(g_hCom, 1, modemAudioProfile);
	 if( ret == ERROR_SUCCESS )
	  {
#if 0
        // Test ATZ influence
        bRet = sendATCmdApp(g_hCom, "ATZ\r\n", respStr);  
		bRet = sendATCmdApp(g_hCom, "AT#DVI?\r\n", respStr);  

        bRet = sendATCmdApp(g_hCom, "AT+FCLASS=8\r\n", respStr);  
        bRet = sendATCmdApp(g_hCom, "ATX0\r\n", respStr);  
		bRet = sendATCmdApp(g_hCom, "AT&D0\r\n", respStr);
        bRet = sendATCmdApp(g_hCom, "AT#DIALMODE=0\r\n", respStr); 
#endif
	   bRes = sendATCmdApp(g_hCom, "AT#DIALMODE=1");
	   bRes = sendATCmdApp(g_hCom, "ATE1");
	
       // Create Line Thread
	   pVModemContext->hCom = g_hCom;
	   g_hLineThr = CreateThread(0, 0, LineThread, pVModemContext, 0, 0 );
	   gModemOn = TRUE;
	   return(TRUE);
	  }
	 else
	  {
       gModemOn = FALSE;
       CloseHandle(g_hCom);
	   g_hCom = INVALID_HANDLE_VALUE;
	   MIC_GSMPower(0);
	   RETAILMSG(1, (L"DIGVOICE:%S, MIC_GSMVoice Error = %d\r\n",__FUNCTION__, ret));      
       return(FALSE);
	  }
}


BOOL modemOnMux(VMODEM_CONTEXT_s  *pVModemContext)
{
 INT32   ret;
 BOOL    bRet;
 

  if( gModemOn == TRUE )
   return(TRUE);
   
  if( modemAudioProfile == -1 )
  {
    gModemOn = FALSE;
    return(FALSE);
  }

  // Anyway test if COM6 is accessable
  if( !testVoicePort(L"COM6:", 30000) )
   return(FALSE);
  
   g_hCom = __OpenComPort(L"COM6:");
   if( g_hCom == INVALID_HANDLE_VALUE )
   {
	TurnOffMuxModem();
	return(FALSE);
   }

	 ret = MIC_GSMVoice(g_hCom, 1, modemAudioProfile);
	 if( ret == ERROR_SUCCESS )
	  {
       // Create Line Thread
       bRet = sendATCmdApp(g_hCom, "AT#DIALMODE=1"); 
	   bRet = sendATCmdApp(g_hCom, "ATE1"); 
	   pVModemContext->hCom = g_hCom;
	   g_hLineThr = CreateThread(0, 0, LineThread, pVModemContext, 0, 0 );
	   gModemOn = TRUE;
	   return(TRUE);
	  }
	 else
	  {
       gModemOn = FALSE;
       CloseHandle(g_hCom);
	   g_hCom = INVALID_HANDLE_VALUE;
	   TurnOffMuxModem();
	   RETAILMSG(1, (L"DIGVOICE:%S, MIC_GSMVoice Error = %d\r\n",__FUNCTION__, ret));      
       return(FALSE);
	  }

  return(TRUE);
}

BOOL modemOff(VMODEM_CONTEXT_s  *pVModemContext)
{
  INT32    ret;
 
  // First, close Line Thread
	if(g_hLineThr)
	{
	 SetCommMask(pVModemContext->hCom, 0);

        //TapiUnlock();
        WaitForSingleObject(g_hLineThr, 5000 /*INFINITE*/);
        //TapiLock();

	  CloseHandle(g_hLineThr);
	  g_hLineThr = NULL;
	}

  if( pVModemContext->bMux == FALSE )
  {
	  //INT32  MIC_GSMPower(INT32 power);
	  if( g_hCom != INVALID_HANDLE_VALUE )
	  {
	   ret = MIC_GSMVoice(g_hCom, 0, 3);
	   ret = MIC_GSMPower(0);
	  }

	  gModemOn = FALSE;

		if( g_hCom != INVALID_HANDLE_VALUE )
		{
		 CloseHandle(g_hCom);
		 g_hCom = INVALID_HANDLE_VALUE;
		 pVModemContext->hCom = g_hCom;
		}

		if( ( ret != 0  )&& ( ret != GSM_ERROR_CONTROLLED_BY_MUX ) )
		{
			//wsprintf( myTest, _T("GSM Modem turn off failure") );
			//SetDlgItemText( hDlg, IDC_STATUS1, myTest );
			return(FALSE);
		}
		else
		{
			//wsprintf( myTest, _T("GSM Modem turned off") );
			//SetDlgItemText( hDlg, IDC_STATUS1, myTest );
			return(TRUE);
		}
  }
  else
  {
	  TurnOffMuxModem();
	  gModemOn = FALSE;

      if( g_hCom != INVALID_HANDLE_VALUE )
		{
		 CloseHandle(g_hCom);
		 g_hCom = INVALID_HANDLE_VALUE;
		 pVModemContext->hCom = g_hCom;
		}

    return(TRUE);
  }
}



BOOL modemOnTest(void)
{
  INT32    ret;

  if( gModemOn == TRUE )
   return(TRUE);

  if( g_hCom == INVALID_HANDLE_VALUE )
  {
    g_hCom = __OpenComPortTest(g_ComPort);
    if( g_hCom == INVALID_HANDLE_VALUE )
     return(FALSE);
  }

		if( modemAudioProfile != -1 )
		{
		 ret = MIC_GSMVoice(g_hCom, 1, modemAudioProfile);
		}
		else
		{
         gModemOn = FALSE;
    	 return(FALSE);
		}

		if( ret == 0 )
		{
		  gModemOn = TRUE;
		  return(TRUE);
		}
		else
		{
         gModemOn = FALSE;
    	 return(FALSE);
		}

}


void modemSetAudioProfile(DWORD pfofile)
{
  modemAudioProfile = pfofile;
}

BOOL  isModemOn(void)
{
  return( gModemOn );
}


INT32  modemVoiceDial(const char *pCmdStr)
{
  //char   atCmd[MAX_SIZE_TO_READ];

  return( MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_DIAL, pCmdStr) );
#if 0

  //sendATCmdApp(pContext->hCom, "AT+CLCC");
  strcpy(atCmd, "ATD");
  strcat(atCmd, pCmdStr);
  strcat(atCmd, ";");

  if( !sendATCmdApp(g_hCom, atCmd))
   {
    RETAILMSG(1, (L"DIGVOICE:%S, Voice DIAL Error = %d\r\n",__FUNCTION__, GSM_ERROR_AT_CMD));
	return(GSM_ERROR_AT_CMD);
  }
  return(0); 

#endif
}

INT32  modemVoiceHangUp(void)
{
  return( MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_HANG_UP, NULL) );
#if 0
  if( !sendATCmdApp(g_hCom, "ATH"))
   {
    RETAILMSG(1, (L"DIGVOICE:%S, Voice DIAL Error = %d\r\n",__FUNCTION__, GSM_ERROR_AT_CMD));
	return(GSM_ERROR_AT_CMD);
  }

  return(0);
#endif
}

INT32  modemVoiceAnswer(void)
{
 return( MIC_GSMVoiceCMD(g_hCom, GSM_VOICE_ANSWER, NULL) );
#if 0
  if( !sendATCmdApp(g_hCom, "ATA"))
   {
    RETAILMSG(1, (L"DIGVOICE:%S, Voice DIAL Error = %d\r\n",__FUNCTION__, GSM_ERROR_AT_CMD));
	return(GSM_ERROR_AT_CMD);
  }
  return(0);
#endif
}