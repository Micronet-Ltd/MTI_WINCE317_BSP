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

#ifndef _MAIN_KEYS_SM_H
#define _MAIN_KEYS_SM_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"
#include "keyParser.h"
#include "unicastKeySM.h"
#include "broadcastKeySM.h"

/* Constants */

#define MAIN_KEYS_TIMEOUT			20000

/* Enumerations */

/* Typedefs */

typedef struct _mainKeys_t    mainKeys_t;

/* Main Sec SM functions */
typedef TI_STATUS (*mainKeysSmStart_t)(struct _mainKeys_t *pMainKeys);
typedef TI_STATUS (*mainKeysSmStop_t)(struct _mainKeys_t *pMainKeys);
typedef TI_STATUS (*mainKeysSmReportUcastStatus_t)(struct _mainKeys_t *pMainKeys, TI_STATUS authStatus);
typedef TI_STATUS (*mainKeysSmReportBcastStatus_t)(struct _mainKeys_t *pMainKeys, TI_STATUS authStatus);
typedef TI_STATUS (*mainKeysSmReportReKey_t)(struct _mainKeys_t *pMainKeys);
typedef TI_STATUS (*mainKeysSmSetKey_t)(struct _mainKeys_t *pMainKeys, TSecurityKeys *pKey);
typedef TI_STATUS (*mainKeysSmRemoveKey_t)(struct _mainKeys_t *pMainKeys, TSecurityKeys *pKey);
typedef TI_STATUS (*mainKeysSmSetDefaultKeyId_t)(struct _mainKeys_t *pMainKeys, TI_UINT8 keyId);
typedef TI_STATUS (*mainKeysSmGetSessionKey_t)(struct _mainKeys_t *pMainKeys, TI_UINT8 *pKey, TI_UINT32 *pKeyLen);

/* Structures */

typedef struct
{
	TI_STATUS		status;
} mainKeysData_t;

struct _mainKeys_t
{
	TI_UINT8                            currentState;
	TI_UINT32                           keysTimeout;
    mainKeysData_t						data;
	fsm_stateMachine_t	                *pMainKeysSm;
    TI_BOOL			 	                mainKeysTimeoutCounter;

	TI_HANDLE							hCtrlData;
	TI_HANDLE			                hReport;
	TI_HANDLE			                hOs;
    TI_HANDLE                           hEvHandler;
    TI_HANDLE                           hConn;
    TI_HANDLE                           hRsn;
	TI_HANDLE							hTimer;

    TI_HANDLE							hSessionTimer;

	keyParser_t							*pKeyParser;
	unicastKey_t						*pUcastSm;
	broadcastKey_t						*pBcastSm;
    struct _mainSec_t                  	*pParent;

    mainKeysSmStart_t                   start;
    mainKeysSmStop_t                    stop;
    mainKeysSmReportUcastStatus_t		reportUcastStatus;
    mainKeysSmReportBcastStatus_t       reportBcastStatus;
    mainKeysSmReportReKey_t				reportReKey;
	mainKeysSmSetKey_t					setKey; 
	mainKeysSmRemoveKey_t				removeKey;
	mainKeysSmSetDefaultKeyId_t			setDefaultKeyId;
	mainKeysSmGetSessionKey_t			getSessionKey;
};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

mainKeys_t* mainKeys_create(TI_HANDLE hOs);

TI_STATUS mainKeys_unload(mainKeys_t *pmainKeys);

TI_STATUS mainKeys_config (mainKeys_t    *pMainKeys, 
                           TRsnPaeConfig *pPaeConfig, 
                           void          *pParent,
                           TI_HANDLE      hReport,
                           TI_HANDLE      hOs,
                           TI_HANDLE      hCtrlData,
                           TI_HANDLE      hEvHandler,
                           TI_HANDLE      hConn,
                           TI_HANDLE      hRsn,
                           TI_HANDLE      hTimer);

void mainKeys_reAuth(TI_HANDLE pHandle);

#endif

