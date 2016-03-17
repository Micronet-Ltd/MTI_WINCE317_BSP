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
/** \file broadcastKeySM.h
 *  \brief station broadcast key SM API
 *
 *  \see broadcastKeySM.c
 */

/****************************************************************************
 *                                                                          *
 *   MODULE:  station broadcast key SM	                                	*
 *   PURPOSE: station broadcast key SM API		                            *
 *                                                                          *
 ****************************************************************************/

#ifndef _BROADCAST_KEY_SM_H
#define _BROADCAST_KEY_SM_H

#include "paramOut.h"
#include "fsm.h"
#include "rsnApi.h"
#include "keyTypes.h"

/* Constants */

/* Enumerations */

/* Typedefs */

typedef struct _broadcastKey_t    broadcastKey_t;

/* Main Sec SM functions */
typedef TI_STATUS (*broadcastKeySmStart_t)(struct _broadcastKey_t *pBroadcastKey);
typedef TI_STATUS (*broadcastKeySmStop_t)(struct _broadcastKey_t *pBroadcastKey);
typedef TI_STATUS (*broadcastKeySmRecvSuccess_t)(struct _broadcastKey_t *pBroadcastKey, encodedKeyMaterial_t *pEncodedKeyMaterial);
typedef TI_STATUS (*broadcastKeySmRecvFailure_t)(struct _broadcastKey_t *pBroadcastKey);

/* Structures */

/* State machine associated data structures. */
typedef struct
{
	encodedKeyMaterial_t    *pEncodedKeyMaterial;
} broadcastKeyData_t;

struct _broadcastKey_t
{
	TI_UINT8                               currentState;
	fsm_stateMachine_t	                *pBcastKeySm;
    broadcastKeyData_t					data;

    struct _mainKeys_t                 	*pParent;
	struct _keyDerive_t					*pKeyDerive;
	
	TI_HANDLE			                hReport;
	TI_HANDLE			                hOs;
    
    broadcastKeySmStart_t               start;
    broadcastKeySmStop_t                stop;
    broadcastKeySmRecvSuccess_t			recvSuccess;
    broadcastKeySmRecvFailure_t			recvFailure;
};

/* External data definitions */

/* External functions definitions */

/* Function prototypes */

broadcastKey_t* broadcastKey_create(TI_HANDLE hOs);

TI_STATUS broadcastKey_unload(broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKey_config(broadcastKey_t *pBroadcastKey, 
						   TRsnPaeConfig *pPaeConfig, 
						   struct _mainKeys_t *pParent,
						   TI_HANDLE hReport,
						   TI_HANDLE hOs);

TI_STATUS broadcastKeySmUnexpected(struct _broadcastKey_t *pBroadcastKey);

TI_STATUS broadcastKeySmNop(struct _broadcastKey_t *pBroadcastKey);

#endif /*  _BROADCAST_KEY_SM_H*/
