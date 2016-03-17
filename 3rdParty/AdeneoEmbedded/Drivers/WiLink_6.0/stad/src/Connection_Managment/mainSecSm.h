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

#ifndef _MAIN_SEC_SM_H
#define _MAIN_SEC_SM_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "rsn.h"
#include "keyTypes.h"

/* Constants */

/* Enumerations */

/* Typedefs */

typedef struct _mainSec_t    mainSec_t;

/* Main Sec SM functions */
typedef TI_STATUS (*mainSecSmStart_t)(struct _mainSec_t *pMainSec);
typedef TI_STATUS (*mainSecSmStop_t)(struct _mainSec_t *pMainSec);
typedef TI_STATUS (*mainSecSmReportAuthStatus_t)(struct _mainSec_t *pMainSec, TI_STATUS authStatus);
typedef TI_STATUS (*mainSecSmReportKeysStatus_t)(struct _mainSec_t *pMainSec, TI_STATUS keyStatus);
typedef TI_STATUS (*mainSecSmReportReauthentication_t)(struct _mainSec_t *pMainSec);
typedef TI_STATUS (*mainSecSmSetSessionKey_t)(struct _mainSec_t *pMainSec, TI_UINT8* pKey, TI_UINT8 keyLen);
typedef TI_STATUS (*mainSecSmGetSessionKey_t)(struct _mainSec_t *pMainSec, TI_UINT8* pKey, TI_UINT32* pKeyLen);
typedef TI_STATUS (*mainSecSmSetKey_t)(struct _mainSec_t *pMainSec, TSecurityKeys *pKey);
typedef TI_STATUS (*mainSecSmRemoveKey_t)(struct _mainSec_t *pMainSec, TSecurityKeys *pKey);
typedef TI_STATUS (*mainSecSmSetDefaultKeyId_t)(struct _mainSec_t *pMainSec, TI_UINT8 keyId);
typedef TI_STATUS (*mainSecSmEapolRecv_t)(struct _mainSec_t *pMainSec, TI_UINT8 *pPacket);
typedef TI_STATUS (*mainSecSm_getAuthIdentity_t)(struct _mainSec_t *pMainSec, authIdentity_t *authIdentity);
typedef TI_STATUS (*mainSecSm_getAuthState_t)(struct _mainSec_t *pMainSec, TIWLN_SECURITY_STATE *secState);
typedef TI_STATUS (*mainSecSm_reportAuthFailure_t)(struct _mainSec_t *pMainSec, EAuthStatus authStatus);

/* Structures */

typedef struct
{
	TRsnPaeConfig   		*pPaeConfig;
	TI_UINT32				heldPeriod;
	TI_UINT32				startPeriod;
	TI_UINT32				authPeriod;
	TI_UINT32				maxStart;
	TI_UINT8				*pUserId;
	TI_UINT8				idLen;
	TI_UINT8				*pUserPasswd;
	TI_UINT8				pwdLen;
} mainSecInitData_t;

typedef struct
{
	TI_STATUS		status;
} mainSecData_t;

struct _mainSec_t
{
	TI_UINT8                            currentState;
    TI_UINT32				            startCount;
	fsm_stateMachine_t	                *pMainSecSm;
	TI_BOOL				                PrivacyOptionImplemented;
    mainSecData_t						data;
	TI_UINT8							sessionKey[SESSION_KEY_LEN];
	TI_UINT8							sessionKeyLen;

    struct _rsn_t                      	*pParent;
	TI_HANDLE			                hReport;
	TI_HANDLE			                hOs;
	
    struct _mainKeys_t                  *pMainKeys; 
    struct _keyParser_t                 *pKeyParser;
    struct externalSec_t                *pExternalSec;
    mainSecSmStart_t                    start;
    mainSecSmStop_t                     stop;
    mainSecSmReportAuthStatus_t         reportAuthStatus;
    mainSecSmReportKeysStatus_t       	reportKeysStatus;
    mainSecSmReportReauthentication_t   reportReAuthenticate;
	mainSecSmSetSessionKey_t			setSessionKey;
	mainSecSmGetSessionKey_t			getSessionKey;
	mainSecSmSetKey_t					setKey;
	mainSecSmRemoveKey_t				removeKey;
	mainSecSmSetDefaultKeyId_t			setDefaultKeyId;
    mainSecSm_getAuthIdentity_t  		getAuthIdentity;
    mainSecSm_getAuthIdentity_t  		setAuthIdentity;
	mainSecSm_getAuthState_t 			getAuthState;
    mainSecSm_reportAuthFailure_t       reportAuthFailure;

};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

mainSec_t* mainSec_create(TI_HANDLE hOs);

TI_STATUS mainSec_unload(mainSec_t *pMainSec);

TI_STATUS mainSec_config (mainSec_t *pMainSec, 
                          mainSecInitData_t *pInitData, 
                          void *pParent,
                          TI_HANDLE hReport,
                          TI_HANDLE hOs,
                          TI_HANDLE hCtrlData,
                          TI_HANDLE hEvHandler,
                          TI_HANDLE hConn,
                          TI_HANDLE hTimer);

#endif

