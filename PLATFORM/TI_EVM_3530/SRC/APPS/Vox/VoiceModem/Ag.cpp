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
#include "Modem.h"
#include "Ag.h"
//#include <commctrl.h>
//#include <mmsystem.h>
#include <MicUserSdk.h>
//#include <gsm_api.h>
//#include "comm.h"
//#include "VoiceModem.h"


/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
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
/*
BOOL TMP_bExample(const uint8 u8Parameter)
{
    return FALSE;
}
*/


DWORD WINAPI AgThread(LPVOID lpParam)
{
 //DWORD       dwEvent; 
 //VOICE_LINE  *pVoiceLine;
 //HANDLE      hAgHS;
 INT32       iRet;
 //BOOL        bRet;
 //LONG        ret;
 AGHS_MSG    AgMsg;
 HICON       hBTIcon = NULL;
 AG_HS_EVENTS  AgEvent;
 //LINEMESSAGE lineMsg; 
 VMODEM_CONTEXT_s  *pContext;

 pContext = (VMODEM_CONTEXT_s *)lpParam;

   //hAgHS = *(HANDLE *)lpParam;


   while(1)
   {
    RETAILMSG(1, (L"DIGVOICE:%S, Enter  'MIC_AGHSWaitForEvent'\r\n",__FUNCTION__));
    iRet =  MIC_AGHSWaitForEvent( pContext->hAgHS, &AgMsg);
    //RETAILMSG(1, (L"DIGVOICE:%S, ret = %d, Event Msg = %d , %d\r\n",__FUNCTION__, iRet, AgMsg.hsEvent, AgMsg.dwParam));

	switch(iRet)
	{
	 case  AG_HS_OK:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Valid Event = 0x%X ",__FUNCTION__, AgMsg.hsEvent));
	   AgEvent = (AG_HS_EVENTS)-1;

	   switch(AgMsg.hsEvent)
	   {
	    case  EV_AG_HS_BUTTON_PRESS:
	     {
           AgEvent = AG_HS_BUTTON_PRESS;
           RETAILMSG(1, (L"[BUTTON_PRESS]"));
		 } break;
		case  EV_AG_HS_MIC_GAIN:
	     {
           AgEvent = AG_HS_MIC_GAIN;
           RETAILMSG(1, (L"[MIC_GAIN]"));
		 } break;
		case  EV_AG_HS_SPEAKER_GAIN:
	     {
           AgEvent = AG_HS_SPEAKER_GAIN;
           RETAILMSG(1, (L"[SPEAKER_GAIN]"));
		 } break;
		case  EV_AG_HS_CTRL:
	     {
           AgEvent = AG_HS_CTRL;
           RETAILMSG(1, (L"[HS_CTRL]"));
		 } break;

		case  EV_AG_HS_AUDIO:
	     {
           AgEvent = AG_HS_AUDIO;
           RETAILMSG(1, (L"[HS_AUDIO]\r\n"));
		   //EnterCriticalSection(&gcsBTVoice);

		   //LeaveCriticalSection(&gcsBTVoice);

		 } break;

		default:
	     {
           RETAILMSG(1, (L"[UNKNOWN!]"));
		 } break;
	   }

	   if( AgEvent != (AG_HS_EVENTS)-1 )
	   {
	    if( PostThreadMessage(pContext->destThreadId, WM_APP+1, (WPARAM)AgEvent, (LPARAM)AgMsg.dwParam) )
		  {
            RETAILMSG(1, (L"DIGVOICE:%S, AG event sent: ev = %d, param = %d\r\n",__FUNCTION__, AgEvent, AgMsg.dwParam));
		  }
		else
		  {
            RETAILMSG(1, (L"DIGVOICE:%S, AG event sent failure\r\n",__FUNCTION__));
		  }
	   }

        RETAILMSG(1, (L" param = %d\r\n", AgMsg.dwParam));

	  } break;

	 case  AG_HS_SERVICE_INVALID_PARAM:
	 case  AG_HS_MSG_CREATE_ERROR:
	 case  AG_HS_MSG_READ_ERROR:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Wait Event Error = %d\r\n",__FUNCTION__, iRet));      
	   return(-2);
	  } break;

	 case  AG_HS_DEV_CLOSED:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Dev Hanlde Closed!\r\n",__FUNCTION__));      
	   return(0);
	  } break;

	 default:
	  {
       RETAILMSG(1, (L"DIGVOICE:%S, Unknown Wait Event = %d\r\n",__FUNCTION__, iRet));      
	   return(-3);
	  } break;
	}

   }
  

  return(0);
}
