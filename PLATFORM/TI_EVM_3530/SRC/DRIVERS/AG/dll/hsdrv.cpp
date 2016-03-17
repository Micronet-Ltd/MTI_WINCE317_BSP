/** =============================================================================
 *
 *  Copyright (c) 2007 Micronet
 *  All rights reserved.
 *
 *  
 *  Title:           C HSDRV module
 *  Author(s):       Michael Streshinsky
 *  Creation Date:   19-jan-2011
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
#include <tapi.h>
#include "btagpub.h"
#include "btagnetwork.h"

#include "hsdrv.h"
#include "ag_hs_api.h"

/********************** LOCAL CONSTANTS ***************************************/
#define  AGHS_QUEUE_NAME		                     L"AGHS queue name"
#define  AGHS_MAPVIEW_NAME		                     L"AGHS_MAP_VIEW"
#define  AGHS_MUTEX_NAME		                     L"AGHS_MUTEX"


#define  HS_DRV_DBG_PRN                              0
/*********** LOCAL STRUCTURES, ENUMS, AND TYPEDEFS ****************************/


/*********************** LOCAL MACROS *****************************************/

/********************* GLOBAL VARIABLES ***************************************/
volatile DWORD                    gTestVarCnt = 0;

/********************* STATIC VARIABLES ***************************************/
volatile static DWORD             gcTestVarCnt = 0;


/****************** STATIC FUNCTION PROTOTYPES *********************************/
//static BT_DEVICE_CONTEXT_AG_HS  *BthAGCheckContext( BT_DEVICE_CONTEXT_AG_HS btContextIn );
static BT_DEVICE_CONTEXT_AG_HS  *BthAGCheckContext( HANDLE pDevContext, DWORD *pContextNum );
static BOOL                      BthAGSendEventExit(void *pBtContext, HANDLE hMap, HANDLE BthAGmutex, BOOL bRet);

/******************** FUNCTION DEFINITIONS ************************************/

BOOL BthAGSetRegEvent(HANDLE pDevContext, DWORD eventMask)
{
  DWORD                     dwErr;
  HANDLE                    BthAGmutex;
  BT_DEVICE_CONTEXT_AG_HS   *pBtContext;
  DWORD                     contextNum;
  //BT_DEVICE_CONTEXT_AG_HS   *pContext;

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S, eventMask = 0x%X\r\n",__FUNCTION__, eventMask));

  if( pDevContext == NULL ) 
   return(FALSE);

  pBtContext = (BT_DEVICE_CONTEXT_AG_HS *)pDevContext;

  // Verify invalid events, empty event is valid, means event de-registering
  if( ( eventMask != 0 ) && ( eventMask & (~EV_AG_ALL_MASK) ) )
   {
    RETAILMSG(1, (L"HS_DRV:-%S, Invalid Event mask\r\n",__FUNCTION__)); 
    return(FALSE);
   }

  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  pBtContext = BthAGCheckContext(pDevContext, &contextNum);
  if( pBtContext == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Context Failure\r\n",__FUNCTION__));
   CloseHandle(BthAGmutex);
   return(FALSE);
  }

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   CloseHandle(BthAGmutex);
   return(FALSE);
  }

  
  pBtContext->hsEventMask = eventMask;

  ReleaseMutex(BthAGmutex);
  CloseHandle(BthAGmutex);

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S\r\n",__FUNCTION__));
  return(TRUE);
}



BOOL  BthAGGetRegEvent(HANDLE pDevContext, DWORD *pEventMask)
{
  DWORD                     dwErr;
  HANDLE                    BthAGmutex;
  DWORD                     contextNum;
  BT_DEVICE_CONTEXT_AG_HS  *pBtContext;

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S\r\n",__FUNCTION__));

  if( pDevContext == NULL ) 
   return(FALSE);

  pBtContext = (BT_DEVICE_CONTEXT_AG_HS *)pDevContext;

  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  pBtContext = BthAGCheckContext(pDevContext, &contextNum);
  if( pBtContext == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Context Failure\r\n",__FUNCTION__));
   CloseHandle(BthAGmutex);
   return(FALSE);
  }

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   CloseHandle(BthAGmutex);
   return(FALSE);
  }

  CeSafeCopyMemory(pEventMask, &pBtContext->hsEventMask, sizeof(DWORD));
  //*pEventMask = pBtContext->hsEventMask;

  ReleaseMutex(BthAGmutex);
  CloseHandle(BthAGmutex);

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S\r\n",__FUNCTION__));
  return(TRUE);

}

