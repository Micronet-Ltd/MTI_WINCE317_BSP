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
#include "Line.h"
//#include <commctrl.h>
//#include <mmsystem.h>
#include <MicUserSdk.h>
//#include <gsm_api.h>
#include "comm.h"
#include "modem.h"
#include "mdmparser.h"
#include "VoiceModem.h"


/********************** LOCAL CONSTANTS ***************************************/
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/
/****************** STATIC FUNCTION PROTOTYPES *********************************/
static void printModemStr(const char *pParseStr);
/*********************** LOCAL MACROS *****************************************/
/********************* GLOBAL VARIABLES ***************************************/
/********************* STATIC VARIABLES ***************************************/
char   gRespStr[MAX_SIZE_TO_READ];
char   gParseStr[MAX_SIZE_TO_READ];
char   tempStr[MAX_SIZE_TO_READ*2];
TCHAR  gURespStr[MAX_SIZE_TO_READ];

VMODEM_EVENTS  g_ModemEvent = VMODEM_OK;



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


DWORD WINAPI LineThread(LPVOID lpParam)
{
 //DWORD       dwEvent; 
 //VOICE_LINE  *pVoiceLine;
// INT32       iRet;
 //BOOL        bRet;
 //LONG        ret;
 //LINEMESSAGE lineMsg; 
   //HANDLE            hCom;
   char              *pParseStr;
   VMODEM_EVENTS     modemEvent;
   VMODEM_STATES     modemState, modemState2Send;
   BOOL              bModemStateQuery = FALSE;
   VMODEM_CONTEXT_s  *pContext;
   RESP_COMM_ERROR   respRet;

   pContext = (VMODEM_CONTEXT_s *)lpParam;

   RETAILMSG(1, (L"DIGVOICE:+%S, lpParam = %p\r\n",__FUNCTION__, lpParam));
   if( lpParam == NULL )
   {
     RETAILMSG(1, (L"DIGVOICE:%S not stared!, invalid parameter\r\n",__FUNCTION__));
     return(3);
   }

   if( pContext->hCom == INVALID_HANDLE_VALUE )
   {
    RETAILMSG(1, (L"DIGVOICE:%S not stared, Invalid Handle\r\n",__FUNCTION__));
    return(2);
   }


   while(1)
   {
     respRet = respATCmdApp(pContext->hCom, gRespStr);
	 switch(respRet)
	 {
	  case  RESP_COMM_OK: break;
	  case  RESP_COMM_EMPTY_EVENT:
	    {
          RETAILMSG(1, (L"DIGVOICE:-%S, Force close thread\r\n",__FUNCTION__)); 
          return(0);
 	    } break;

	  case  RESP_COMM_FAILURE:
	    {
          RETAILMSG(1, (L"DIGVOICE:%S, COMM failure (probably suspend/resume)\r\n",__FUNCTION__)); 
          continue;
 	    } break;

      default: break;
	 }

	 strcpy( gParseStr, gRespStr );
	 pParseStr = gParseStr;

	  while(pParseStr != NULL)
	  {
		  pParseStr = findEvent(pParseStr, &modemEvent, &modemState);
		  if( pParseStr == NULL )
           break;

		  //printModemStr(pParseStr);
		  RETAILMSG(1, (L"DIGVOICE:%S, cur ev = %d, ev = %d, state = %d\r\n",__FUNCTION__, g_ModemEvent, modemEvent, modemState));
#if 0
		  if( ( bModemStateQuery == TRUE ) && (modemState != (VMODEM_STATES)-1) )
		  {
           bModemStateQuery = FALSE; 
		   // Send Modem state message
		   if( PostThreadMessage(pContext->destThreadId, WM_APP, (WPARAM)modemState, (LPARAM)0) )
		    {
             RETAILMSG(1, (L"DIGVOICE:%S, State send: state = %d\r\n",__FUNCTION__, modemState));
		     // Quit message test
		     //PostThreadMessage(pContext->destThreadId, WM_QUIT, (WPARAM)0, (LPARAM)0);
		    }
		   else
		    {
             RETAILMSG(1, (L"DIGVOICE:%S, State send failure\r\n",__FUNCTION__));
		    }
	       }
#endif

		  //if( ( modemEvent == VMODEM_RING ) || (modemEvent == VMODEM_NO_CARRIER) )
		  //{
            PostThreadMessage(pContext->destThreadId, WM_APP, (WPARAM)modemEvent, (LPARAM)0);
		  //}

		   //RETAILMSG(1, (L"DIGVOICE:%S, State send: state = %d\r\n",__FUNCTION__, modemState));
		 
		  // if Modem Event (changed) verify modem status
		  if( (modemEvent != (VMODEM_EVENTS)-1 )&& 
			  (modemEvent != VMODEM_CURRENT_CALL)&&
			  (modemEvent != VMODEM_OK)&&
			  (modemEvent != g_ModemEvent ) 
			)
		  {
           bModemStateQuery = TRUE;
		   g_ModemEvent = modemEvent;
		   //sendATCmdApp(pContext->hCom, "AT+CLCC");
		   //RETAILMSG(1, (L"DIGVOICE:%S, State query: ev = %d, state = %d\r\n",__FUNCTION__, modemEvent, modemState));
		  }
		 

	  }	  // while(pParseStr != NULL)

   } // while(1)
  
  RETAILMSG(1, (L"DIGVOICE:-%S\r\n",__FUNCTION__));
  return(0);
}


static void printModemStr(const char *pStr)
{
 DWORD i,j;

		  j = 0;
		  for( i = 0; i < strlen(pStr); i++ )
		  {
			if( pStr[i] == '\r' )
			{
			  tempStr[j++] = '['; tempStr[j++] = '\\'; tempStr[j++] = 'r'; tempStr[j++] = ']';
			}
			else if( pStr[i] == '\n' )
			{
			 tempStr[j++] = '['; tempStr[j++] = '\\'; tempStr[j++] = 'n'; tempStr[j++] = ']';
			}
			else
			  tempStr[j++] = pStr[i];
		  }

		  tempStr[j] = 0;

  RETAILMSG(1, (L"   DIGVOICE:%S, mdm resp = {%S}\r\n",__FUNCTION__, tempStr));
}
