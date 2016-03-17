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
/** \file mainKeysSm.h
 *  \brief RSN main security SM
 *
 *  \see mainKeysSm.c
 */


/***************************************************************************/
/*																		   */
/*		MODULE:	mainKeysSm.h												   */
/*    PURPOSE:	RSN main security SM									   */
/*																	 	   */
/***************************************************************************/

#ifndef _MAIN_KEYS_INTERNAL_H
#define _MAIN_KEYS_INTERNAL_H

#include "paramOut.h"
#include "rsnApi.h"

#include "mainKeysSm.h"

/* Constants */


/* Enumerations */

/* Typedefs */

/** state machine states */
typedef enum 
{
	MAIN_KEYS_STATE_IDLE                  = 0,	
	MAIN_KEYS_STATE_START                 = 1,	
	MAIN_KEYS_STATE_UNICAST_COMPLETE      = 2,		
	MAIN_KEYS_STATE_BROADCAST_COMPLETE    = 3,	
	MAIN_KEYS_STATE_COMPLETE              = 4,
    MAIN_KEYS_NUM_STATES
} mainKeys_smStates;

/** State machine events */
typedef enum 
{
	MAIN_KEYS_EVENT_START					= 0,
	MAIN_KEYS_EVENT_STOP					= 1,
	MAIN_KEYS_EVENT_UCAST_COMPLETE			= 2,
	MAIN_KEYS_EVENT_BCAST_COMPLETE			= 3,
	MAIN_KEYS_EVENT_SESSION_TIMEOUOT		= 4,
    MAIN_KEYS_NUM_EVENTS
} mainKeys_smEvents;


/* Main Sec SM functions */

/* Structures */


/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS mainKeys_start(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_stop(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_reportUcastStatus(struct _mainKeys_t *pMainKeys, TI_STATUS ucastStatus);
TI_STATUS mainKeys_reportBcastStatus(struct _mainKeys_t *pMainKeys, TI_STATUS bcastStatus);
void mainKeys_sessionTimeout(void *pMainKeys, TI_BOOL bTwdInitOccured);
TI_STATUS mainKeys_setKey(struct _mainKeys_t *pMainKeys, TSecurityKeys *pKey);
TI_STATUS mainKeys_removeKey(struct _mainKeys_t *pMainKeys, TSecurityKeys *pKey);
TI_STATUS mainKeys_setDefaultKeyId(struct _mainKeys_t *pMainKeys, TI_UINT8 keyId);
TI_STATUS mainKeys_getSessionKey(struct _mainKeys_t *pMainKeys, TI_UINT8 *pKey, TI_UINT32 *pKeyLen);

TI_STATUS mainKeys_startIdle(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_stopStart(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_stopUcastComplete(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_bcastCompleteUcastComplete(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_smTimeOut(void* data);
TI_STATUS mainKeys_stopBcastComplete(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_ucastCompleteBcastComplete(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeys_stopComplete(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeySmUnexpected(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeySmNop(struct _mainKeys_t *pMainKeys);
TI_STATUS mainKeySmSetKeyCompleted(struct _mainKeys_t *pMainKeys);




#endif