//BthAGOpenDevice
BT_DEVICE_CONTEXT_AG_HS  *BthAGOpenDevice( void )
{
 MSGQUEUEOPTIONS		   msgOptions;
 DWORD                     lTestVarCnt = 0;
 HANDLE                    hMap;
 DWORD                     hMapErr;
 DWORD                     dwSize, dwErr;
 DWORD                     contextNum;
 BT_DEVICE_CONTEXT_AG_HS   *pBtContext;
 HANDLE                    BthAGmutex;
 //void                      *pSharedMem;
 
  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S\r\n",__FUNCTION__));

  dwSize = (MAX_OPEN_HANDLERS * sizeof(BT_DEVICE_CONTEXT_AG_HS)) + SHARED_MEM_SCRATCH_PAD_SIZE;

  hMap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwSize, AGHS_MAPVIEW_NAME);
  hMapErr = GetLastError();

  if( hMap != NULL )
  {
   pBtContext = (BT_DEVICE_CONTEXT_AG_HS *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0,0, dwSize );
   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, CreateFileMapping Ok, err = %d, addr = 0x%X\r\n",__FUNCTION__, hMapErr, pBtContext));
  }
  else
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateFileMapping Failed, err = %d\r\n",__FUNCTION__, hMapErr));
   return(NULL);
  }

  if( (void *)pBtContext != NULL )
  {
    if( hMapErr != ERROR_ALREADY_EXISTS )
	{
     for( contextNum = 0; contextNum < MAX_OPEN_HANDLERS; contextNum++ )
	 {
      memset( &pBtContext[contextNum], 0, sizeof(BT_DEVICE_CONTEXT_AG_HS) );
	 }
	 contextNum = 0;
	}
	else
	{
     for( contextNum = 0; contextNum < MAX_OPEN_HANDLERS; contextNum++ )
	 {
      if( pBtContext[contextNum].processId == 0 )
       break; 
	 }
	}

   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, SharedMem = 0x%X\r\n",__FUNCTION__, (void *)pBtContext));
  }
  else
  {
   RETAILMSG(1, (L"HS_DRV:%S, MapViewOfFile Failed\r\n",__FUNCTION__));
   CloseHandle(hMap);
   return(NULL);
  }

#if 0
  // For test only
  gTestVarCnt++;
  gcTestVarCnt++;
  lTestVarCnt++;
  RETAILMSG(1, (L"HS_DRV:%S, gTestVarCnt (0x%X) = %d\r\n",__FUNCTION__, &gTestVarCnt, gTestVarCnt));
  RETAILMSG(1, (L"HS_DRV:%S, gcTestVarCnt (0x%X) = %d\r\n",__FUNCTION__, &gcTestVarCnt, gcTestVarCnt));
  RETAILMSG(1, (L"HS_DRV:%S, lTestVarCnt (0x%X) = %d\r\n",__FUNCTION__, &lTestVarCnt, lTestVarCnt));
