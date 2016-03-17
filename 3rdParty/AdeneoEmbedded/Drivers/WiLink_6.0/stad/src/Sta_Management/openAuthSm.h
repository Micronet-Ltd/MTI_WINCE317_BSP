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
/** \file assocSm.h
 *  \brief 802.11 authentication SM
 *
 *  \see assocSm.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	assocSm.h												   */
/*    PURPOSE:	802.11 authentication SM									   */
/*																	 	   */
/***************************************************************************/

#ifndef _OPEN_AUTH_SM_H
#define _OPEN_AUTH_SM_H

#include "fsm.h"
#include "mlmeApi.h"
#include "authSm.h"

/* Constants */

/* Enumerations */

/* state machine states */
typedef enum 
{
	OPEN_AUTH_SM_STATE_IDLE		= 0,
	OPEN_AUTH_SM_STATE_WAIT    	= 1,
	OPEN_AUTH_SM_STATE_AUTH		= 2
} openAuth_smStates_t;

/* State machine inputs */
typedef enum 
{
	OPEN_AUTH_SM_EVENT_START		= 0,
	OPEN_AUTH_SM_EVENT_STOP			= 1,
	OPEN_AUTH_SM_EVENT_SUCCESS    	= 2,
	OPEN_AUTH_SM_EVENT_FAIL			= 3,
	OPEN_AUTH_SM_EVENT_TIMEOUT		= 4,
	OPEN_AUTH_SM_EVENT_MAX_RETRY	= 5
} openAuth_smEvents_t;

/* Typedefs */

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS openAuth_Config(TI_HANDLE 	hAuth,
					   TI_HANDLE 	pOs);

TI_STATUS openAuth_Recv(TI_HANDLE pAssoc, mlmeFrameInfo_t *pFrame);

TI_STATUS openAuth_Timeout(auth_t *pAuth);

TI_STATUS auth_osSMEvent(TI_UINT8 *currentState, TI_UINT8 event, TI_HANDLE hAuth);

/* state machine functions */

TI_STATUS openAuth_smStartIdle(auth_t *pAuth);
TI_STATUS openAuth_smStopWait(auth_t *hAuth);
TI_STATUS openAuth_smSuccessWait(auth_t *hAuth);
TI_STATUS openAuth_smFailureWait(auth_t *hAuth);
TI_STATUS openAuth_smTimeoutWait(auth_t *hAuth);
TI_STATUS openAuth_smMaxRetryWait(auth_t *hAuth);
TI_STATUS openAuth_smStopAuth(auth_t *hAuth);
TI_STATUS openAuth_smActionUnexpected(auth_t *hAuth);

TI_STATUS openAuth_smResetRetry(auth_t *hAuth);
TI_STATUS openAuth_smIncRetry(auth_t *hAuth);
TI_STATUS openAuth_smReportSuccess(auth_t *hAuth);
TI_STATUS openAuth_smReportFailure(auth_t *hAuth);
TI_STATUS openAuth_smSendAuthReq(auth_t *hAuth);
TI_STATUS openAuth_smStartTimer(auth_t *hAuth);
TI_STATUS openAuth_smStopTimer(auth_t *hAuth);

#endif

