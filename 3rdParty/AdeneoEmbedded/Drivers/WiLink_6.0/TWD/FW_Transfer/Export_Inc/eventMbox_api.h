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

/****************************************************************************
 *
 *   MODULE:  eventMbox_api.h : event Mail Box API
 *   PURPOSE: Notify EventMbox in case of an incoming event from the FW
 *
 ****************************************************************************/
#ifndef _EVENT_MBOX_API_H
#define _EVENT_MBOX_API_H


#include "TWDriverInternal.h"


/*
 *  TEventMboxDataCb : This Call back is for EventMbox Client 
 *                     that expect an event with Data associated  
 *                     str:    The Data String
 *                     strLen : The Length of the Data
 */
typedef void (*TEventMboxDataCb)(TI_HANDLE hCb, TI_CHAR* str, TI_UINT32 strLen);

/*
 *  TEventMboxEvCb : This Call back is for EventMbox Client 
 *                   that expect an event without any Data
 */
typedef void (*TEventMboxEvCb)(TI_HANDLE hCb);


/*****************************************************************************
 **         API                                                            **
 *****************************************************************************/

TI_HANDLE eventMbox_Create          (TI_HANDLE hOs);
void      eventMbox_Destroy			(TI_HANDLE hEventMbox);
void      eventMbox_Stop			(TI_HANDLE hEventMbox);
void      eventMbox_Config			(TI_HANDLE hEventMbox,
									 TI_HANDLE hTwif,
									 TI_HANDLE hReport,
									 TI_HANDLE hFwEvent,
									 TI_HANDLE hCmdBld);
TI_STATUS eventMbox_InitMboxAddr	(TI_HANDLE hEventMbox, fnotify_t fCb, TI_HANDLE hCb);
void      eventMbox_InitComplete	(TI_HANDLE hEventMbox);
TI_STATUS eventMbox_RegisterEvent	(TI_HANDLE hEventMbox, TI_UINT32 EvID, void *fCb, TI_HANDLE hCb);
TI_STATUS eventMbox_ReplaceEvent	(TI_HANDLE hEventMbox,
                                     TI_UINT32   EvID, 
                                     void       *fNewCb, 
                                     TI_HANDLE   hNewCb,                                    
                                     void      **pPrevCb, 
                                     TI_HANDLE  *pPrevHndl);                                
TI_STATUS eventMbox_UnMaskEvent		(TI_HANDLE hEventMbox, TI_UINT32 EvID, void *fCb, TI_HANDLE hCb);
TI_STATUS eventMbox_MaskEvent		(TI_HANDLE hEventMbox, TI_UINT32 EvID, void *fCb, TI_HANDLE hCb);
ETxnStatus      eventMbox_Handle		   	(TI_HANDLE hEventMbox, FwStatus_t *pFwStatus);
#ifdef TI_DBG
TI_STATUS eventMbox_Print           (TI_HANDLE hEventMbox);
#endif

#endif /* _EVENT_MBOX_API_H */
