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

/** \file AssocSM.h
 *  \brief 802.11 Association SM
 *
 *  \see assocSM.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	AssocSM.h												   */
/*    PURPOSE:	802.11 Association SM									   */
/*																	 	   */
/***************************************************************************/

#ifndef _ASSOC_SM_H
#define _ASSOC_SM_H

#include "fsm.h"
#include "mlmeApi.h"

/* Constants */

#define MAX_ASSOC_MSG_LENGTH			(512 + RSN_MAX_GENERIC_IE_LENGTH)

#define ASSOC_SM_CAP_ESS_MODE			0x0001
#define ASSOC_SM_CAP_IBSS_MODE			0x0002
#define ASSOC_SM_CAP_CF_POLLABLE		0x0004
#define ASSOC_SM_CAP_CF_POLL_REQ		0x0008
#define ASSOC_SM_CAP_PIVACY				0x0010
#define ASSOC_SM_CAP_SHORT_PREAMBLE		0x0020
#define ASSOC_SM_CAP_PBCC				0x0040
#define ASSOC_SM_CAP_CH_AGILITY			0x0080

/* Enumerations */

/* state machine states */
typedef enum 
{
	ASSOC_SM_STATE_IDLE		= 0,
	ASSOC_SM_STATE_WAIT    	= 1,
	ASSOC_SM_STATE_ASSOC	= 2
} assoc_smStates_t;

/* State machine inputs */
typedef enum 
{
	ASSOC_SM_EVENT_START		= 0,
	ASSOC_SM_EVENT_STOP			= 1,
	ASSOC_SM_EVENT_SUCCESS    	= 2,
    ASSOC_SM_EVENT_FAIL			= 3,
   	ASSOC_SM_EVENT_TIMEOUT		= 4,
   	ASSOC_SM_EVENT_MAX_RETRY	= 5
} assoc_smEvents_t;



/* Typedefs */

typedef struct
{
	fsm_stateMachine_t	        *pAssocSm;
	TI_UINT32				        timeout;
	TI_UINT8				        currentState;
	TI_UINT32				        maxCount;
	TI_UINT8				        retryCount;
	TI_UINT32				        assocRejectCount;
	TI_UINT32				        assocTimeoutCount;
	char				        *pChalange;
    TI_UINT8                       assocRespBuffer[MAX_ASSOC_MSG_LENGTH];
    TI_UINT32                      assocRespLen;
    TI_UINT8                       assocReqBuffer[MAX_ASSOC_MSG_LENGTH];
    TI_UINT32                      assocReqLen;
  	TI_BOOL					       reAssocResp;

	TI_BOOL						reAssoc;
	TI_BOOL 						disAssoc; /* When set dissasociation frame will be sent. */

	TI_HANDLE			        hMlme;
	TI_HANDLE			        hSiteMgr;
	TI_HANDLE			        hCtrlData;
	TI_HANDLE			        hTWD;
	TI_HANDLE			        hRsn;
	TI_HANDLE			        hReport;
	TI_HANDLE			        hOs;
	TI_HANDLE					hRegulatoryDomain;
	TI_HANDLE			        hXCCMngr;
	TI_HANDLE			        hQosMngr;
    TI_HANDLE			        hMeasurementMgr;
	TI_HANDLE			        hApConn;
	TI_HANDLE			        hTimer;
     	TI_HANDLE			        hStaCap;
	TI_HANDLE			        hSme;

	TI_HANDLE			        hAssocSmTimer;
} assoc_t;

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS assoc_start(TI_HANDLE hAssoc);

TI_STATUS reassoc_start(TI_HANDLE hAssoc);

TI_STATUS assoc_stop(TI_HANDLE hAssoc);

TI_STATUS assoc_recv(TI_HANDLE hAssoc, mlmeFrameInfo_t *pFrame);

TI_STATUS assoc_setDisAssocFlag(TI_HANDLE hAssoc, TI_BOOL disAsoccFlag);

TI_STATUS assoc_smCapBuild(assoc_t *pCtx, TI_UINT16 *cap);


/* local functions */

TI_STATUS assoc_saveAssocRespMessage(assoc_t *pAssocSm, TI_UINT8 *pAssocBuffer, TI_UINT32 length);


#endif