#endif

  if( contextNum >= MAX_OPEN_HANDLERS )
  {
   RETAILMSG(1, (L"HS_DRV:%S, contextNum exceed maximum\r\n",__FUNCTION__));
   UnmapViewOfFile((void *)pBtContext);
   CloseHandle(hMap);
   return(NULL);
  }
  else
  {
   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, Found context num = %d\r\n",__FUNCTION__, contextNum));
  }

  
  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   UnmapViewOfFile((void *)pBtContext);
   CloseHandle(hMap);
   return(NULL);
  }

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   UnmapViewOfFile((void *)pBtContext);
   CloseHandle(hMap);
   CloseHandle(BthAGmutex);
   return(NULL);
  }


  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, contextNum = %d, SharedMem[%d] = 0x%X\r\n",__FUNCTION__,  contextNum, contextNum, &pBtContext[contextNum] ));

  pBtContext[contextNum].contextNum = contextNum;
  pBtContext[contextNum].hMap = hMap;
  pBtContext[contextNum].pSharedMem = pBtContext;

  //gBtContext[contextNum].contextNum = contextNum;

  memset(&msgOptions, 0, sizeof(msgOptions));
  msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
  msgOptions.dwFlags = 0;
  msgOptions.cbMaxMessage = sizeof(AGHS_MSG);
  msgOptions.bReadAccess = 0;

			// initialize input specific data
  memset(pBtContext[contextNum].hsQueueName, 0, sizeof(pBtContext[contextNum].hsQueueName));
  wsprintf(pBtContext[contextNum].hsQueueName, L"%s%d%s", AGHS_QUEUE_NAME, contextNum, L"W"); 

  // create write msg queue
  pBtContext[contextNum].hsQueue = CreateMsgQueue(pBtContext[contextNum].hsQueueName, &msgOptions);
  if( pBtContext[contextNum].hsQueue == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, create write msg failure, contextNum = %d\r\n",__FUNCTION__, contextNum));
   UnmapViewOfFile((void *)pBtContext);
   CloseHandle(hMap);
   ReleaseMutex(BthAGmutex);
   CloseHandle(BthAGmutex);
   return(NULL);
  }

  pBtContext[contextNum].bRes = TRUE;
  //memcpy(btContextOut.hsQueueName, pBtContext[contextNum].hsQueueName, sizeof(pBtContext[contextNum].hsQueueName));

  pBtContext[contextNum].hsEventMask = 0;

  pBtContext[contextNum].processId = GetCurrentProcessId();


  RETAILMSG(HS_DRV_DBG_PRN, (L"  HS_DRV:%S, context Num = %d\r\n",__FUNCTION__,pBtContext[contextNum].contextNum));
  RETAILMSG(HS_DRV_DBG_PRN, (L"  HS_DRV:%S, Queue Name = %s\r\n",__FUNCTION__, pBtContext[contextNum].hsQueueName));
  RETAILMSG(HS_DRV_DBG_PRN, (L"  HS_DRV:%S, Queue handle = 0x%x\r\n",__FUNCTION__, pBtContext[contextNum].hsQueue));

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S\r\n",__FUNCTION__));

  //UnmapViewOfFile((void *)pBtContext);
  //CloseHandle(hMap);
  ReleaseMutex(BthAGmutex);

  return(&pBtContext[contextNum] /*btContextOut*/);
}


INT32  BthAGCloseDevice( HANDLE pDevContext )
{
 DWORD                     dwErr;
 DWORD                     contextNum;
 BT_DEVICE_CONTEXT_AG_HS   *pBtContext;
 BT_DEVICE_CONTEXT_AG_HS   *pContext;
 INT32                     contextCnt;
 HANDLE                    BthAGmutex;
 HANDLE                    hMap;
 void                      *pSharedMem;

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S, pDevContext = 0x%X\r\n",__FUNCTION__, pDevContext));

  if( pDevContext == NULL ) 
   return(-1);

  pContext = (BT_DEVICE_CONTEXT_AG_HS *)pDevContext;

  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   return(-2);
  }


  if( !BthAGCheckContext(pContext, &contextNum) )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Context Failure\r\n",__FUNCTION__));
   //UnmapViewOfFile(pContext->hMap);
   CloseHandle(BthAGmutex);
  }

  hMap = pContext->hMap;
  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, pBtContext(hMap) = 0x%X\r\n",__FUNCTION__, pContext->hMap));

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   UnmapViewOfFile(pContext->pSharedMem);
   CloseHandle(BthAGmutex);
   return(-3);
  }

  
	  if( pContext->hsQueue != NULL )
	  {
       RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, CloseMsgQueue & it's Handle\r\n",__FUNCTION__)); 
       CloseMsgQueue(pContext->hsQueue);
	   CloseHandle(pContext->hsQueue);
	  }

	  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, Close Context[# %d] = 0x%X\r\n",__FUNCTION__, pContext->contextNum, pContext)); 
	  pSharedMem = pContext->pSharedMem;
      memset( pContext, 0, sizeof(BT_DEVICE_CONTEXT_AG_HS) );

	  // Look for remained contexts
	  pBtContext = (BT_DEVICE_CONTEXT_AG_HS *)pSharedMem;
	  contextCnt = 0;
	  for( contextNum = 0; contextNum < MAX_OPEN_HANDLERS; contextNum++ )
	  {
	   if( pBtContext[contextNum].processId != 0 )
		contextCnt++;
	  }

  
  UnmapViewOfFile(pContext->pSharedMem);
  if( contextCnt == 0 )
   CloseHandle(hMap);

  ReleaseMutex(BthAGmutex);
  CloseHandle(BthAGmutex);

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S, contextCnt = %d\r\n",__FUNCTION__, contextCnt));
  return(contextCnt);
}



