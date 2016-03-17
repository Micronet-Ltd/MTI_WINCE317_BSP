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
/** \file MainSecSm.h
 *  \brief RSN main security SM
 *
 *  \see MainSecSm.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	MainSecSm.h												   */
/*    PURPOSE:	RSN main security SM									   */
/*																	 	   */
/***************************************************************************/

#ifndef _MAIN_SEC_KEYS_ONLY_H
#define _MAIN_SEC_KEYS_ONLY_H

#include "fsm.h"
#include "mainSecSm.h"

/* Constants */

/** number of events in the state machine */
#define	MAIN_SEC_KEYS_ONLY_NUM_EVENTS		4

/** number of states in the state machine */
#define	MAIN_SEC_KEYS_ONLY_NUM_STATES		4

/* Enumerations */

/* state machine states */
typedef enum 
{
	MAIN_KO_STATE_IDLE				= 0,
	MAIN_KO_STATE_START				= 1,
	MAIN_KO_STATE_AUTHORIZED		= 2,
	MAIN_KO_STATE_NONAUTHORIZED		= 3
} main_keysOnlyStates;

/* State machine inputs */
typedef enum 
{
	MAIN_KO_EVENT_START				= 0,
	MAIN_KO_EVENT_STOP				= 1,
	MAIN_KO_EVENT_KEYS_COMPLETE		= 2,
	MAIN_KO_EVENT_SEC_ATTACK		= 3
} main_keysOnlyEvents;


/* Typedefs */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS mainSecKeysOnly_config(mainSec_t *pMainSec, 
                            TRsnPaeConfig *pPaeConfig);

TI_STATUS mainSecKeysOnly_start(mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_stop(mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_setSessionKey(mainSec_t *pMainSec, TI_UINT8* pKey, TI_UINT8 keyLen);

TI_STATUS mainSecKeysOnly_getSessionKey(mainSec_t *pMainSec, TI_UINT8* pKey, TI_UINT32* pKeyLen);

TI_STATUS mainSecKeysOnly_reportKeysStatus(mainSec_t *pMainSec, TI_STATUS keyStatus);

/* state machine functions */

TI_STATUS mainSecKeysOnly_startIdle(struct _mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_stopStart(struct _mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_keysCompleteStart(struct _mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_keysTOStart(struct _mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_stopAuthorized(struct _mainSec_t *pMainSec);

TI_STATUS mainSecKeysOnly_stopNonAuthorized(struct _mainSec_t *pMainSec);

/* state machine action functions */

TI_STATUS mainSecKeysOnly_StartMainKeySm(void* pData);

TI_STATUS mainSecKeysOnly_StopMainKeySm(void* pData);

TI_STATUS mainSecKeysOnly_ReportAuthSuccess(void* pData);

TI_STATUS mainSecKeysOnly_ReportAuthFailure(void* pData);

TI_STATUS mainSecKeysOnly_Nop(void* pData);

TI_STATUS mainSecKeysOnly_unexpected(void* pData);

#endif

