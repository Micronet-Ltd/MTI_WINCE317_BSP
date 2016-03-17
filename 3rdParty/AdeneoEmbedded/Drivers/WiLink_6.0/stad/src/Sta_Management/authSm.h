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

#ifndef _AUTH_SM_H
#define _AUTH_SM_H

#include "fsm.h"
#include "mlmeApi.h"

/* Constants */

#define AUTH_MSG_HEADER_LEN		6
#define MAX_CHALLANGE_LEN		256
#define MAX_AUTH_MSG_LEN		MAX_CHALLANGE_LEN + AUTH_MSG_HEADER_LEN

/* Enumerations */

/* Typedefs */

typedef struct
{
	TI_UINT16				status;
	char				*pChalange;
	TI_UINT8				challangeLen;
} authData_t;

typedef struct
{
	legacyAuthType_e	authType;
	fsm_stateMachine_t	*pAuthSm;
	TI_UINT8				currentState;
	TI_UINT32				maxCount;
	TI_UINT8				retryCount;
	TI_UINT32				authRejectCount;
	TI_UINT32				authTimeoutCount;
	TI_UINT32				timeout;
    authData_t          authData;
	TI_HANDLE			hAuthSmTimer;

	TI_HANDLE			hMlme;
	TI_HANDLE			hRsn;
	TI_HANDLE			hReport;
	TI_HANDLE			hOs;
	TI_HANDLE			hTimer;
} auth_t;

/* Structures */

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

TI_STATUS auth_start(TI_HANDLE hAuth);

TI_STATUS auth_stop(TI_HANDLE hpAuth, TI_BOOL sendDeAuth, mgmtStatus_e reason );

TI_STATUS auth_recv(TI_HANDLE hAuth, mlmeFrameInfo_t *pFrame);

void auth_smTimeout(TI_HANDLE hAssoc, TI_BOOL bTwdInitOccured);

TI_STATUS auth_smMsgBuild(auth_t *pCtx, 
					   TI_UINT16 seq, 
					   TI_UINT16 statusCode, 
					   TI_UINT8* pChallange, 
					   TI_UINT8 challangeLen);

/* local functions */

#endif