BOOL BthAGSendEvent(DWORD dwEvent, DWORD dwParam)
{
 HANDLE                    hMap;
 DWORD                     hMapErr;
 DWORD                     dwSize, dwErr;
 DWORD                     contextNum;
 BT_DEVICE_CONTEXT_AG_HS   *pBtContext;
 HANDLE                    BthAGmutex;
 AGHS_MSG                  AgMsg;
 MSGQUEUEOPTIONS		   msgOptions;

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S, event = 0x%X, param = 0x%X\r\n",__FUNCTION__, dwEvent, dwParam));

  // Empty event
  if( dwEvent == 0 )
  {
   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S, Empty event\r\n",__FUNCTION__));
   return(TRUE);
  }

  // Only one AG event per message
  if( ( (dwEvent & (dwEvent-1)) != 0 ) ||
	  ( (dwEvent & EV_AG_ALL_MASK)== 0 )
	)
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid event = 0x%X\r\n",__FUNCTION__, dwEvent));
   return(FALSE);
  }


  dwSize = (MAX_OPEN_HANDLERS * sizeof(BT_DEVICE_CONTEXT_AG_HS)) + SHARED_MEM_SCRATCH_PAD_SIZE;

  hMap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, dwSize, AGHS_MAPVIEW_NAME);
  hMapErr = GetLastError();

  if( hMap != NULL )
  {
   pBtContext = (BT_DEVICE_CONTEXT_AG_HS *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0,0, dwSize );
   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, CreateFileMapping Ok, err = %d, addr = 0x%X\r\n",__FUNCTION__, hMapErr, pBtContext));
  }
  else
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateFileMapping Failed, err = %d\r\n",__FUNCTION__, hMapErr));
   return(FALSE);
  }

  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   UnmapViewOfFile((void *)pBtContext);
   CloseHandle(hMap);
   CloseHandle(BthAGmutex);
   return(FALSE);
  }

  if( (void *)pBtContext != NULL )
  {
    if( hMapErr != ERROR_ALREADY_EXISTS )
	{
     // No Device was opened
     RETAILMSG(1, (L"HS_DRV:%S, No Device was opened\r\n",__FUNCTION__));
     return( BthAGSendEventExit(pBtContext, hMap, BthAGmutex, FALSE) );
	}
	else
	{
     for( contextNum = 0; contextNum < MAX_OPEN_HANDLERS; contextNum++ )
	 {
      if( pBtContext[contextNum].processId != 0 )
	  {
        BOOL  bIsBad;
        // Verify Context
		bIsBad = IsBadReadPtr( &pBtContext[contextNum], sizeof(BT_DEVICE_CONTEXT_AG_HS) ); 
        if( ( bIsBad )||
			( pBtContext[contextNum].contextNum != contextNum ) ||
		    ( pBtContext[contextNum].pSharedMem != (void *)pBtContext )
		  )
		{
		 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, Invalid context # %d\r\n",__FUNCTION__, contextNum));
		 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, bad ptr = %d, member contextNum = %d, shared mem = 0x%X - 0x%X\r\n",__FUNCTION__, 
			            bIsBad, pBtContext[contextNum].contextNum, pBtContext[contextNum].pSharedMem, (void *)pBtContext));

		 return( BthAGSendEventExit(pBtContext, hMap, BthAGmutex, FALSE) );	 
		}
		else
		{
         if( (pBtContext[contextNum].hsQueue != NULL) && 
			 ( pBtContext[contextNum].hsEventMask & dwEvent )
		   )
		 {
          // here, origin msg handle is invalid because this function called from other instance !
		  memset(&msgOptions, 0, sizeof(msgOptions));
		  msgOptions.dwSize = sizeof(MSGQUEUEOPTIONS);
		  msgOptions.dwFlags = 0;
		  msgOptions.cbMaxMessage = sizeof(AGHS_MSG);
		  msgOptions.bReadAccess = 0;

					// initialize input specific data
		  memset(pBtContext[contextNum].hsQueueName, 0, sizeof(pBtContext[contextNum].hsQueueName));
		  wsprintf(pBtContext[contextNum].hsQueueName, L"%s%d%s", AGHS_QUEUE_NAME, contextNum, L"W"); 

		  // create write msg queue
		  pBtContext[contextNum].hsQueue = CreateMsgQueue(pBtContext[contextNum].hsQueueName, &msgOptions);
		  if( pBtContext[contextNum].hsQueue == NULL )
		  {
		   RETAILMSG(1, (L"HS_DRV:%S, get write msg handle failure, contextNum = %d\r\n",__FUNCTION__, contextNum));
		   return( BthAGSendEventExit(pBtContext, hMap, BthAGmutex, FALSE) );
		  }
		    

          AgMsg.hsEvent = dwEvent;
          AgMsg.dwParam = dwParam; 
          if( !WriteMsgQueue(pBtContext[contextNum].hsQueue, &AgMsg, sizeof(AgMsg), 0, 0) )
		  {
            dwErr = GetLastError();
			RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, WriteMsgQueue failed, ret = %d, hsQueue = 0x%X\r\n",__FUNCTION__, dwErr, pBtContext[contextNum].hsQueue ));
			switch(dwErr)
			{
			 case   ERROR_INSUFFICIENT_BUFFER: 
				 {
					 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, WriteMsgQueue failed, 'ERROR_INSUFFICIENT_BUFFER'\r\n",__FUNCTION__));
				 } break;

			 case   ERROR_OUTOFMEMORY:
				 {
					 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, WriteMsgQueue failed, 'ERROR_OUTOFMEMORY'\r\n",__FUNCTION__));
				 } break;

			 case   ERROR_INVALID_HANDLE:
				 {
					 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, WriteMsgQueue failed, 'ERROR_INVALID_HANDLE'\r\n",__FUNCTION__));
				 } break;

			 case   ERROR_TIMEOUT:
				 {
					 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, WriteMsgQueue failed, 'ERROR_TIMEOUT'\r\n",__FUNCTION__));
				 } break;

			 default:
				 {
					 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, WriteMsgQueue failed, 'Unknown Error'\r\n",__FUNCTION__));
				 } break;

			}
          
		  }
		  else
		  {
		   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, Queue Message Sent\r\n",__FUNCTION__));
		  }

		  CloseHandle(pBtContext[contextNum].hsQueue);
		 }
		 else
		 {
          //RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, get write msg handle failure, contextNum = %d\r\n",__FUNCTION__, contextNum));
		  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, Queue handle = 0x%X, Queue Event Mask = 0x%X, Param = %d\r\n",__FUNCTION__, pBtContext[contextNum].hsQueue, pBtContext[contextNum].hsEventMask));
		 }
		}
	  }
	 } //for( contextNum = 0; contextNum < MAX_OPEN_HANDLERS; contextNum++ )
	}
  }
  else
  {
   RETAILMSG(1, (L"HS_DRV:%S, MapViewOfFile Failed\r\n",__FUNCTION__));
   return( BthAGSendEventExit(pBtContext, hMap, BthAGmutex, FALSE) );
  }

  
  if( dwErr == ERROR_SUCCESS )
    return( BthAGSendEventExit(pBtContext, hMap, BthAGmutex, TRUE) );
  else
    return( BthAGSendEventExit(pBtContext, hMap, BthAGmutex, FALSE) );

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S, dwErr = %d\r\n",__FUNCTION__, dwErr));
}

static BOOL BthAGSendEventExit(void *pBtContext, HANDLE hMap, HANDLE BthAGmutex, BOOL bRet)
{
  UnmapViewOfFile(pBtContext);
  CloseHandle(hMap);
  ReleaseMutex(BthAGmutex);
  CloseHandle(BthAGmutex);
  return(bRet);   
}



BOOL BthAGWriteSharedMem(void *pSharedMem, DWORD offset, void *pData, DWORD length)
{ 
 DWORD                     dwErr;
 HANDLE                    BthAGmutex;

 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S\r\n",__FUNCTION__));

 if( pSharedMem == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid parameter, pSharedMem = NULL\r\n",__FUNCTION__));
   return(FALSE);
  }

 if( pData == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid parameter, pData = NULL\r\n",__FUNCTION__));
   return(FALSE);
  }

 if( (offset + length) > SHARED_MEM_SCRATCH_PAD_SIZE )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid parameters, offset or/and length\r\n",__FUNCTION__));
   return(FALSE);
  }
  
  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  offset += (MAX_OPEN_HANDLERS * sizeof(BT_DEVICE_CONTEXT_AG_HS));
  // CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
  memcpy((char *)pSharedMem + offset, pData, length);

  ReleaseMutex(BthAGmutex);

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S\r\n",__FUNCTION__));
  return(TRUE);
}


BOOL BthAGReadSharedMem(void *pSharedMem, DWORD offset, void *pData, DWORD length)
{ 
 DWORD                     dwErr;
 HANDLE                    BthAGmutex;


 RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S\r\n",__FUNCTION__));

 if( pSharedMem == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid parameter, pSharedMem = NULL\r\n",__FUNCTION__));
   return(FALSE);
  }

 if( pData == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid parameter, pData = NULL\r\n",__FUNCTION__));
   return(FALSE);
  }

 if( (offset + length) > SHARED_MEM_SCRATCH_PAD_SIZE )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid parameters, offset or/and length\r\n",__FUNCTION__));
   return(FALSE);
  }
  
  BthAGmutex = CreateMutex(NULL, FALSE, AGHS_MUTEX_NAME);
  if( BthAGmutex == NULL )
  {
   RETAILMSG(1, (L"HS_DRV:%S, CreateMutex Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  dwErr = WaitForSingleObject(BthAGmutex, INFINITE);
  if( dwErr != WAIT_OBJECT_0 )
  {
   RETAILMSG(1, (L"HS_DRV:%S, WaitForSingleObject Failure\r\n",__FUNCTION__));
   return(FALSE);
  }

  offset += (MAX_OPEN_HANDLERS * sizeof(BT_DEVICE_CONTEXT_AG_HS));
  // CeSafeCopyMemory( pOutBuf, &dwValue, sizeof(DWORD) );
  //memcpy((char *)hMap + offset, pData, length);
  memcpy( pData, (char *)pSharedMem + offset, length);

  ReleaseMutex(BthAGmutex);

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S\r\n",__FUNCTION__));
  return(TRUE);
}


static BT_DEVICE_CONTEXT_AG_HS  *BthAGCheckContext( HANDLE pDevContext, DWORD *pContextNum )
{
 DWORD                     dwSize;
 DWORD                     contextNum;
 BT_DEVICE_CONTEXT_AG_HS  *pBtContext;
 BT_DEVICE_CONTEXT_AG_HS  *pContext;

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:+%S, pDevContext = 0x%X\r\n",__FUNCTION__, pDevContext));

  if( pDevContext == NULL )
   return(NULL);

  if( pContextNum == NULL )
   return(NULL);

  pContext = (BT_DEVICE_CONTEXT_AG_HS *)pDevContext;
  dwSize = sizeof(BT_DEVICE_CONTEXT_AG_HS);

  if( IsBadReadPtr(pContext, dwSize ) )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Shared memory access Failed\r\n",__FUNCTION__));
   return(NULL);
  }

  // Look for context
  pBtContext = (BT_DEVICE_CONTEXT_AG_HS *)pContext->pSharedMem;
  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, shared mem (pBtContext) = 0x%X\r\n",__FUNCTION__, pBtContext));

  for( contextNum = 0; contextNum < MAX_OPEN_HANDLERS; contextNum++ )
  {
   RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, pBtContext[%d] = 0x%X\r\n",__FUNCTION__, contextNum, &pBtContext[contextNum]));

   if( ( &pBtContext[contextNum] == pContext ) &&
	   ( pBtContext[contextNum].contextNum == pContext->contextNum )&&
	   ( pBtContext[contextNum].processId == pContext->processId ) 
	 )
    break;
  }

  if( contextNum >= MAX_OPEN_HANDLERS )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Context not found\r\n",__FUNCTION__));
   return(NULL);
  }

  // Verify by process Id
  if( pContext->processId != GetCurrentProcessId() )
  {
   RETAILMSG(1, (L"HS_DRV:%S, Invalid Context\r\n",__FUNCTION__));
   return(NULL);
  }

  *pContextNum = contextNum;

  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:%S, Found: pBtContext[%d] = 0x%X\r\n",__FUNCTION__, contextNum, &pBtContext[contextNum]));
  RETAILMSG(HS_DRV_DBG_PRN, (L"HS_DRV:-%S\r\n",__FUNCTION__));
  return(&pBtContext[contextNum]);
}